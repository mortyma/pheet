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
	  TODOMK
	 * A merge is required if:
	 * - block->prev() exists
	 * - and has the same level (equal to same capacity) as block
	 */
	ActiveBlock* merge_required(ActiveBlock* block);

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
	 *
	 */
	void move(ActiveBlock* source, ActiveBlock* destination)
	{
		pheet_assert(destination->is_dead());
		pheet_assert(source->lvl() == destination->lvl());

		//TODOMK: remove debug code
		for (ActiveBlock* it = source->prev(); it != destination; it = it->prev()) {
			//all blocks between source and destination must be dead.
			pheet_assert(it->is_dead());
			//Additionally, the level of these dead blocks decreases (non-strictly)
			//monotonically
			pheet_assert(it->lvl() <= it->next()->lvl());
		}

		VAIt source_it = m_array.iterator_to(source->offset());
		VAIt end_it = m_array.iterator_to(source->offset() + source->capacity());
		VAIt destination_it = m_array.iterator_to(destination->offset());
		for (; source_it != end_it; source_it++) {
			Item* source = *source_it;
			Item* destination = *destination_it;
			pheet_assert(destination == nullptr);
			*destination_it = source;
			*source_it = nullptr;
			destination_it++;
		}

		//change the previously dead to an active block
		destination->set_dead(false);

		//change the previously active to a dead block
		source->set_dead(true);
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

private: //methods to check internal consistency

	void check_blocks()
	{
#ifdef PHEET_DEBUG_MODE
		pheet_assert(!last->next());
		pheet_assert(!last->is_dead());
		pheet_assert(!first->prev());

		ActiveBlock* it = last;
		ActiveBlock* prev;
		while (it) {
			do {
				prev = it->prev();
				if (prev && prev->is_dead()) {
					if (it == last) {
						pheet_assert(prev->lvl() >= it->lvl());
					} else {
						pheet_assert(prev->lvl() > it->lvl());
					}
					if (prev->next()->is_dead()) {
						pheet_assert(prev->lvl() <= prev->next()->lvl());
					}
				}
			} while (prev && prev->is_dead());

			if (prev) {
				if (it == last) {
					pheet_assert(prev->lvl() >= it->lvl());
				} else {
					pheet_assert(prev->lvl() > it->lvl());
				}
			}
			it = it->prev();
		}
#endif
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
	check_blocks();

	pheet_assert(!last->next());
	if (!last->try_put(&item)) {
		ActiveBlock* block = last;
		bool merged = false;
		//merge recursively, if previous block has same level
		while (ActiveBlock* merge = merge_required(block)) {
			block = merge->merge_next();
			merged = true;
		}
		//did we merge?
		if (merged) {
			//repartition block that resulted from merge
			block->partition();

			last = get_last(block);
			last = drop_dead_blocks(last);

		}
		pheet_assert(last == block);

		check_blocks();

		//increase capacity of virtual array
		m_array.increase_capacity(MAX_PARTITION_SIZE);
		//create new block
		size_t nb_offset = block->offset() + block->capacity();
		ActiveBlock* nb = new ActiveBlock(m_array, nb_offset, &m_pivots);
		nb->prev(block);
		pheet_assert(!block->next());
		block->next(nb);
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
	check_blocks();

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
				//TODOMK: make sure we do not create a sequence of dead blocks
				//TODOMK: check if we need to merge

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

		check_blocks();

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

template < class Pheet,
           class TaskStorage,
           class ParentTaskStoragePlace,
           class Strategy >
typename ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::ActiveBlock*
ParetoLocalityTaskStoragePlace<Pheet, TaskStorage, ParentTaskStoragePlace, Strategy>::
merge_required(ActiveBlock* block)
{
	// If block does not have a predecessor, no merge is required.
	if (!block->prev()) {
		return nullptr;
	}

	// Else, find active_pred, the closest non-dead predecessor of block. Such a
	//block has to exist.
	ActiveBlock* predecessor = block->prev();
	/*bool move_required = false;
	while (predecessor->is_dead()) {
		predecessor = predecessor->prev();
		//there is at least one dead block between block and active_pred.
		move_required = true;
	}*/

	//if active predecessor is of same level as block, we can merge them
	if (block->lvl() == predecessor->lvl()) {
		/*if (move_required) {
			pheet_assert(false);
			ActiveBlock* destination = predecessor->next();
			pheet_assert(predecessor->lvl() == destination->lvl());
			//move item pointers from source (block) to destination
			move(block, destination);
			if (last == block) {
				while (last != destination) {
					pheet_assert(last->is_dead());
					m_array.decrease_capacity(last->capacity());
					last = last->prev();
					delete last->next();
					last->next(nullptr);
				}

				last = destination->prev();
			}
			return destination->prev();
		}*/
		return predecessor;
	}
	return nullptr;
}

} /* namespace pheet */

#endif /* PARETOLOCALITYTASKSTORAGEPLACE_H_ */
