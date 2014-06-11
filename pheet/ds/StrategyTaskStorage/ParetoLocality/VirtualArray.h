/*
 * Copyright Jakob Gruber, Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef VIRTUAL_ARRAY_H_
#define VIRTUAL_ARRAY_H_

#include <iostream>
#include <iterator>

#include "VirtualArrayBlock.h"

#define DATA_BLOCK_SIZE (100)

namespace pheet
{

/**
 * A dynamic virtual array of (potentially) infinite size.
 *
 * The virtual array allows access to any element within the range
 * [start_idx(), start()+capacity]. If the element was not yet set, nullptr will be returned.
 *
 * Implemented as a linked list of VirtualArrayBlock. Only the owning thread may
 * change the structure or traverse the linked list backwards; other threads may
 * only traverse it forwards, i.e., by calling VirtualArrayBlock->next(), which is
 * atomic.
 *
 * VirtualArrayIterator should be used to iteratate over ranges of the virtual array.
 * Contrary to the usual C++ convention, end() will return an iterator to
 * the last accessible element of the virtual array. The actual capacity of the
 * virtual array is capacity()+1, so that the last element is a dummy so that
 * the iterator always references a valid block.
 *
 * Note that the virtual array may shrink from the left; that is, elements in
 * the range [0  m_start_index[ may not be accessed!
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
		VirtualArrayIterator() : m_block(nullptr), m_idx_in_block(0)
		{
		}

		VirtualArrayIterator(const VirtualArrayIterator& that)
			: m_block(that.m_block), m_idx_in_block(that.m_idx_in_block)
		{
		}

		VirtualArrayIterator& operator=(const VirtualArrayIterator& that)
		{
			m_block = that.m_block;
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
				m_idx_in_block = (m_block == nullptr) ? 0 : m_block->capacity() - 1;
			} else {
				m_idx_in_block--;
			}
			pheet_assert(m_block);
			return *this;
		}

		bool valid() const
		{
			return m_block;
		}

		/**
		 * Returns true iff the iterator points to a valid, i.e., accessible and
		 * non-null Item.
		 */
		bool validItem() const
		{
			return m_block && m_block->operator [](m_idx_in_block);
		}

		std::atomic<T>& operator*() const
		{
			pheet_assert(m_block);
			return m_block->operator [](m_idx_in_block);
		}

		T operator->() const
		{
			pheet_assert(m_block);
			return m_block->operator [](m_idx_in_block);
		}

		bool operator==(const VirtualArrayIterator& that)
		{
			pheet_assert(m_block);
			return (m_block == that.m_block && m_idx_in_block == that.m_idx_in_block);
		}

		bool operator!=(const VirtualArrayIterator& that)
		{
			return !operator == (that);
		}

		bool operator<(const VirtualArrayIterator& that)
		{

			pheet_assert(m_block);
			if (m_block->nr() < that.m_block->nr()) {
				return true;
			}
			if (m_block->nr() > that.m_block->nr()) {
				return false;
			}
			return (m_idx_in_block < that.m_idx_in_block);
		}

		size_t index(size_t offset = 0) const
		{
			pheet_assert(m_block);
			size_t idx = m_block->nr() * DATA_BLOCK_SIZE + m_idx_in_block - offset;
			pheet_assert(idx >= 0);
			return idx;
		}

	private:
		VirtualArray::Block* m_block;
		size_t m_idx_in_block; /** Index within the block */
	};

	VirtualArray()
		: m_start_idx(0), m_end_idx(1)
	{
		m_last = m_start = m_first = new Block(0);
	}

	~VirtualArray()
	{
		std::cerr << "Virtual array capacity: " << m_end_idx - 65 - m_start_idx << "\n";
		while (m_first->next != nullptr) {
			m_first = m_first->next;
			delete m_first->prev;
		}
		delete m_first;
	}

	VirtualArrayIterator begin() const
	{
		return iterator_to(m_start_idx);
	}

	//TODOMK:
	//VirtualArrayIterator iterator_to(VirtualArrayIterator start, const size_t idx) const

	VirtualArrayIterator iterator_to(const size_t idx) const
	{
		pheet_assert(m_start_idx <= idx);
		pheet_assert(idx < m_end_idx);
		Block* block = find_block(idx);
		VirtualArrayIterator it;
		it.m_block = block;
		it.m_idx_in_block = idx % block_size();
		return it;
	}

	/** The iterator returned by end() points to the last accessible element */
	VirtualArrayIterator end() const
	{
		return iterator_to(m_end_idx - 1);
	}

	static const VirtualArrayIterator& min(const VirtualArrayIterator& it1,
	                                       const VirtualArrayIterator& it2)
	{
		return it1.index() < it2.index() ? it1 : it2;
	}

	std::atomic<T>& operator[](size_t idx)
	{
		pheet_assert(m_start_idx <= idx);
		pheet_assert(idx < m_end_idx);

		Block* block = find_block(idx);
		return (*block)[idx % block_size()];
	}

	size_t capacity() const
	{
		pheet_assert(m_end_idx >= m_start_idx.load(std::memory_order_relaxed));
		return m_end_idx - m_start_idx.load(std::memory_order_relaxed);
	}

	constexpr size_t block_size() const
	{
		return DATA_BLOCK_SIZE;
	}

	/**
	 * Increase the capacity of this VirtualArray by the given value.
	 *
	 * After calling this method, accessing this VirtualArray at any index in
	 * [start_idx(), previous capacity + value[ is legal.
	 */
	void increase_capacity(size_t value)
	{
		size_t free = block_size() - (m_end_idx % block_size());
		while (free < value) {
			add_block();
			free += block_size();
		}
		m_end_idx += value;
	}

	/**
	 * Decrease the capacity of this VirtualArray by the given value.
	 *
	 * After calling this method, accessing this VirtualArray at any position
	 * greater or equal to previous capacity - value is illegal, i.e., the legal
	 * range is [start_idx(), previous capacity - value[
	 */
	void decrease_capacity(size_t value)
	{
		pheet_assert(value < (m_end_idx - m_start_idx));
		m_end_idx -= value;
		/* We do not reduce/free the blocks since another thread might currently
		 * access them. Just keep them for later reusage. They are eventually
		 * fred in the destructor. */
	}

	/**
	 * Decrease the capacity of this VirtualArray by the given value by shrinking
	 * from the start, i.e.:
	 *
	 * After calling this method, accessing this VirtualArray at any position
	 * smaller than previous start + value is illegal, i.e., the legal
	 * range is [previous start_idx() + value, capacity[
	 */
	void decrease_capacity_from_start(size_t value)
	{
		pheet_assert(m_start_idx + value < m_end_idx);
		size_t freed = block_size() - (m_start_idx % block_size());
		while (freed < value) {
			m_start.store(m_start.load()->next);
			freed += block_size();
			/* We do not reduce/free the blocks since another thread might currently
			 * access them. Just keep them for later reusage. They are eventually
			 * fred in the destructor. */
		}
		m_start_idx += value;
	}

