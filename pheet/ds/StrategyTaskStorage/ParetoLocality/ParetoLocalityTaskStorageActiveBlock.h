/*
 * Copyright Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef PARETOLOCALITYTASKSTORAGEACTIVEBLOCK_H
#define PARETOLOCALITYTASKSTORAGEACTIVEBLOCK_H

#include "ParetoLocalityTaskStorageBaseBlock.h"
#include "ParetoLocalityTaskStorageDeadBlock.h"

namespace pheet
{

template<class Item, size_t MAX_PARTITION_SIZE>
class ParetoLocalityTaskStorageActiveBlock
	: public ParetoLocalityTaskStorageBaseBlock<Item, MAX_PARTITION_SIZE>
{
public:
	typedef ParetoLocalityTaskStorageBaseBlock<Item, MAX_PARTITION_SIZE> BaseBlock;
	typedef typename BaseBlock::T T;
	typedef typename BaseBlock::VA VA;
	typedef typename BaseBlock::VAIt VAIt;
	typedef ParetoLocalityTaskStorageDeadBlock<Item, MAX_PARTITION_SIZE> DeadBlock;

	ParetoLocalityTaskStorageActiveBlock(VA& array, size_t offset,
	                                     PivotQueue* pivots)
		: ParetoLocalityTaskStorageBaseBlock<Item, MAX_PARTITION_SIZE>(array, offset),
		  m_pivots(pivots)
	{
		create_partition_pointers(0, m_capacity, 0);
	}

	~ParetoLocalityTaskStorageActiveBlock()
	{
		delete m_partitions;
	}

	/**
	 * Return an iterator to an item that is not dominated by any other item in this block.
	 *
	 * Do not remove this item from the block. If such an item does not exist,
	 * return an invalid iterator.
	 *
	 * Any dead items that are inspected are cleaned up. Thus, if an Iterator
	 * to a non-valid Item is returned, the block can be destructed.
	 */
	virtual VAIt top()
	{
		VAIt best_it;
		//iterate through items in right-most partition
		auto it = m_partitions->last();
		const auto end_it = VA::min(m_partitions->end(),
		                            m_partitions->dead_partition());
		for (; it < end_it; it++) {
			pheet_assert(it.index() < end_it.index());
			if (!it.validItem()) {
				continue;
			}
			if (it->is_taken()) {
				*it = nullptr;
				continue;
			}

			if (it->is_dead()) {
				it->take_and_delete();
				// Memory manager will take care of deleting items
				*it = nullptr;
				continue;
			}

			if (!best_it.validItem()
			        || it->strategy()->prioritize(*(best_it)->strategy())) {
				best_it = it;
			}
		}

		//only happens if no more item that is not null is in current partition
		//thus, fall back to previous partition, if possible
		if (!best_it.validItem() && m_partitions->fall_back()) {
			//repartition the new right-most partition (if neccessary)
			VAIt start_it = m_partitions->last();
			end_it = VA::min(m_partitions->dead_partition(), m_partitions->end());
			pheet_assert(end_it.index() >= start_it.index());
			if (end_it.index() - start_it.index() > MAX_PARTITION_SIZE) {
				--end_it;
				partition(m_partitions->size() - 1, start_it, end_it);
				//check if we can reduce the level of this block by 1
				/*if(try_shrink()) {
						//mark the second half as dead
						DeadBlock* dead_block = new DeadBlock(m_data, m_offset + m_capacity, m_lvl);
						dead_block->next(this->next());
						this->next()->prev(dead_block);
						dead_block->prev(this);
						this->next(dead_block);
					}*/
			}
			//call top() again
			best_it = top();
		}
		return best_it;
	}

	ParetoLocalityTaskStorageActiveBlock* merge_next()
	{
		pheet_assert(m_next.load(std::memory_order_acquire) != nullptr);
		pheet_assert(m_next.load(std::memory_order_acquire)->lvl() == m_lvl);
		//we only merge full blocks
		pheet_assert(m_partitions->end().index(m_offset) == m_capacity);

		//expand this block to cover this as well as next block
		++m_lvl;
		m_capacity <<= 1;

		//splice out next
		auto tmp  = m_next.load(std::memory_order_acquire);
		if (tmp->next()) {
			tmp->next()->prev(this);
		}
		m_next.store(tmp->next(), std::memory_order_release);
		delete tmp;

		return this;
	}

	void partition()
	{
		pheet_assert(!this->next());

		//drop the old partition pointers and create new ones
		delete m_partitions;
		create_partition_pointers(0,  m_capacity, m_capacity);

		//partition the whole block
		auto left = m_data.iterator_to(m_offset);
		auto right = m_data.iterator_to(m_offset + m_capacity - 1);
		partition(0, left, right);

		//check if we can reduce the level of this block
		if (m_partitions->dead_partition().index(m_offset) <= m_capacity / 2) {
			//reduce lvl and capacity
			//TODOMK: can we reduce by more than 1?
			--m_lvl;
			m_capacity >>= 1;
			//update end pointer
			//TODOMK: more efficent way to get new end iterator
			VAIt it = m_data.iterator_to(m_offset + m_capacity);
			m_partitions->end(it);
			pheet_assert(m_capacity == m_partitions->end().index()
			             - m_partitions->first().index());

			/* delete the rest of the block (by reducing the capacity of the
			 * VirtualArray) */
			m_data.decrease_capacity(m_capacity);
		}
	}

