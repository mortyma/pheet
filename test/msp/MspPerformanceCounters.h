/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef MSPPERFORMANCECOUNTERS_H_
#define MSPPERFORMANCECOUNTERS_H_

#include "pheet/primitives/PerformanceCounter/Basic/BasicPerformanceCounter.h"
#include <pheet/primitives/PerformanceCounter/Time/LastTimePerformanceCounter.h>

namespace pheet
{

template <class Pheet>
class MspPerformanceCounters
{
private:
	static const bool msp_count_dead_tasks = true;
	static const bool msp_count_actual_tasks = true;

public:
	typedef MspPerformanceCounters<Pheet> Self;

	MspPerformanceCounters()
	{
	}

	MspPerformanceCounters(Self& other)
		: num_dead_tasks(other.num_dead_tasks),
		  num_actual_tasks(other.num_actual_tasks)
	{
	}

	MspPerformanceCounters(Self&& other)
		: num_dead_tasks(std::move(other.num_dead_tasks)),
		  num_actual_tasks(std::move(other.num_actual_tasks))
	{
	}

	static void print_headers()
	{
		BasicPerformanceCounter<Pheet, msp_count_dead_tasks>::print_header("num_dead_tasks\t");
		BasicPerformanceCounter<Pheet, msp_count_actual_tasks>::print_header("num_actual_tasks\t");
	}

	void print_values()
	{
		num_dead_tasks.print("%d\t");
		num_actual_tasks.print("%d\t");
	}

	BasicPerformanceCounter<Pheet, msp_count_dead_tasks> num_dead_tasks;
	BasicPerformanceCounter<Pheet, msp_count_actual_tasks> num_actual_tasks;
};

} /* namespace pheet */

#endif /* MSPPERFORMANCECOUNTERS_H_ */
