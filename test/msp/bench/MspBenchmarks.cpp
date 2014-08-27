/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "MspBenchmarks.h"

#include "MspBenchmark.h"
#include "pheet/sched/BStrategy/BStrategyScheduler.h"
#include "pheet/sched/Strategy2/StrategyScheduler2.h"
#include "pheet/sched/Synchroneous/SynchroneousScheduler.h"
#include "Sequential/SequentialMsp.h"
#include "Strategy/MspTask.h"

namespace
{

template <class Pheet, template <class P> class Partitioner>
void
run_algorithm(graph::Graph* g, graph::Node* src, int n, std::string comment)
{
	pheet::MspBenchmark<Pheet, Partitioner> gbt(n, g, src, comment);
	gbt.run_test();
}

}

namespace pheet
{

void
MspBenchmarks::
run_benchmarks(BenchOpts const& opts)
{
	for (auto& it : opts.files) {
		graph::Graph* g = graph::Graph::read(fopen(it.c_str(), "r"));
		graph::Node* src = g->nodes().front();

		/* Note: no need to execute with SynchroneousScheduler for different
		   amount of cores */
		if (opts.sequential) {
			for (int i = 0; i < opts.repetitions; i++) {
				::run_algorithm < Pheet::WithScheduler<SynchroneousScheduler>,
				SequentialMsp > (g, src, 1, opts.comment);
			}
		}

		if (opts.strategy) {
			for (auto& it : opts.ncpus) {
				for (int i = 0; i < opts.repetitions; i++) {
					::run_algorithm < Pheet::WithScheduler<BStrategyScheduler>
					::WithTaskStorage<DistKStrategyTaskStorage>,
					StrategyMspTask > (g, src, it, opts.comment);
				}
			}
		}

		if (opts.s2klsm) {
			for (auto& it : opts.ncpus) {
				for (int i = 0; i < opts.repetitions; i++) {
					::run_algorithm < Pheet::WithScheduler<StrategyScheduler2>,
					Strategy2MspKLSMTask > (g, src, it, opts.comment);

				}
			}
		}

		if (opts.s2lsm) {
			for (auto& it : opts.ncpus) {
				for (int i = 0; i < opts.repetitions; i++) {
					::run_algorithm < Pheet::WithScheduler<StrategyScheduler2>,
					Strategy2MspLSMTask > (g, src, it, opts.comment);

				}
			}
		}

		if (opts.s2pareto) {
			for (auto& it : opts.ncpus) {
				for (int i = 0; i < opts.repetitions; i++) {
					::run_algorithm < Pheet::WithScheduler<StrategyScheduler2>,
					Strategy2MspParetoTask > (g, src, it, opts.comment);

				}
			}
		}

		delete g;
	}
}

} /* namespace pheet */
