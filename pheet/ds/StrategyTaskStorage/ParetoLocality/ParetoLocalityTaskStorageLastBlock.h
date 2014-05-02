/*
 * Copyright Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef PARETOLOCALITYTASKSTORAGELASTBLOCK_H
#define PARETOLOCALITYTASKSTORAGELASTBLOCK_H

#include "ParetoLocalityTaskStorageActiveBlock.h"

namespace pheet
{

template<class Item, size_t MAX_PARTITION_SIZE>
class ParetoLocalityTaskStorageLastBlock
	: public ParetoLocalityTaskStorageActiveBlock<Item, MAX_PARTITION_SIZE>
{
public:
	typedef ParetoLocalityTaskStorageActiveBlock<Item, MAX_PARTITION_SIZE> ActiveBlock;
	typedef typename ActiveBlock::T T;
	typedef typename ActiveBlock::VA VA;
	typedef typename ActiveBlock::VAIt VAIt;

	ParetoLocalityTaskStorageLastBlock(VA& array, size_t offset, PivotQueue* pivots)
		: ParetoLocalityTaskStorageActiveBlock<Item, MAX_PARTITION_SIZE>(array, offset, pivots)
	{

	}

	bool try_put(Item* item)
	{
		if (m_partitions->end().index(m_offset) == m_capacity) {
			return false;
		}
		put(item);
		return true;
	}

	void put(Item* item)
	{
		pheet_assert(m_partitions->end().index(m_offset) < m_capacity);
		//we only put data in a lvl 0 block
		pheet_assert(m_lvl == 0);
		//no CAS needed, since only the owning thread writes to local VirtualArray
		*(m_partitions->end()) = item;
		m_partitions->increment_end();
	}


private:
	using ActiveBlock::m_data;
	using ActiveBlock::m_capacity;
	using ActiveBlock::m_lvl;
	using ActiveBlock::m_offset;
	using ActiveBlock::m_partitions;
};

} //namespace pheet
#endif // PARETOLOCALITYTASKSTORAGELASTBLOCK_H
