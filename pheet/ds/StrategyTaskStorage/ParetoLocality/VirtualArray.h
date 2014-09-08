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
 * [0, capacity]. If the element was not yet set, nullptr will be returned.
 *
 * Implemented as a linked list of VirtualArrayBlock. Only the owning thread may
 * change the structure or traverse the linked list backwards; other threads may
 * only traverse it in forward direction, i.e., by calling
 * VirtualArrayBlock->next(), which is atomic.
 *
 * VirtualArrayIterator should be used to iteratate over ranges of the virtual array.
 * Contrary to the usual C++ convention, end() will return an iterator to
 * the last accessible element of the virtual array. The actual capacity of the
 * virtual array is capacity()+1; the last element is a dummy so that
 * the iterator always references a valid block.
 *
 */
template <class T>
class VirtualArray
{

public:
	typedef VirtualArrayBlock<T, DATA_BLOCK_SIZE> Block;
	typedef typename VirtualArrayBlock<T, DATA_BLOCK_SIZE>::Ref Ref;

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

		/**
		 * Invalidate the iterator.
		 *
		 * After this method is called, valid() will return false.
		 */
		void invalidate()
		{
			m_block = nullptr;
		}

		/**
		 * Returns true iff the iterator points to some item (the item may be a
		 * nullptr).
		 */
		bool valid() const
		{
			return m_block;
		}

		/**
		 * Returns true iff the iterator points to a valid, i.e., accessible and
		 * non-null Item.
		 *
		 * Note that validItem() implies valid().
		 */
		bool validItem() const
		{
			return m_block && m_block->operator [](m_idx_in_block);
		}

		Ref operator*() const
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
		: m_end_idx(1)
	{
		m_last = m_first = new Block(0);
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

	/**
	 * Get an iterator to the element at idx.
	 */
	VirtualArrayIterator iterator_to(const size_t idx) const
	{
		pheet_assert(idx < end_idx());
		Block* block = find_block(idx, m_first);
		VirtualArrayIterator it;
		it.m_block = block;
		it.m_idx_in_block = idx % block_size();
		return it;
	}

	/**
	 * Get an iterator to the element at idx. The search for this element is
	 * started at the block containing the item start_it points to.
	 */
	VirtualArrayIterator iterator_to(VirtualArrayIterator start_it, const size_t idx) const
	{
		pheet_assert(start_it.index() <= idx);
		pheet_assert(idx < end_idx());
		Block* block = find_block(idx, start_it.m_block);
		VirtualArrayIterator it;
		it.m_block = block;
		it.m_idx_in_block = idx % block_size();
		return it;
	}


	/** The iterator returned by end() points to the last accessible element */
	VirtualArrayIterator end() const
	{
		return iterator_to(end_idx() - 1);
	}

	Ref operator[](size_t idx)
	{
		pheet_assert(idx < end_idx());

		Block* block = find_block(idx, m_first);
		return (*block)[idx % block_size()];
	}

	size_t capacity() const
	{
		return end_idx();
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
		size_t free = block_size() - (end_idx() % block_size() + 1);
		while (free < value) {
			add_block();
			free += block_size();
		}
		end_idx(end_idx() + value);
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
		pheet_assert(value < end_idx());
		end_idx(end_idx() - value);
		/* We do not reduce/free the blocks since another thread might currently
		 * access them. Just keep them for later reusage. They are eventually
		 * fred in the destructor. */
	}

private:

	/**
	 * Get the block that contains the item at position idx.
	 *
	 * Start iterating through the list of blocks at start_block.
	 * On calling the method, the following must hold:
	 *     start_block->nr() * block_size() <= idx < m_end_idx
	 */
	Block* find_block(size_t idx, Block* start_block) const
	{
		pheet_assert(start_block->nr() * block_size() <= idx);
		pheet_assert(idx < end_idx());

		//find block that stores element at location idx
		Block* tmp = start_block;
		size_t cnt = (start_block->nr() + 1) * block_size();
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

	size_t end_idx() const
	{
		return m_end_idx.load(std::memory_order_relaxed);
	}

	void  end_idx(size_t idx)
	{
		m_end_idx.store(idx, std::memory_order_relaxed);
	}

private:
	/** the last accessible item in the virtual array is at (m_end_idx - 1) */
	std::atomic<size_t> m_end_idx;
	Block* m_first; /** the first block in the doubly-linked list of blocks */
	Block* m_last; /** the last block in the doubly-linked list of blocks */
};

} /* namespace pheet */

#endif /* VIRTUAL_ARRAY_H_ */
