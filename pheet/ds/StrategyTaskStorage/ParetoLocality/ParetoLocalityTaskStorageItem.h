/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef PARETOLOCALITYTASKSTORAGEITEM_H_
#define PARETOLOCALITYTASKSTORAGEITEM_H_

namespace pheet
{

template < class Pheet,
           class Place,   //TODO: do we need the place that owns the item here?
           class BaseItem,
           class Strategy >
class ParetoLocalityTaskStorageItem : public BaseItem
{
public:
	typedef typename BaseItem::T T;

	ParetoLocalityTaskStorageItem() {}

	T take()
	{
		//TODO: no concurrency yet
		this->taken.store(true, std::memory_order_relaxed);
		return this->data;
	}

	void take_and_delete()
	{
		//TODO: no concurrency yet
		this->taken.store(true, std::memory_order_relaxed);
		this->data.drop_item();
	}

	bool is_dead()
	{
		return m_strategy.dead_task();
	}

	bool is_taken() const
	{
		return this->taken.load();
	}

	bool is_taken_or_dead()
	{
		return is_taken() || is_dead();
	}

	Strategy* strategy() {
		return &m_strategy;
	}

	void strategy(Strategy&& strategy)
	{
		m_strategy = std::move(strategy);
	}

private:
	Strategy m_strategy;
};
} /* namespace pheet */
#endif /* PARETOLOCALITYTASKSTORAGEITEM_H_ */

