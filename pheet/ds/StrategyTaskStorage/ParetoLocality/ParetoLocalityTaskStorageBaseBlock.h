/*
 * Copyright Jakob Gruber, Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef PARETOLOCALITYTASKSTORAGEBASEBLOCK_H_
#define PARETOLOCALITYTASKSTORAGEBASEBLOCK_H_

#include "ItemComparator.h"
#include "PartitionPointers.h"
#include "PivotQueue.h"
#include "VirtualArray.h"

#include <algorithm>
#include <cmath>
#include <random>

/* Maximum number of attempts to partition a block s.t. after partitioning the
 * block, the right-most partition (excluding dead) contains at least 1 item. */
#define MAX_ATTEMPTS_TO_PARTITION (2)

/* Maximum number of attempts to generate a pivot element s.t. an equal element
 * is not yet on the PivotQueue. */
#define MAX_ATTEMPTS_TO_GENERATE_PIVOT (2)

/* Number of sample items to draw for the generation of a pivot element. */
#define NR_SAMPLES_FOR_PIVOT_GENERATION (5)

/* Maximum number of attemps to sample the NR_SAMPLES_FOR_PIVOT_GENERATION items.
 * If a sampled item is invalid (null, taken or dead), the sampling failed and is
 * retried */
#define MAX_ATTEMPTS_TO_SAMPLE (10)

namespace pheet
{

template<class Item, size_t MAX_PARTITION_SIZE>
class ParetoLocalityTaskStorageBaseBlock
{
public:
	typedef typename Item::T T;
	typedef VirtualArray<Item*> VA;
	typedef typename VA::VirtualArrayIterator VAIt;

	ParetoLocalityTaskStorageBaseBlock(VirtualArray<Item*>& array, size_t offset)
		: m_lvl(0), m_data(array), m_offset(offset), m_next(nullptr)
	{
		m_capacity = MAX_PARTITION_SIZE * pow(2, this->m_lvl);
	}

	virtual ~ParetoLocalityTaskStorageBaseBlock()
	{

	}

	size_t lvl() const
	{
		return m_lvl;
	}

	size_t capacity() const
	{
		return m_capacity;
	}

	size_t offset() const
	{
		return m_offset;
	}


	ParetoLocalityTaskStorageBaseBlock* prev() const
	{
		return m_prev;
	}

	void prev(ParetoLocalityTaskStorageBaseBlock* b)
	{
		m_prev = b;
	}

	ParetoLocalityTaskStorageBaseBlock* next() const
	{
		return m_next.load(std::memory_order_acquire);
	}

	void next(ParetoLocalityTaskStorageBaseBlock* b)
	{
		m_next.store(b, std::memory_order_release);
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
	virtual VAIt top() = 0;

	/**
	 * Take the given item and return its data.
	 *
	 * An item that is taken is marked for deletion/reuse and will not be returned
	 * via a call to top() anymore.
	 */
	T take(VAIt item)
	{
		pheet_assert(*item);
		T data = item->take();
		/* Set the Item to nullptr in the VirtualArray so other threads
		   and operations don't see it any more. Memory manager will take care of
		   deleting the Item */
		*item = nullptr;
		return data;
	}

protected:
	//lvl is the actual size of this block
	size_t m_lvl;

	VirtualArray<Item*>& m_data;
	size_t m_offset;
	size_t m_capacity;

	std::atomic<ParetoLocalityTaskStorageBaseBlock*> m_next;
	ParetoLocalityTaskStorageBaseBlock* m_prev = nullptr;

};

} /* namespace pheet */

#endif /* PARETOLOCALITYTASKSTORAGEBASEBLOCK_H_ */
