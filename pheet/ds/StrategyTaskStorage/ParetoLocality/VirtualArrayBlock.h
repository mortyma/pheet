/*
 * Copyright Jakob Gruber, Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef DATA_BLOCK_H_
#define DATA_BLOCK_H_

#include <atomic>
#include <array>
#include "pheet/misc/assert.h"

namespace pheet
{

template <class T, size_t N>
class VirtualArrayBlock
{
private:
	typedef VirtualArrayBlock<T, N> Self;

public:
	/* Ref is the result of a call to the overloaded []-operator. It applies the
	 * load or store operation on the atomic variable ref_ based on how the
	 * instance is used in a future expression.
	 *
	 * This way, we can use relaxed memory order semantics in combination with
	 * the subscript operator.
	 */
	struct Ref {
		std::atomic<T>& ref_;
		Ref(std::atomic<T>& r) : ref_(r) {}
		operator T() const
		{
			return ref_.load(std::memory_order_relaxed);
		}
		T operator = (T val) const
		{
			ref_.store(val, std::memory_order_relaxed);
			return ref_;
		}
	};

	VirtualArrayBlock<T, N>(size_t nr)
		: next(nullptr), prev(nullptr), m_nr(nr)
	{
		for (auto& it : m_data) {
			it = nullptr;
		}
	}

	Ref operator[](const size_t idx)
	{
		pheet_assert(idx < N);
		return m_data[idx];
	}

	constexpr size_t capacity() const
	{
		return N;
	}

	constexpr size_t nr() const
	{
		return m_nr;
	}

public:
	std::atomic<Self*> next;
	Self* prev;

private:
	std::array<std::atomic<T>, N> m_data;
	const size_t m_nr;
};

} /* namespace pheet */

#endif /* DATA_BLOCK_H_ */
