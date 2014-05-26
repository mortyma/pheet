/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef PARETOLOCALITYTASKSTORAGEPLACE_H_
#define PARETOLOCALITYTASKSTORAGEPLACE_H_

#include "ParetoLocalityTaskStorageActiveBlock.h"
#include "ParetoLocalityTaskStorageItem.h"
#include "ParetoLocalityTaskStorageItemReuseCheck.h"

#include "pheet/memory/BlockItemReuse/BlockItemReuseMemoryManager.h"

#include <vector>

#define MAX_PARTITION_SIZE (64)

namespace pheet
{

template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
class ParetoLocalityTaskStoragePlace : public ParentTaskStoragePlace::BaseTaskStoragePlace
{

public:
	typedef ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy> Self;
	typedef typename ParentTaskStoragePlace::BaseItem BaseItem;
	typedef ParetoLocalityTaskStorageItem<Pheet, Self, BaseItem, Strategy> Item;
	typedef ParetoLocalityTaskStorageActiveBlock<Item, MAX_PARTITION_SIZE> ActiveBlock;
	typedef typename BaseItem::T T;
	typedef BlockItemReuseMemoryManager<Pheet, Item, ParetoLocalityTaskStorageItemReuseCheck<Item>>
	        ItemMemoryManager;
	typedef VirtualArray<Item*> VA;
	typedef typename VA::VirtualArrayIterator VAIt;
	typedef typename ParentTaskStoragePlace::PerformanceCounters PerformanceCounters;


	ParetoLocalityTaskStoragePlace(ParentTaskStoragePlace* parent_place);
	~ParetoLocalityTaskStoragePlace();

	void push(Strategy&& strategy, T data);
	T pop(BaseItem* boundary);
	T steal(BaseItem* boundary);
	void clean_up();

	const VirtualArray<Item*>& array()
	{
		return m_array;
	}

private:

	/**
	 * Ensure correct ordering of dead blocks, starting from block.
	 *
	 * After a call to this method, the sequence of dead blocks starting at "block"
	 * and ending at the nearest non-dead predecessor (non-dead-pred) of "block"
	 * will be ordered s.t. a preceeding dead block (d_i) is of less or equal size
	 * than any suceeding dead block in the sequence, i.e.,
	 *
	 * non-dead-pred-lvl() <= d_j.lvl() <= ... <= d_i.lvl <= block.lvl()
	 */
	ActiveBlock* handle_dead(ActiveBlock* block)
	{
		pheet_assert(block->is_dead());
		ActiveBlock* predecessor = block->prev();
		//block must have a predecessor
		pheet_assert(predecessor);

		if (predecessor->is_dead() && predecessor->lvl() > block->lvl()) {
			predecessor = swap_dead(predecessor, block);
			predecessor = handle_dead(predecessor);
		}
		return predecessor;
	}

	/**
	 * Swap two dead blocks, where "predecessor" is the immediate predecessor of "block".
	 */
	ActiveBlock* swap_dead(ActiveBlock* predecessor, ActiveBlock* block)
	{
		pheet_assert(predecessor->next() == block);
		pheet_assert(block->prev() == predecessor);
		/* We need to swap the two dead blocks. Luckily, since both are dead
		 * blocks and thus contain only elements pointing to null, we can
		 * simply decrease the level of predecessor and increase the level of
		 * block */
		//create the blocks
		size_t offset = predecessor->offset();
		size_t lvl = block->lvl();
		ActiveBlock* new_predecessor = new ActiveBlock(m_array, offset, &m_pivots, lvl);
		new_predecessor->set_dead(true);

		pheet_assert(block->offset() >= (predecessor->capacity() - block->capacity()));
		offset = block->offset() - (predecessor->capacity() - block->capacity());
		lvl = predecessor->lvl();
		ActiveBlock* new_block = new ActiveBlock(m_array, offset, &m_pivots, lvl);
		new_block->set_dead(true);

		//put them into the linked list
		new_predecessor->prev(predecessor->prev());
		new_predecessor->prev()->next(new_predecessor);
		new_predecessor->next(new_block);

		new_block->prev(new_predecessor);
		new_block->next(block->next());
		if (new_block->next()) {
			new_block->next()->prev(new_block);
		}

		//maintain last
		if (block == last) {
			last = new_block;
		}
		//delete the old blocks
		delete block;
		delete predecessor;

		return new_predecessor;
	}

