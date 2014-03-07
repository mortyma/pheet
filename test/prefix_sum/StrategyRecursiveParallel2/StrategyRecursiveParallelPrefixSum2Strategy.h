/*
 * StrategyRecursiveParallelPrefixSum2Strategy.h
 *
 *  Created on: 07.03.2014
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0
 */

#ifndef STRATEGYRECURSIVEPARALLELPREFIXSUM2STRATEGY_H_
#define STRATEGYRECURSIVEPARALLELPREFIXSUM2STRATEGY_H_

#include <pheet/ds/StrategyTaskStorage/LSMLocality/LSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet>
class StrategyRecursiveParallelPrefixSum2Strategy : public Pheet::Environment::BaseStrategy {
public:
	typedef StrategyRecursiveParallelPrefixSum2Strategy<Pheet> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	typedef LSMLocalityTaskStorage<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	StrategyRecursiveParallelPrefixSum2Strategy() {}

	StrategyRecursiveParallelPrefixSum2Strategy(size_t block_id, Place* in_order)
	: block_id(block_id), in_order(in_order) {}

	StrategyRecursiveParallelPrefixSum2Strategy(Self& other)
	: BaseStrategy(other), block_id(other.block_id), in_order(other.in_order) {}

	StrategyRecursiveParallelPrefixSum2Strategy(Self&& other)
	: BaseStrategy(other), block_id(other.block_id), in_order(other.in_order) {}

	~StrategyRecursiveParallelPrefixSum2Strategy() {}

	Self& operator=(Self&& other) {
		block_id = other.block_id;
		in_order = other.in_order;
		return *this;
	}


	bool prioritize(Self& other) {
		if(in_order != nullptr || other.in_order != nullptr) {
			Place* p = Pheet::get_place();
			if(in_order == p) {
				if(other.in_order != p) {
					return true;
				}
				return block_id < other.block_id;
			}
			else if(other.in_order == p) {
				return true;
			}
		}
		return block_id > other.block_id;
	}

	bool can_call(TaskStoragePlace*) {
		return false;
	}

	bool dead_task() {
		return false;
	}

private:
	size_t block_id;
	Place* in_order;
};

} /* namespace pheet */
#endif /* STRATEGYRECURSIVEPARALLELPREFIXSUM2STRATEGY_H_ */