private:

	void create_partition_pointers(size_t start, size_t dead, size_t end)
	{
		auto start_it = m_data.iterator_to(m_offset + start);
		auto dead_it = m_data.iterator_to(m_offset + dead);
		auto end_it = m_data.iterator_to(m_offset + end);
		m_partitions = new PartitionPointers<Item>(m_pivots, start_it, dead_it, end_it);
	}

	void partition(size_t depth, VAIt& left, VAIt& right)
	{
		check_correctness();
		pheet_assert(left.index() < right.index());
		const auto old_left = left;

		PivotElement* pivot = get_pivot(depth, left, right);
		if (pivot == nullptr) {
			/* could not generate suitable pivot element -> Abort partitioning
			 * Right-most partition will exceed MAX_PARTITION_SIZE
			 */
			return;
		}
		const size_t p_dim = pivot->dimension();
		const size_t p_val = pivot->value();

		do {
			//TODOMK: try to call is_dead as little as possible (may be expensive, since user implemented)
			while (left < right && *left && !left->is_taken_or_dead()
			        && left->strategy()->less_priority(p_dim, p_val)) {
				left++;
			}

			while (left < right && *right && !right->is_taken_or_dead()
			        && (right->strategy()->greater_priority(p_dim, p_val)
			            || right->strategy()->equal_priority(p_dim, p_val))) {
				--right;
			}
			if (left != right) {
				if (!*right || right->is_taken_or_dead()) {
					//right is dead
					if (m_partitions->dead_partition().index() - right.index()  == 1) {
						//element after right is dead too. Advance dead and right.
						//This is safe since left < right
						drop_dead_item(right);
						m_partitions->decrease_dead();
						pheet_assert(left < right);
						--right;
					} else {
						//swap right with rightmost non-dead element
						m_partitions->decrease_dead();
						VAIt dead = m_partitions->dead_partition();
						swap_to_dead(right, dead);
					}
				} else if (!*left || left->is_taken_or_dead()) {
					/* left is dead. Note that left+1==dead may never occur while
					 * left < right, since right < dead holds. */
					pheet_assert(left.index() + 1 < m_partitions->dead_partition().index());
					/* swap left with rightmost non-dead element. This may swap
					 * an element >=pivot to left, but we will not advance left.
					 * Progress is made by putting one dead element into it'S final
					 * place */
					m_partitions->decrease_dead();
					VAIt dead = m_partitions->dead_partition();
					swap_to_dead(left, dead);
					//if now right == dead, advance right
					if (m_partitions->dead_partition() == right) {
						--right;
					}
				} else {
					/* neither left nor right are dead. Swap left and right */
					swap(left, right);
					/* items at left and right are now at the correct position.
					 * We thus may advance both indices. However, in case we
					 * swapped when left + 1 == right, this will result in
					 * left == right + 1. */
					left++;
					--right;
				}
			}
		} while (left < right);
		/* Partitioning finished when left <= right. Left == right +1 is the case
		 * if the last swap was on indices s.t. left + 1 == right and both items
		 * were not dead. */
		pheet_assert(left == right || left.index() == right.index() + 1);

		//if the taken or dead status changes between *1 and *2, the item could
		//be sorted incorrectly if left->is_taken_or_dead() would be called at each
		//place instead of using this variable.
		bool was_taken_or_dead = false;
		if (*left) {
			was_taken_or_dead = left->is_taken_or_dead();
		}
		//*1: check if left points to dead item
		if (!*left || was_taken_or_dead) {
			//decrease the dead partition pointer
			m_partitions->decrease_dead();
			//if left==dead_partition, just drop item at left
			drop_dead_item(left);
			//Otherwise, swap dead and left
			if (left != m_partitions->dead_partition()) {
				VAIt dead = m_partitions->dead_partition();
				swap_to_dead(left, dead);
			}
		}
		pheet_assert(left.index() <= m_partitions->dead_partition().index());

		//*2: check if item at left belongs to left or right partition
		if (*left && !was_taken_or_dead && left->strategy()->less_priority(p_dim, p_val)) {
			left++;
			pheet_assert(left.index() <= m_partitions->dead_partition().index());
		}

		//check if the last partitioning step needs to be redone
		if (!partition_failed(pivot, left)) {
			m_failed_attempts = 0;
		} else {
			/* partitioning failed. Reset the start of the section that needs
			 * to be partitioned further. Note: The end of this section is
			 * the start of the dead partition.
			 */
			left = old_left;
			/* If right-most partition > MAX_PARTITION_SIZE, we will partition again.
			 * This could potentially run indefinitely (think of all items having
			 * the same priority vector). Thus, bound the number of subsequent
			 * partitioning attempts that do not create a new partition.
			 */
			++m_failed_attempts;
		}

		if ((m_partitions->dead_partition().index() - left.index() > MAX_PARTITION_SIZE)
		        && m_failed_attempts < MAX_ATTEMPTS_TO_PARTITION) {
			/* If partitioning succeeded but the resulting right-most (excluding
			 * dead) partition is >MAX_PARTITION_SIZE, partition recursively */
			if (m_failed_attempts == 0) {
				++depth;
			}
			auto new_right = m_partitions->dead_partition();
			--new_right;
			partition(depth, left, new_right);
		}
		m_failed_attempts = 0;
		check_correctness();
	}

	/**
	 *
	 * A partitioning step failed iff
	 * - The newly created right-most partition (excluding dead partition) is
	 *   empty; and
	 * - The pivot element used for the partitioning step is not used by any
	 *   other block
	 *
	 * If the partitioning step did not fail, a new partition pointer is saved if
	 * neccessary.
	 *
	 * @param pivot The pivot element used for the partition step
	 * @param left Start of the newly created partition (which is the right-most
	 * partition of the block, except for the dead partition).
	 * @return False, iff partition can proceed with the next step. True
	 * otherwise (implying that the last partitioning step has to be repeated
	 * with a new pivot element).
	 */
	bool partition_failed(PivotElement* pivot, VAIt& left)
	{
		if (left.index() != m_partitions->dead_partition().index()) {
			/* if rightmost partition contains at least 1 item, add a partition
			   pointer */
			m_partitions->add(left, pivot);
			return false;
		}
		/* all items were partitioned into left or dead partition. Thus, our
		 * rightmost non- empty partition is [old_left, dead[. In other words:
		 * [left, dead[ is an empty partition. Thus, we can release the pivot
		 * element.
		 */
		m_pivots->release(pivot);
		if (pivot->refcnt() == 0) {
			/* The pivot element isn't used anywhere else. Thus, tell the caller
			 * that partitioning failed (So that he can try again with a
			 * different pivot element).
			 */
			return true;
		}
		/* the pivot element that caused [left, dead[ to be empty
		 * is already used by other blocks. We do not need to create the
		 * partition and can continue to partition recursively, but cannot
		 * remove the pivot element or try to partition again with a different
		 * pivot element.
		 */
		return false;
	}

	/**
	 * Move item at rhs to lhs and set item at rhs to nullptr.
	 *
	 * If item at lhs is not null, take and drop it.
	 */
	void swap_to_dead(VAIt& lhs, VAIt& rhs)
	{
		Item* right = *rhs;
		Item* left = *lhs;
		pheet_assert(left == nullptr || left->is_taken_or_dead());
		//if item is dead but not taken by another place, drop it
		drop_dead_item(lhs);
		*lhs = right;
		*rhs = nullptr;
	}

	/**
	 * Swap item at lhs with item of rhs
	 */
	void swap(VAIt& lhs, VAIt& rhs)
	{
		Item* left = *lhs;
		Item* right = *rhs;
		*lhs = right;
		*rhs = left;
		//Note: the below does not work since std::atomic is not copy-assignable
		//*lhs = *rhs;
	}

	void drop_dead_items(VAIt start, VAIt end)
	{
		for (; start != end; start++) {
			//we cannot drop items that need yet be processed
			pheet_assert(*start == nullptr || start->is_taken_or_dead());

			//if item is taken, the place that took it will drop it
			//so take and delete only non-null items that are not taken
			drop_dead_item(start);

			// Memory manager will take care of deleting items
			*start = nullptr;
		}
	}

	/**
	 * If item is not null and not taken by another place, take and delete it.
	 */
	void drop_dead_item(VAIt item)
	{
		if (*item && !item->is_taken()) {
			item->take_and_delete();
		}
		*item = nullptr;
	}

	PivotElement* get_pivot(size_t depth, VAIt& left, VAIt& right)
	{
		PivotElement* pivot = nullptr;
		//generate new pivot element if neccesarry
		if (m_pivots->size() <= depth) {
			pivot = generate_pivot(left, right, depth);
		} else {
			pivot = m_pivots->get(depth);
		}
		return pivot;
	}

	PivotElement* generate_pivot(VAIt& left, VAIt& right, size_t pos)
	{
		std::mt19937 rng;
		size_t seed = std::random_device()();
		//quick and dirty fix to de-randomize tests when in debug mode
#ifdef PHEET_DEBUG_MODE
		seed = 42;
#endif
		rng.seed(seed);
		std::uniform_int_distribution<std::mt19937::result_type> dist_e(left.index(), right.index());

		size_t upper = left->strategy()->nr_dimensions() - 1;
		std::uniform_int_distribution<std::mt19937::result_type> dist_d(0, upper);

		Item* item;
		size_t attempts_to_generate_pivot = 0;

		while (attempts_to_generate_pivot < MAX_ATTEMPTS_TO_GENERATE_PIVOT) {
			//random dimension
			size_t d = dist_d(rng);

			//sample items
			size_t attempts_to_sample = 0;
			std::vector<Item*> samples;
			while (samples.size() < NR_SAMPLES_FOR_PIVOT_GENERATION) {
				item = m_data[dist_e(rng)];
				if (item && !item->is_taken_or_dead()) {
					samples.push_back(item);
				}
				++attempts_to_sample;
				if (attempts_to_sample == MAX_ATTEMPTS_TO_SAMPLE) {
					//Could not get required number of sample items
					//TODOMK: if we got at least one, we could still partition
					return nullptr;
				}
			}

			//find the median of priority vectors at dimension d
			ItemComparator<Item> itemCompare(d);
			std::sort(samples.begin(), samples.end(), itemCompare);
			item = samples[(size_t) NR_SAMPLES_FOR_PIVOT_GENERATION / 2];

			//try to generate the pivot
			PivotElement* pivot = new PivotElement(d, item->strategy()->priority_at(d), pos);
			if (m_pivots->try_put(pivot)) {
				return pivot;
			}
			++attempts_to_generate_pivot;
		}
		return nullptr;
	}

