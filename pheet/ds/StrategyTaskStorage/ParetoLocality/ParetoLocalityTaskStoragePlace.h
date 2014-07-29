/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef PARETOLOCALITYTASKSTORAGEPLACE_H_
#define PARETOLOCALITYTASKSTORAGEPLACE_H_

#include "ParetoLocalityTaskStorageBlock.h"
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
	typedef ParetoLocalityTaskStorageBlock<Item, MAX_PARTITION_SIZE> Block;
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
	 * Put the item in the topmost block.
	 */
	void put(Item& item);

	/**
	 * Ensure correct ordering of dead blocks, starting from block.
	 *
	 * After a call to this method, the sequence of dead blocks starting at "block"
	 * and ending at the nearest non-dead predecessor (non-dead-pred) of "block"
	 * will be ordered s.t. a preceeding dead block (d_i) is of less or equal size
	 * than any suceeding dead block in the sequence, i.e.,
	 *
	 * non-dead-pred-lvl() <= d_j.lvl() <= ... <= d_i.lvl <= block.lvl()
	 *
	 * The returned block (rb) is a predecessor of block, s.t. rb is either not dead
	 * or does not require reordering.
	 *
	 */
	Block* reorder_dead_predecesors(Block* block)
	{
		pheet_assert(block->is_dead());
		Block* predecessor = block->prev();
		//block must have a predecessor
		pheet_assert(predecessor);

		if (predecessor->is_dead() && predecessor->lvl() > block->lvl()) {
			predecessor = swap_dead(predecessor, block);
			predecessor = reorder_dead_predecesors(predecessor);
		}
		return predecessor;
	}

	void reorder_dead_blocks(Block* source)
	{
		while (source && source->is_dead()) {
			Block* next = source->next();
			reorder_dead_predecesors(source);
			source = next;
		}
	}

	/**
	 * Swap two dead blocks, where "predecessor" is the immediate predecessor of "block".
	 */
	Block* swap_dead(Block* predecessor, Block* block)
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
		Block* new_predecessor = new Block(m_array, &m_pivots, offset, lvl, true);

		pheet_assert(block->offset() >= (predecessor->capacity() - block->capacity()));
		offset = block->offset() - (predecessor->capacity() - block->capacity());
		lvl = predecessor->lvl();
		Block* new_block = new Block(m_array, &m_pivots, offset, lvl, true);

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
		//TODOMK: can we unify merge_from_last and merge_from?
		//merge recursively, if previous block has same level, starting from last
		if (merge_recursively(last)) {

			//repartition block that resulted from merge
			last->partition();

			//TODMK: simplify
			//try to shrink the block
			size_t shrunk_by = last->shrink();
			if (shrunk_by) {
				//reset last and drop dead blocks at the end of the list
				m_array.decrease_capacity(shrunk_by);
			}
		}
		pheet_assert(!last->next());
		pheet_assert(!last->is_dead());
	}

	/**
	 * Starting from block, merge preceeding blocks as long as necessary. If block
	 * was merged, partition it and maintain pointers to block and last
	 */
	void merge_from(Block*& block)
	{
		if (block == last) {
			merge_from_last();
			return;
		}
		if (merge_recursively(block)) {
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
	bool merge_recursively(Block*& block)
	{
		pheet_assert(!block->is_dead());
		bool merged = false;
		while (merge(block)) {
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
	bool merge(Block*& block)
	{
		pheet_assert(!block->is_dead());
		pheet_assert(block != insert);

		//If block does not have a predecessor, no merge is required.
		if (!block->prev()) {
			return false;
		}

		//Else, find active_pred, the closest non-dead predecessor of block. Such a
		//block has to exist.
		Block* predecessor = block->prev();
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
		Block* destination = predecessor->next();
		//predecessor->next has to be dead...
		pheet_assert(destination->is_dead());
		//and will take the items from block if it is of same size
		if (block->lvl() == destination->lvl()) {
			//move item pointers from source (block) to destination
			move(block, destination, true);
			//maintain pointer to last
			bool was_last = block == last;
			block = destination;
			if (was_last) {
				last = drop_dead_blocks_at_end(get_last(block));
			}
			//predecessor and predecessor->next have the same size, we can merge them
			if (predecessor->lvl() == destination->lvl()) {
				block = destination->prev()->merge_next();
				reorder_dead_blocks(block->next());
				return true;
			}
			//otherwise, no need to merge
			pheet_assert(predecessor->lvl() > destination->lvl());
			return true;
		}
		pheet_assert(block->lvl() < destination->lvl());
		reorder_dead_blocks(destination);
		return false;
	}

	/**
	 * Move all item pointers from source to destination.
	 *
	 * Source and destination blocks must have the same level. Additionally,
	 * the destination block is assumed to be a dead block.
	 *
	 * On return, source will be a dead and destination will be and active block.
	 */
	void move(Block* source, Block* destination, bool reorder_dead)
	{
		pheet_assert(destination->is_dead());
		pheet_assert(source->lvl() == destination->lvl());

		//move all items from block source to block destination
		VAIt source_it = source->start();
		VAIt end_it = source->end();
		VAIt destination_it = destination->start();
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
		if (reorder_dead) {
			reorder_dead_blocks(source);
		}
	}

	/**
	 * Get the last block of the linked list of blocks starting at block.
	 */
	Block* get_last(Block* block)
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
	Block* drop_dead_blocks_at_end(Block* last)
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

	Block* get_active_successor(Block* block)
	{
		Block* succ = block;
		do {
			succ = succ->next();
		} while (succ && succ->is_dead());
		if (!succ || succ->is_dead()) {
			return block;
		}
		return succ;
	}

	/**
	 * Set block dead if possible.
	 *
	 * Returns true if block was set dead; otherwise false
	 */
	bool try_set_dead(Block* block)
	{
		pheet_assert(block != insert);
		//never set the first block in the doubly linked list dead
		if (!block->prev()) {
			return false;
		}
		/* only set a block to dead if it does not have a dead successor
		 (this would require reordering blocks that have not been traversed yet) */
		if (block->next() && block->next()->is_dead()) {
			return false;
		}
		block->set_dead(true);
		return true;
	}

private: //methods to check internal consistency

	bool check_linked_list()
	{
		//iterate through all the blocks in the linked list, checking basic properties
		Block* it = insert;
		while (it) {
			if (!it->prev()) {
				pheet_assert(it == insert || it == insert->next());
			} else {
				pheet_assert(it->prev()->next() == it);
				pheet_assert(it->prev()->offset() + it->prev()->capacity() == it->offset());
			}
			if (!it->next()) {
				pheet_assert(it == last);
			} else {
				if (it == insert) {
					pheet_assert(it->next()->prev() == nullptr);
				} else {
					pheet_assert(it->next()->prev() == it);
				}
			}

			it = it->next();
		}
		return true;
	}

	bool check_blocks()
	{
		return check_blocks(nullptr);
	}

	bool check_blocks(Block* stop_at)
	{
		Block* prev;
		//iterate through all the blocks in the linked list, starting at the end
		Block* it = last;
		while (it != stop_at) {
			prev = it->prev();
			//find the closest active predecessor; check dead blocks on the way
			while (prev && prev->is_dead() && prev != stop_at) {
				//a predecessing dead block has to be larger than the closest
				//active successor
				pheet_assert(prev->lvl() > it->lvl());
				if (prev->next()->is_dead()) {
					//if the successor is dead too, prev has to be of less or equal size
					pheet_assert(prev->lvl() <= prev->next()->lvl());
				}
				//a dead block has to have a predecessor
				pheet_assert(prev->prev());
				if (!prev->prev()->is_dead()) {
					//if the predecessor is not dead, prev has to have the same
					//size as the non-dead predecessor
					pheet_assert(prev->prev()->lvl() >= prev->lvl());
				}
				prev = prev->prev();
			}

			if (prev != stop_at) {
				//if a predecessor was found, it has to be active
				pheet_assert(!prev || !prev->is_dead());
				//we only look at active blocks here
				pheet_assert(!it->is_dead());

				if (prev) {
					//the active predecessor of a block has to be larger than the block
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

	Block* insert;
	Block* last;

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
	last = new Block(m_array, &m_pivots, 0, 0, false, 0);
	insert = last;
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
	if (last != insert) {
		Block* block = last;
		while (block->prev()) {
			block = block->prev();
			delete block->next();
		}
		delete block;
	}
	delete insert;
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
	pheet_assert(!insert->prev());
	if (!insert->try_put(&item)) {
		/*insert is full. Move data to new block at the end and merge with
		  previous block, if necessary*/

		//increase capacity of virtual array
		m_array.increase_capacity(MAX_PARTITION_SIZE);

		//create new block
		size_t nb_offset = last->offset() + last->capacity();
		Block* nb = new Block(m_array, &m_pivots, nb_offset, 0, true);
		if (last != insert) {
			nb->prev(last);
		}
		pheet_assert(!last->next());
		last->next(nb);
		last = nb;

		//move all data from insert to last
		move(insert, last, false);
		//partition last (it's unlikely that shrinking would be possible; but we do
		//not want to reduce the level of a lvl 0 block anyway)
		last->partition();
		//merge blocks, if necessary
		merge_from_last();

		//re-initialize insert
		insert->reinitialize();
		insert->set_dead(false);
		//put the item in the (now empty) insert block
		insert->put(&item);
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
		Block* best_block = nullptr;
		VAIt best_it;
		check_blocks();
		//iterate through all blocks
		for (Block* block = insert; block != nullptr; block = block->next()) {

			//get the top element
			VAIt top_it = block->top();
			pheet_assert(!block->is_dead());

			//We found a new best item
			if (!best_it.validItem()  || (top_it.validItem() &&
			                              top_it->strategy()->prioritize(*(best_it)->strategy()))) {
				best_block = block;
				best_it = top_it;
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

	if (!item->is_taken()) {
		put(*item);
		parent_place->push(item);

		//steal only if place to steal from has a lot more items than this place
		//TODOMK: choose good factor
		size_t other_cap = other_place->array().capacity();
		size_t this_cap = this->array().capacity();
		if (other_cap > 2 * this_cap) {
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
			//std::cerr << "steal!\n";
		}/* else {
			std::cerr << "no steal..other place: " << other_cap << " this place: " << this_cap << " \n";
		}*/

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
