/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef PARETOLOCALITYTASKSTORAGEITEM_H_
#define PARETOLOCALITYTASKSTORAGEITEM_H_

#include <pheet/misc/type_traits.h>

namespace pheet
{

template < class Pheet,
           class Place,
           class BaseItem,
           class Strategy >
class ParetoLocalityTaskStorageItem : public BaseItem
{
public:
	typedef typename BaseItem::T T;

	ParetoLocalityTaskStorageItem()
		: m_owner(nullptr)
	{}

	T take()
	{
		bool expected = false;
		if (this->taken.compare_exchange_strong(
		            expected, true,
		            std::memory_order_release, std::memory_order_acquire)) {
			this->version.store(this->version.load(std::memory_order_relaxed) + 1,
			                    std::memory_order_relaxed);
			return this->data;
		}
		return nullable_traits<T>::null_value;
	}

	void take_and_delete()
	{
		bool expected = false;
		if (this->taken.compare_exchange_strong(
		            expected, true,
		            std::memory_order_release, std::memory_order_acquire)) {
			this->version.store(this->version.load(std::memory_order_relaxed) + 1,
			                    std::memory_order_relaxed);
			this->data.drop_item();
		}
	}

	bool is_dead()
	{
		return m_strategy.dead_task();
	}

	bool is_taken() const
	{
		return this->taken.load(/*TODOMK: aquire?*/);
	}

	bool is_taken_or_dead()
	{
		return is_taken() || is_dead();
	}

	Strategy* strategy()
	{
		return &m_strategy;
	}

	void strategy(Strategy&& strategy)
	{
		m_strategy = std::move(strategy);
	}

	void owner(Place* const owner)
	{
		m_owner = owner;
	}

	Place* owner() const
	{
		return m_owner;
	}

private:
	Place* m_owner;
	Strategy m_strategy;

};
} /* namespace pheet */
#endif /* PARETOLOCALITYTASKSTORAGEITEM_H_ */

