/*
 * Copyright Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef PARETOLOCALITYTASKSTORAGEDEADBLOCK_H
#define PARETOLOCALITYTASKSTORAGEDEADBLOCK_H

#include "ParetoLocalityTaskStorageBaseBlock.h"

namespace pheet
{

template<class Item, size_t MAX_PARTITION_SIZE>
class ParetoLocalityTaskStorageDeadBlock
	: public ParetoLocalityTaskStorageBaseBlock<Item, MAX_PARTITION_SIZE>
{
public:
	typedef ParetoLocalityTaskStorageBaseBlock<Item, MAX_PARTITION_SIZE> BaseBlock;
	typedef typename BaseBlock::T T;
	typedef typename BaseBlock::VA VA;
	typedef typename BaseBlock::VAIt VAIt;

	ParetoLocalityTaskStorageDeadBlock(VA& array, size_t offset, size_t lvl)
		: ParetoLocalityTaskStorageBaseBlock<Item, MAX_PARTITION_SIZE>(array, offset, lvl)
	{

	}
};

}


#endif // PARETOLOCALITYTASKSTORAGEDEADBLOCK_H