private: //methods to test correctness of data structure

	/**
	 * Check if the block is in a valid state.
	 */
	void check_correctness()
	{
#ifdef PHEET_DEBUG_MODE
		for (size_t i = 1; i < m_partitions->size(); i++) {
			auto start = m_partitions->get(i - 1).first;
			PivotElement* pivot = m_partitions->get(i).second;
			auto pp = m_partitions->get(i).first;
			auto end = m_partitions->dead_partition();
			check_partition(pivot, start, pp, end);
		}
		check_dead();
#endif
	}

	/**
	 * Check a partition of this block for correctness.
	 *
	 * In detail:
	 * - In [start, pp[, all elements have to be > pivot (i.e., the pivot element is <
	 *  than any element in the given range (for the dimension given by the pivot)
	 * - In [pp, end[, all elements have to be <= pivot
	 */
	void check_partition(PivotElement* pivot, VAIt& start, VAIt& pp, VAIt& end)
	{
		Item* item;
		for (; start != pp; start++) {
			item = *start;
			assert(item->strategy()->priority_at(pivot->dimension()) > pivot->value());
		}
		for (; pp != end; pp++) {
			item = *pp;
			assert(item->strategy()->priority_at(pivot->dimension()) <= pivot->value());
		}
	}

	/**
	 * Check the "dead" partition of this block for correctness.
	 *
	 * In detail:
	 * - check that all items in the "dead" partition are either null.
	 */
	void check_dead()
	{
		auto it = m_partitions->dead_partition();
		const auto end_it = m_partitions->end();
		for (; it != end_it; it++) {
			if (*it == nullptr) {
				continue;
			}
			//should never get here
			assert(false);
		}
	}

protected:
	using BaseBlock::m_data;
	using BaseBlock::m_capacity;
	using BaseBlock::m_lvl;
	using BaseBlock::m_offset;
	using BaseBlock::m_next;
	using BaseBlock::m_prev;
	PartitionPointers<Item>* m_partitions;

private:
	PivotQueue* m_pivots;
	size_t m_failed_attempts;
};

} //namespace pheet

#endif // PARETOLOCALITYTASKSTORAGEACTIVEBLOCK_H