private:
	Block* find_block(size_t idx) const
	{
		pheet_assert(m_start_idx <= idx);
		pheet_assert(idx < m_end_idx);

		//find block that stores element at location idx
		Block* tmp = m_start;
		size_t cnt = (m_start.load()->nr() + 1) * block_size();
		//TODOMK: reduce asymptotic complexity
		while (cnt <= idx && tmp != nullptr) {
			tmp = tmp->next;
			cnt += block_size();
		}
		pheet_assert(tmp);
		return tmp;
	}

	void add_block()
	{
		Block* tmp = new Block(m_last->nr() + 1);
		tmp->prev = m_last;
		m_last->next.store(tmp, std::memory_order_release);
		m_last = tmp;
	}

private:
	std::atomic<size_t> m_start_idx; /** index of the first accessible item in the virtual array */
	size_t m_end_idx; /** the last accessible item in the virtual array is at (m_end_idx - 1) */
	Block* m_first; /** the first block in the doubly-linked list of blocks */
	std::atomic<Block*> m_start; /** the first block considered when iterating
									 over/accessing the virtual array. Contains m_start_idx */
	Block* m_last; /** the last block in the doubly-linked list of blocks */


};

} /* namespace pheet */

#endif /* VIRTUAL_ARRAY_H_ */
