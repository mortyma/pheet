/*
 * NumaStrategyPrefixSumPerformanceCounters.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef NUMASTRATEGYPREFIXSUMPERFORMANCECOUNTERS_H_
#define NUMASTRATEGYPREFIXSUMPERFORMANCECOUNTERS_H_

#include <pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h>
#include <iostream>

namespace pheet {

template <class Pheet>
class NumaStrategyPrefixSumPerformanceCounters {
public:
	typedef NumaStrategyPrefixSumPerformanceCounters<Pheet> Self;

	NumaStrategyPrefixSumPerformanceCounters() {}
	NumaStrategyPrefixSumPerformanceCounters(Self& other)
	:blocks(other.blocks), preprocessed_blocks(other.preprocessed_blocks) {}
	NumaStrategyPrefixSumPerformanceCounters(Self&& other)
	:blocks(other.blocks), preprocessed_blocks(other.preprocessed_blocks) {}
	~NumaStrategyPrefixSumPerformanceCounters() {}

	static void print_headers() {
		BasicPerformanceCounter<Pheet, prefix_sum_log_pf_blocks>::print_header("blocks\t");
		BasicPerformanceCounter<Pheet, prefix_sum_log_pf_preprocessed_blocks>::print_header("preprocessed_blocks\t");
	}
	void print_values() {
		blocks.print("%lu\t");
		preprocessed_blocks.print("%lu\t");
	}

	BasicPerformanceCounter<Pheet, prefix_sum_log_pf_blocks> blocks;
	BasicPerformanceCounter<Pheet, prefix_sum_log_pf_preprocessed_blocks> preprocessed_blocks;
};

} /* namespace pheet */
#endif /* NUMASTRATEGYPREFIXSUMPERFORMANCECOUNTERS_H_ */
