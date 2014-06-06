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
	VirtualArrayBlock<T, N>(size_t nr)
		: next(nullptr), prev(nullptr), m_nr(nr)
	{
		for (auto& it : m_data) {
			it = nullptr;
		}
	}

	std::atomic<T>& operator[](const size_t idx)
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