	/**
	 * Starting from last, merge preceeding blocks as long as necessary. If a block
	 * was merged, partition it and maintain pointer to last
	 */
	void merge_from_last()
	{
		//merge recursively, if previous block has same level, starting from last
		if (merge_recursively(last, true)) {

			//repartition block that resulted from merge
			last->partition();

			//try to shrink the block
			if (last->try_shrink()) {
				//TODOMK: shrink may create a dead block at the very end only.
				//reset last and drop dead blocks at the end of the list
				last = drop_dead_blocks(get_last(last));
			}
		}
		pheet_assert(!last->is_dead());
	}

	/**
	 * Starting from block, merge preceeding blocks as long as necessary. If block
	 * was merged, partition it and maintain pointers to block and last
	 */
	void merge_from(ActiveBlock*& block)
	{
		pheet_assert(block != last);
		if (merge_recursively(block, false)) {
			//repartition block that resulted from merge
			//last does not change, since we never merge it in here
			block->partition();
			//TODOMK: should we shrink if possible?
		}
		pheet_assert(!last->is_dead());
	}

	/**
	 * Starting from block, merge blocks together as long as neccessary.
	 *
	 * Returns true, if blocks were merged. The caller is responsible for
	 * re-partitioning the resulting last block and cleaning up dead blocks
	 * at the end of the linked list, if necessary.
	 */
	bool merge_recursively(ActiveBlock*& block, bool merge_last)
	{
		pheet_assert(!block->is_dead());
		bool merged = false;
		while (merge(block, merge_last)) {
			merged = true;
		}
		return merged;
	}

	/**
	 * Merge block with active predecessor, if necessary.
	 *
	 * Returns true if a merge was performed; block will point to the
	 * resulting block. Pointer to last is maintained.
	 */
	bool merge(ActiveBlock*& block, bool merge_last = false)
	{
		pheet_assert(!block->is_dead());

		//if block is the last block and merge_last is not set, return
		if (block == last && !merge_last) {
			return false;
		}

		//If block does not have a predecessor, no merge is required.
		if (!block->prev()) {
			return false;
		}

		//Else, find active_pred, the closest non-dead predecessor of block. Such a
		//block has to exist.
		ActiveBlock* predecessor = block->prev();
		while (predecessor->is_dead()) {
			predecessor = predecessor->prev();
		}

		if (predecessor == block->prev()) {
			/* the two blocks to merge (predecessor and block) are next to each
			 * other... */
			if (block->lvl() == predecessor->lvl()) {
				//..and of the same size. We can merge them.
				pheet_assert(!predecessor->is_dead());
				block = predecessor->merge_next();
				return true;
			} else {
				//...but of different size. No more merges are required.
				pheet_assert(predecessor->lvl() > block->lvl());
				return false;
			}
		}

		//there is at least one dead block between block and predecessor.
		ActiveBlock* destination = predecessor->next();
		//predecessor->next has to be dead...
		pheet_assert(destination->is_dead());
		//and will take the items from block if it is of same size
		if (block->lvl() == destination->lvl()) {
			//move item pointers from source (block) to destination
			move(block, destination);

			//maintain pointer to last
			bool was_last = block == last;
			block = destination;
			if (was_last) {
				last = drop_dead_blocks(get_last(block));
			}

			//predecessor and predecessor->next have the same size, we can merge them
			if (predecessor->lvl() == destination->lvl()) {
				block = destination->prev()->merge_next();
				return true;
			}
			//otherwise, no need to merge
			pheet_assert(predecessor->lvl() > destination->lvl());
			return true;
		}
		pheet_assert(block->lvl() < destination->lvl());
		return false;
	}

	/**
	 * Put the item in the topmost block.
	 */
	void put(Item& item);

	/**
	 * Move all item pointers from source to destination.
	 *
	 * Source and destination blocks must have the same level. Additionally,
	 * the destination block is assumed to be a dead block.
	 *
	 * On return, source will be a dead and destination will be and active block.
	 */
	void move(ActiveBlock* source, ActiveBlock* destination)
	{
		pheet_assert(destination->is_dead());
		pheet_assert(source->lvl() == destination->lvl());

		//move all items from block source to block destination
		VAIt source_it = m_array.iterator_to(source->offset());
		VAIt end_it = m_array.iterator_to(source->offset() + source->capacity());
		VAIt destination_it = m_array.iterator_to(destination->offset());
		for (; source_it != end_it; source_it++) {
			Item* source_item = *source_it;
			pheet_assert(*destination_it == nullptr);
			*destination_it = source_item;
			*source_it = nullptr;
			destination_it++;
		}

		//change the previously dead to an active block
		destination->set_dead(false);

		//change the previously active to a dead block
		source->set_dead(true);

		//make sure that consecutive dead blocks increase in size
		handle_dead(source);
	}

