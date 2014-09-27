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

#define DATA_BLOCK_SIZE (128)

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

		VirtualArrayIterator& operator++(int)
		{
			pheet_assert(m_block);

			m_idx_in_block++;
			if (m_idx_in_block >= m_block->capacity()) {
				m_block = m_block->next;
				m_idx_in_block = 0;
			}
			pheet_assert(m_block);
			return *this;
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

		VirtualArrayIterator& advance(size_t cnt)
		{
			if (m_idx_in_block + cnt < m_block->capacity()) {
				m_idx_in_block += cnt;
			} else {
				cnt -= (m_block->capacity() - m_idx_in_block);
				m_block = m_block->next;
				pheet_assert(m_block);
				m_idx_in_block = 0;
				while (cnt >= m_block->capacity()) {
					m_block = m_block->next;
					pheet_assert(m_block);
					cnt -= m_block->capacity();
				}
				pheet_assert(cnt < m_block->capacity());
				m_idx_in_block = cnt;
			}
			pheet_assert(m_idx_in_block < m_block->capacity());
			return *this;
		}

		VirtualArrayIterator retreat(size_t cnt)
		{
			if (m_idx_in_block >= cnt) {
				m_idx_in_block -= cnt;
			} else {
				cnt -= m_idx_in_block;
				m_block = m_block->prev;
				pheet_assert(m_block);
				m_idx_in_block = m_block->capacity();
				while (cnt >= m_block->capacity()) {
					m_block = m_block->prev;
					pheet_assert(m_block);
					cnt -= m_block->capacity();
				}
				m_idx_in_block -= cnt;
			}
			return *this;
		}

		VirtualArrayIterator advance_cmp(const size_t cnt)
		{
			VirtualArrayIterator that(*this);
			return that.advance(cnt);
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

		Ref operator*() const
		{
			pheet_assert(m_block);
			return m_block->operator [](m_idx_in_block);
		}

		Ref operator->() const
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

		/**
		 * Calculate how many items there are in the range from this to that.
		 * Assumes that this <= that
		 *
		 */
		size_t nr_items_to(VirtualArrayIterator that)
		{
			size_t diff = 0;
			pheet_assert(this->m_block->nr() < that.m_block->nr()
			             || this->m_idx_in_block <= that.m_idx_in_block);
			if (that.m_block == m_block) {
				diff = that.m_idx_in_block - m_idx_in_block;
			} else {
				diff += m_block->capacity() - m_idx_in_block;
				diff += that.m_idx_in_block;
				Block* tmp = m_block->next;
				while (tmp != that.m_block) {
					diff += m_block->capacity();
					pheet_assert(tmp);
					tmp = tmp->next;
					pheet_assert(tmp);
				}
			}
			return diff;
		}

		size_t index() const
		{
			pheet_assert(m_block);
			size_t idx = m_block->nr() * DATA_BLOCK_SIZE + m_idx_in_block;
			pheet_assert(idx >= 0);
			return idx;
		}

	private:
		VirtualArray::Block* m_block;
		size_t m_idx_in_block; /** Index within the block */
	};

	VirtualArray()
	{
		m_last = m_first = new Block(0);
		VirtualArrayIterator* it = new VirtualArrayIterator();
		it->m_block = m_first;
		it->m_idx_in_block = 1;
		end_it.store(it);
	}

	~VirtualArray()
	{
		delete end_it.load(std::memory_order_relaxed);
		while (!m_spliced_out.empty()) {
			delete m_spliced_out.back();
			m_spliced_out.pop_back();
		}
		while (m_first->next != nullptr) {
			m_first = m_first->next;
			delete m_first->prev;
		}
		delete m_first;
	}

	VirtualArrayIterator begin() const
	{
		VirtualArrayIterator it;
		it.m_block = m_first;
		it.m_idx_in_block = 0;
		return it;
	}




	/** The iterator returned by end() points to the last accessible element */
	VirtualArrayIterator* end() const
	{
		pheet_assert(end_it.load(std::memory_order_relaxed)->m_block);
		return end_it.load(std::memory_order_relaxed);
	}

	void end(VirtualArrayIterator* it)
	{
		pheet_assert(it->m_block);
		end_it.store(it, std::memory_order_relaxed);
	}

	size_t capacity() const
	{
		return  end()->index();
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
		size_t free = block_size() - (capacity() % block_size() + 1);
		while (free < value) {
			if (m_last->next) {
				m_last = m_last->next;
			} else {
				add_block();
			}
			free += block_size();
		}
		VirtualArrayIterator* it = new VirtualArrayIterator(*end());
		it->advance(value);
		VirtualArrayIterator* tmp = end();
		end(it);
		delete tmp;
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
		if (value == 0) {
			return;
		}
		VirtualArrayIterator* it = new VirtualArrayIterator(*end());
		it->retreat(value);
		VirtualArrayIterator* tmp = end();
		end(it);
		delete tmp;
		/* We do not reduce/free the blocks since another thread might currently
		 * access them. Just keep them for later reusage. They are eventually
		 * fred in the destructor. */
	}

	size_t delete_range(VirtualArrayIterator left, VirtualArrayIterator right)
	{
		size_t shrunk_by = 0;
		while (left.m_block != right.m_block && left.m_block->next != right.m_block) {
			Block* tmp = left.m_block;
			m_spliced_out.push_back(tmp->next);
			size_t ends_at = (1 + tmp->next.load()->nr()) * block_size();
			pheet_assert(ends_at <= capacity());
			tmp->next.store(tmp->next.load()->next);
			tmp->next.load()->prev = tmp;
			shrunk_by += block_size();
		}
		return shrunk_by;
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
		pheet_assert(idx < capacity());

		//find block that stores element at location idx
		Block* tmp = start_block;
		size_t cnt = (start_block->nr() + 1) * block_size();
		//TODOMK: reduce asymptotic complexity
		while (cnt <= idx) {
			tmp = tmp->next;
			pheet_assert(tmp != nullptr);
			cnt += block_size() * (tmp->nr() - tmp->prev->nr());
		}
		pheet_assert(tmp);
		pheet_assert((tmp->nr()) * block_size() <= idx);
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
	Block* m_first; /** the first block in the doubly-linked list of blocks */
	Block* m_last; /** the last block in the doubly-linked list of blocks */
	std::vector<Block*> m_spliced_out;
	std::atomic<VirtualArrayIterator*> end_it;
};

} /* namespace pheet */

#endif /* VIRTUAL_ARRAY_H_ */
