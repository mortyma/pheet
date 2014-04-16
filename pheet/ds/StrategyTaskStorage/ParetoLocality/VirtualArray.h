/*
 * Copyright Jakob Gruber, Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef VIRTUAL_ARRAY_H_
#define VIRTUAL_ARRAY_H_

#include <iterator>

#include "VirtualArrayBlock.h"

#define DATA_BLOCK_SIZE (100)

namespace pheet
{

/**
 * A virtual array of infinite size
 */
template <class T>
class VirtualArray
{
public:
	typedef VirtualArrayBlock<T, DATA_BLOCK_SIZE> Block;

	class VirtualArrayIterator : public std::iterator<std::random_access_iterator_tag, T>
	{
		friend class VirtualArray<T>;

	public:
		VirtualArrayIterator() : m_block(nullptr), m_block_nr(0), m_idx_in_block(0)
		{
		}

		VirtualArrayIterator(const VirtualArrayIterator& that)
			: m_block(that.m_block), m_block_nr(that.m_block_nr), m_idx_in_block(that.m_idx_in_block)
		{
		}

		VirtualArrayIterator& operator=(const VirtualArrayIterator& that)
		{
			m_block = that.m_block;
			m_block_nr = that.m_block_nr;
			m_idx_in_block = that.m_idx_in_block;
			return *this;
		}

		VirtualArrayIterator operator++(int)
		{
			pheet_assert(m_block);

			VirtualArrayIterator orig(*this);

			m_idx_in_block++;
			if (m_idx_in_block >= m_block->capacity()) {
				m_block = m_block->next;
				m_block_nr++;
				m_idx_in_block = 0;
			}
			pheet_assert(m_block);
			return orig;
		}

		VirtualArrayIterator& operator--()
		{
			pheet_assert(m_block);

			if (m_idx_in_block == 0) {
				m_block = m_block->prev;
				m_block_nr--;
				m_idx_in_block = (m_block == nullptr) ? 0 : m_block->capacity() - 1;
			} else {
				m_idx_in_block--;
			}
			pheet_assert(m_block);
			return *this;
		}

		std::atomic<T>& operator*() const
		{
			return m_block->operator [](m_idx_in_block);
		}

		T operator->() const
		{
			return m_block->operator [](m_idx_in_block);
		}

		bool operator==(const VirtualArrayIterator& that)
		{
			return (m_block_nr == that.m_block_nr && m_idx_in_block == that.m_idx_in_block);
		}

		bool operator!=(const VirtualArrayIterator& that)
		{
			return !operator ==(that);
		}

		bool operator<(const VirtualArrayIterator& that)
		{
			if (m_block_nr < that.m_block_nr) {
				return true;
			}
			if (m_block_nr > that.m_block_nr) {
				return false;
			}
			return (m_idx_in_block < that.m_idx_in_block);
		}

		size_t index(size_t offset = 0) const
		{
			size_t idx = m_block_nr * DATA_BLOCK_SIZE + m_idx_in_block - offset;
			pheet_assert(idx >= 0);
			return idx;
		}

	private:
		VirtualArray::Block* m_block;
		ssize_t m_block_nr;	/**< The nr of the current block within the ordered sequence. */
		size_t m_idx_in_block; /** Index within the block */
	};

	VirtualArray()
		: m_block_cnt(1), m_capacity(1)
	{
		m_last = m_first = new Block();
	}

	~VirtualArray()
	{
		while (m_first->next != nullptr) {
			m_first = m_first->next;
			delete m_first->prev;
		}
		delete m_first;
	}

	VirtualArrayIterator begin() const
	{
		return iterator_to(0);
	}

	VirtualArrayIterator iterator_to(const size_t idx) const
	{
		Block* block = find_block(idx);
		VirtualArrayIterator it;
		it.m_block = block;
		it.m_idx_in_block = idx % block_size();
		for (const Block* i = m_first; i != block; i = i->next) {
			it.m_block_nr++;
		}

		return it;
	}

	/** The iterator returned by end() points to the last accessible element */
	VirtualArrayIterator end() const
	{
		return iterator_to(m_capacity - 1);
	}

	static const VirtualArrayIterator& min(const VirtualArrayIterator& it1,
	                                       const VirtualArrayIterator& it2)
	{
		return it1.index() < it2.index() ? it1 : it2;
	}

	/**
	 * Increase the capacity of this VirtualArray by the given value.
	 *
	 * After calling this method, accessing this VirtualArray at any position
	 * smaller than previous capacity + value is legal.
	 */
	void increase_capacity(size_t value)
	{
		m_capacity += value;
		while (m_block_cnt * block_size() < m_capacity) {
			add_block();
		}
	}

	/**
	 * Decrease the capacity of this VirtualArray by the given value.
	 *
	 * After calling this method, accessing this VirtualArray at any position
	 * greater or equal to previous capacity - value is illegal.
	 */
	void decrease_capacity(size_t value)
	{
		m_capacity -= value;
	}

	std::atomic<T>& operator[](size_t idx)
	{
		pheet_assert(idx < m_capacity);

		Block* block = find_block(idx);
		return (*block)[idx % block_size()];
	}

	constexpr size_t block_size() const
	{
		return DATA_BLOCK_SIZE;
	}

private:
	Block* find_block(size_t idx) const
	{
		//find block that stores element at location idx
		Block* tmp = m_first;
		size_t cnt = block_size();
		//TODO: reduce asymptotic complexity
		while (cnt <= idx && tmp != nullptr) {
			tmp = tmp->next;
			cnt += block_size();
		}
		return tmp;
	}

	void add_block()
	{
		Block* tmp = new Block();
		tmp->prev = m_last;
		m_last->next.store(tmp, std::memory_order_release);
		m_last = tmp;
		++m_block_cnt;
	}

private:
	Block* m_first;
	Block* m_last;
	size_t m_block_cnt;
	size_t m_capacity;
};

} /* namespace pheet */

#endif /* VIRTUAL_ARRAY_H_ */
