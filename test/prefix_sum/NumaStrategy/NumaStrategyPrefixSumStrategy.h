/*
 * NumaStrategyPrefixSumStrategy.h
 *
 *  Created on: 07.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0
 */

#ifndef NUMASTRATEGYPREFIXSUMSTRATEGY_H_
#define NUMASTRATEGYPREFIXSUMSTRATEGY_H_

#include <pheet/ds/StrategyTaskStorage/KLSMLocality/KLSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet, size_t BlockSize>
class NumaStrategyPrefixSumStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef NumaStrategyPrefixSumStrategy<Pheet, BlockSize> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	typedef KLSMLocalityTaskStorage<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	NumaStrategyPrefixSumStrategy() {}

	NumaStrategyPrefixSumStrategy(unsigned int* block_offset, size_t block_id, Place* owner, bool in_order)
	: block_id(block_id), owner(owner), in_order(in_order) {}

	NumaStrategyPrefixSumStrategy(Self& other)
	: BaseStrategy(other), block_offset(other.block_offset), block_id(other.block_id), owner(other.owner) {}

	NumaStrategyPrefixSumStrategy(Self&& other)
	: BaseStrategy(other), block_offset(other.block_offset), block_id(other.block_id), owner(other.owner) {}

	~NumaStrategyPrefixSumStrategy() {}

	Self& operator=(Self&& other) {
		block_offset = other.block_offset;
		block_id = other.block_id;
		owner = other.owner;
		return *this;
	}


	bool prioritize(Self& other) {
		Place* p = Pheet::get_place();

		bool is_local = p->is_partially_numa_local(block_offset, BlockSize);
		bool is_other_local = p->is_partially_numa_local(other.block_offset, BlockSize);

		if(is_local != is_other_local) {
			return is_local;
		}

		if(owner == p) {
			if(other.owner != p) {
				return true;
			}
			return block_id < other.block_id;
		}
		else if(other.owner == p) {
			return false;
		}
		return block_id > other.block_id;
	}

	bool can_call(TaskStoragePlace*) {
		return in_order;
	}

	bool dead_task() {
		return false;
	}

	size_t get_k() {
		return 8;
	}

private:
	unsigned int* block_offset;
	size_t block_id;
	Place* owner;
	bool in_order;
};

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUMSTRATEGY_H_ */
