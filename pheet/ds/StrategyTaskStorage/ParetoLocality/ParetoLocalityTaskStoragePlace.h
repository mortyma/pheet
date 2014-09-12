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

#define MAX_PARTITION_SIZE (128)

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
	 * Starting from block, merge with predecessing blocks as long as neccessary.
	 *
	 * A merge with the predecessor is performed if the predecessor exists and
	 *   (i)  the predecessing block has the same level as this block, or
	 *   (ii) the predecessing block is dead (regardless of the level of the
	 *        predecessing block).
	 *
	 * On return, block will point to the
	 * resulting block, which will be partitioned. Pointer to last is maintained.
	 */
	void merge_from(Block*& block)
	{
		pheet_assert(!block->is_dead());
		bool merged = false;
		Block* pred;
		while ((pred = block->prev()) && (pred->is_dead() || pred->lvl() == block->lvl())) {
			// If block is the last block in the linked list and being merged with
			//predecessor, the later will be the last block in the linked list after the merge.
			if (block == last) {
				last = pred;
			}
			//merge predecessor if it is a dead block, or a non-dead block of same lvl
			block = pred->merge_next();
			merged = true;
		}
		if (merged) {
			//repartition block that resulted from merge
			block->partition();
			if (last == block) {
				//shrink the last block, if possible
				size_t shrunk_by = last->shrink();
				if (shrunk_by > 0) {
					m_array.decrease_capacity(shrunk_by);
				}
			}
			//Shrinking non-last blocks might result in another merge
		}
		pheet_assert(!last->next());
		pheet_assert(!last->is_dead());
	}

	/**
	 * Move all item pointers from source to destination.
	 *
	 * Source and destination blocks must have the same level. Additionally,
	 * the destination block is assumed to be a dead block.
	 *
	 * On return, source will be a dead and destination will be and active block.
	 */
	void move(Block* source, Block* destination)
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
	}

	/**
	 * Drop all dead blocks at the end of the linked list (until a non-dead
	 * block is encountered)
	 */
	Block* drop_dead_blocks_at_end(Block* last)
	{
		size_t shrink_by = 0;
		while (last->is_dead() && last != insert) {
			if (!last->prev()) {
				last = insert;
			} else {
				last = last->prev();
			}
			shrink_by += last->next()->capacity();
			delete last->next();
			last->next(nullptr);
		}
		m_array.decrease_capacity(shrink_by);
		return last;
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
			if (prev && prev != stop_at) {
				//the predecessor of a block has to be larger than the block
				pheet_assert(prev->lvl() >= it->lvl());
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
		pheet_assert(!last->is_dead());
		last->next(nb);
		last = nb;

		//move all data from insert to last
		move(insert, last);
		//merge blocks, if necessary
		merge_from(last);

		//re-initialize insert
		insert->reinitialize();

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
		//iterate through all blocks
		for (Block* block = insert; block != nullptr; block = block->next()) {
			if (!block->is_dead()) {
				//get the top element
				VAIt top_it = block->peek();
				if (block != insert) { //the insert block is never changed
					//If top_it is not a valid item, block contains no more valid items.
					//We set the block dead.
					if (!top_it.validItem()) {
						block->set_dead(true);
						if (block == last) {
							//if we set the last block dead, drop all the dead blocks
							//at the end of the list. Otherwise, we will have to merge
							//block and block->next in the next iteration of the loop.
							block = last = drop_dead_blocks_at_end(last);
						}
					} else {
						//block is not dead.
						if (block == last) {
							//shrink block as far as possible
							block->shrink();
						} else {
							//Reduce the lvl of block if possible. Note that before this operation,
							//we know that block->prev()->lvl() > block->lvl() > block->next()->lvl.
							//If block == last, we can simply shrink it. Otherwise, we reduce
							//its level; afterwards, we may have block->lvl() == block->next()->lvl.
							//In that case we have to merge block and block->prev().
							block->try_reduce_lvl();
							pheet_assert(block->capacity() == block->end().index() - block->start().index());
						}
						//Merge with previous blocks if neccessary,
						//i.e., if a sequence of previous blocks is all dead blocks or
						//of the same lvl as block.
						if (block->prev() && (block->prev()->is_dead()
						                      || block->prev()->lvl() == block->lvl())) {
							merge_from(block);
						}
					}
				}
				//We found a new best item
				if (!best_it.validItem()  || (top_it.validItem() &&
				                              top_it->strategy()->prioritize(*(best_it)->strategy()))) {
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
			for (; it != end; it++) {
				Item* item = *it;
				if (item && !item->is_taken_or_dead()) {
					put(*item);
				}
			}
			//std::cerr << "steal!\n";
		}/* else {
			std::cerr << "no steal..other place: " << other_cap << " this place: " << this_cap << " \n";
		}*/

		// Now that we have spied some items from the owner of the boundary, we can just pop an item
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