	/**
	 * Get the last block of the linked list of blocks starting at block.
	 */
	ActiveBlock* get_last(ActiveBlock* block)
	{
		while (block->next()) {
			block = block->next();
		}
		return block;
	}

	/**
	 * Drop all dead blocks at the end of the linked list (until a non-dead
	 * block is encountered)
	 */
	ActiveBlock* drop_dead_blocks(ActiveBlock* last)
	{
		while (last->is_dead()) {
			pheet_assert(last->prev());
			last = last->prev();
			m_array.decrease_capacity(last->next()->capacity());
			delete last->next();
			last->next(nullptr);
		}
		return last;
	}

	ActiveBlock* get_active_successor(ActiveBlock* block)
	{
		ActiveBlock* succ = block;
		do {
			succ = succ->next();
		} while (succ && succ->is_dead());
		if (!succ || succ->is_dead()) {
			return block;
		}
		return succ;
	}

private: //methods to check internal consistency

	bool check_linked_list()
	{
		//iterate through all the blocks in the linked list, checking basic properties
		ActiveBlock* it = first;
		while (it) {
			if (!it->prev()) {
				pheet_assert(it == first);
			} else {
				pheet_assert(it->prev()->next() == it);
				pheet_assert(it->prev()->offset() + it->prev()->capacity() == it->offset());
			}
			if (!it->next()) {
				pheet_assert(it == last);
			} else {
				pheet_assert(it->next()->prev() == it);
			}

			it = it->next();
		}
		return true;
	}

	bool check_blocks()
	{
		return check_blocks(last);
	}

	bool check_blocks(ActiveBlock* it)
	{
		ActiveBlock* prev;
		//iterate through all the blocks in the linked list, starting at the end
		while (it) {
			prev = it->prev();
			//find the closest active predecessor; check dead blocks on the way
			while (prev && prev->is_dead()) {
				if (it == last) {
					//the predecessor (which is dead in this case) of the last block
					//has to be >= the last block
					pheet_assert(prev->lvl() >= last->lvl());
				} else {
					//a predecessing dead block has to be larger than the closest
					//active successor
					pheet_assert(prev->lvl() > it->lvl());
				}
				if (prev->next()->is_dead()) {
					//if the successor is dead too, prev has to be of less or equal size
					pheet_assert(prev->lvl() <= prev->next()->lvl());
				}
				//a dead block has to have a predecessor
				pheet_assert(prev->prev());
				if (!prev->prev()->is_dead()) {
					//if the predecessor is not dead, prev has to have the same
					//size as the non-dead predecessor
					//pheet_assert(prev->prev()->lvl() == prev->lvl());
				}
				prev = prev->prev();
			}

			//if a predecessor was found, it has to be active
			pheet_assert(!prev || !prev->is_dead());
			//we only look at active blocks here
			pheet_assert(!it->is_dead());

			if (prev) {
				if (it == last) {
					//the active predecessor of the last block has to be >= the last
					//block (the last block is not merged until it is full)
					pheet_assert(prev->lvl() >= last->lvl());

				} else {
					//if it is not the last block, the active predecessor has to
					//be larger
					pheet_assert(prev->lvl() > it->lvl());
				}
			}
			it = prev;
		}
		return true;
	}

private:
	ParentTaskStoragePlace* parent_place;
	TaskStorage* task_storage;
	bool created_task_storage;

	ItemMemoryManager items;

	VirtualArray<Item*> m_array;
	PivotQueue m_pivots;

	ActiveBlock* first;
	ActiveBlock* last;

