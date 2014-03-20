/*
 * NumaStrategyPrefixSum.h
 *
 *  Created on: Mar 07, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef NUMASTRATEGYPREFIXSUM_H_
#define NUMASTRATEGYPREFIXSUM_H_

#include <pheet/pheet.h>
#include <pheet/misc/align.h>
#include "NumaStrategyPrefixSumOffsetTask.h"
#include "../RecursiveParallel2/RecursiveParallelPrefixSum2LocalSumTask.h"
#include "NumaStrategyPrefixSumPerformanceCounters.h"

#include <iostream>
#include <atomic>

namespace pheet {

template <class Pheet, size_t BlockSize, bool Inclusive>
class NumaStrategyPrefixSumImpl : public Pheet::Task {
public:
	typedef NumaStrategyPrefixSumImpl<Pheet, BlockSize, Inclusive> Self;
	typedef NumaStrategyPrefixSumImpl<Pheet, BlockSize, false> ExclusiveSelf;
	typedef NumaStrategyPrefixSumOffsetTask<Pheet, BlockSize, Inclusive> OffsetTask;
	typedef RecursiveParallelPrefixSum2LocalSumTask<Pheet, BlockSize, Inclusive> LocalSumTask;
	typedef NumaStrategyPrefixSumPerformanceCounters<Pheet> PerformanceCounters;

	NumaStrategyPrefixSumImpl(unsigned int* data, size_t length)
	:data(data), length(length), step(1), root(true) {

	}

	NumaStrategyPrefixSumImpl(unsigned int* data, size_t length, size_t step, bool root)
	:data(data), length(length), step(step), root(root) {

	}

	NumaStrategyPrefixSumImpl(unsigned int* data, size_t length, PerformanceCounters& pc)
	:data(data), length(length), step(1), root(true), pc(pc) {

	}

	NumaStrategyPrefixSumImpl(unsigned int* data, size_t length, size_t step, bool root, PerformanceCounters& pc)
	:data(data), length(length), step(step), root(root), pc(pc) {

	}

	virtual ~NumaStrategyPrefixSumImpl() {}

	virtual void operator()() {
		if(length <= BlockSize) {
			unsigned int sum = 0;
			for(size_t i = 0; i < length; ++i) {
				unsigned int tmp = data[i];
				if(Inclusive) {
					sum += tmp;
					data[i] = sum;
				}
				else {
					data[i] = sum;
					sum += tmp;
				}
			}
		}
		else {
			size_t num_blocks = ((length - 1) / BlockSize) + 1;

			aligned_data<unsigned int, 64> auxiliary_data(num_blocks);
			aligned_data<procs_t, 64> numa_nodes(num_blocks);

			std::atomic<size_t> sequential(0);

			numa_nodes.ptr()[0] = Pheet::get_place()->get_data_numa_node_id(data);
			// Calculate offsets
			Pheet::template finish<OffsetTask>(data, auxiliary_data.ptr(), numa_nodes.ptr(), num_blocks, length, 0, sequential, Pheet::get_place());
			size_t seq = sequential.load(std::memory_order_relaxed);
			pc.blocks.add(num_blocks);
			pc.preprocessed_blocks.add(seq);
			if(seq < num_blocks) {
				// Prefix sum on offsets
				pheet_assert(seq > 0);
				Pheet::template finish<ExclusiveSelf>(auxiliary_data.ptr() + seq - 1, num_blocks + 1 - seq, pc);

				// Calculate local prefix sums based on offset
				Pheet::template finish<LocalSumTask>(data + (seq * BlockSize), auxiliary_data.ptr() + seq, num_blocks - seq, length - (seq * BlockSize));
			}
		}
	}

	static char const name[];

private:
	unsigned int* data;
	size_t length;
	size_t step;
	bool root;
	PerformanceCounters pc;
};

template <class Pheet, size_t BlockSize, bool Inclusive>
char const NumaStrategyPrefixSumImpl<Pheet, BlockSize, Inclusive>::name[] = "NumaStrategyPrefixSum";

template <class Pheet>
using NumaStrategyPrefixSum = NumaStrategyPrefixSumImpl<Pheet, 4096, true>;

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUM_H_ */