	PerformanceCounters pc;
};

template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::
ParetoLocalityTaskStoragePlace(ParentTaskStoragePlace* parent_place)
	: parent_place(parent_place), pc(parent_place->pc)
{
	//increase capacity of virtual array
	m_array.increase_capacity(MAX_PARTITION_SIZE);
	last = new ActiveBlock(m_array, 0, &m_pivots);
	first = last;
	task_storage = TaskStorage::get(this, parent_place->get_central_task_storage(),
	                                created_task_storage);
}

template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::
~ParetoLocalityTaskStoragePlace()
{
	if (created_task_storage) {
		delete task_storage;
	}
	ActiveBlock* block = last;
	while (block->prev()) {
		block = block->prev();
		delete block->next();
	}
	delete block;
}

template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
void
ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::
push(Strategy&& strategy, T data)
{
	Item& item = items.acquire_item();
	item.data = data;
	item.strategy(std::forward < Strategy && > (strategy));
	item.task_storage = task_storage;
	item.owner(this);

	// Release the item so that other threads can see it.
	item.taken.store(false, std::memory_order_release);

	//put the item in the local data structure and push it to the parent place.
	put(item);
	parent_place->push(&item);
}

template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
void
ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::
put(Item& item)
{
	pheet_assert(!last->next());
	if (!last->try_put(&item)) {
		//last is full. Merge it into previous block
		merge_from_last();

		//increase capacity of virtual array
		m_array.increase_capacity(MAX_PARTITION_SIZE);

		//create new block
		size_t nb_offset = last->offset() + last->capacity();
		ActiveBlock* nb = new ActiveBlock(m_array, nb_offset, &m_pivots);
		nb->prev(last);
		pheet_assert(!last->next());
		last->next(nb);
		last = nb;

		//put the item in the new block
		last->put(&item);
	}
}


template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
typename ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::T
ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::
pop(BaseItem* boundary)
{
	Item* boundary_item = reinterpret_cast<Item*>(boundary);
	while (!boundary_item->is_taken()) {
		ActiveBlock* best_block = nullptr;
		VAIt best_it;
		//iterate through all blocks
		for (ActiveBlock* block = first; block != nullptr; block = block->next()) {
			//only check the block if it is an ActiveBlock
			if (!block->is_dead()) {
				//get the top element
				VAIt top_it = block->top();
				pheet_assert(!block->is_dead());

				//is the block empty?
				if (!top_it.validItem()) {
					/* it->top() returned non-valid iterator, thus no more active
					* items are in block it. */
					continue;
				}
				//We found a new best item
				if (!best_it.validItem() ||
				        top_it->strategy()->prioritize(*(best_it)->strategy())) {
					best_block = block;
					best_it = top_it;
				}
			}
		}

		//if the boundary item was taken in the meantime, pop has to return null...
		if (boundary_item->is_taken() || !best_it.validItem()) {
			return nullable_traits<T>::null_value;
		}
		//..otherwise a best item (and the block it is stored in) has to exist
		pheet_assert(best_block);
		pheet_assert(best_it.validItem());
		T pop = best_block->take(best_it);

		if (best_block->try_shrink()) {
			//TODOMK: not nice to get last block here
			last = drop_dead_blocks(get_last(best_block));
		}

		//merge if necessary
		ActiveBlock* block = get_active_successor(best_block);
		pheet_assert(!last->is_dead());

		if (block == last) {
			merge_from_last();
		} else {
			merge_from(block);
		}

		//If take did not succeed, best_item was taken by another thread in the
		//meantime. Try again.
		if (pop != nullable_traits<T>::null_value) {
			return pop;
		}
	}

	// Linearized to check of boundary item
	return nullable_traits<T>::null_value;
}

template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
typename ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::T
ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::
steal(BaseItem* boundary)
{
	Item* item = reinterpret_cast<Item*>(boundary);
	Self* other_place = item->owner();
	/* This assertion may fail if not all of the tasks generated by a place have been
	 * executed or dropped. */
	pheet_assert(other_place != this);

	//TODOMK: steal only if place to steal from has a lot more items than this place
	if (!item->is_taken()) {
		put(*item);
		parent_place->push(item);

		auto it = other_place->array().begin();
		auto end = other_place->array().end();

		//TODOMK: this works, but may not be very efficient
		for (; end.valid() && it != end; it++) {
			pheet_assert(it.index() < end.index());
			Item* item = *it;
			if (item && !item->is_taken_or_dead()) {
				put(*item);
			}
		}

		//TODOMK: linearization; make sure boundary item is not taken
		return pop(boundary);
	}

	return nullable_traits<T>::null_value;
}

template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
void
ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::
clean_up()
{

}
} /* namespace pheet */

#endif /* PARETOLOCALITYTASKSTORAGEPLACE_H_ */
