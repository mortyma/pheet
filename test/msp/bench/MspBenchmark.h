/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef MSPBENCHMARK_H_
#define MSPBENCHMARK_H_

#include "../Test.h"
#include "lib/Graph/Generator/Generator.h"
#include "lib/Pareto/Sets.h"
#include "pheet/pheet.h"

namespace pheet
{

template <class Pheet, template <class P> class Algorithm>
class MspBenchmark : Test
{
public:
	/**
	 * MspBenchmark executes a single run of the multi-criteria shortest path algorithm
	 * with the given parameters.
	 */
	MspBenchmark(const procs_t cpus,
	             const graph::Graph* g,
	             const graph::Node* src,
	             const std::string comment
	            );
	virtual ~MspBenchmark();

	void run_test();

private:
	procs_t cpus;
	graph::Graph const* g;
	graph::Node const* src;
	std::string comment;

	static char const* const types[];
};

template <class Pheet, template <class P> class Algorithm>
char const* const MspBenchmark<Pheet, Algorithm>::types[] = {"random"};

template <class Pheet, template <class P> class Algorithm>
MspBenchmark<Pheet, Algorithm>::
MspBenchmark(const procs_t cpus,
             const graph::Graph* g,
             const graph::Node* src,
             const std::string comment
            )
	: cpus(cpus), g(g), src(src), comment(comment)
{
}

template <class Pheet, template <class P> class Algorithm>
MspBenchmark<Pheet, Algorithm>::
~MspBenchmark()
{
}

template <class Pheet, template <class P> class Algorithm>
void
MspBenchmark<Pheet, Algorithm>::
run_test()
{
	typename Pheet::Environment::PerformanceCounters pc;
	typename Algorithm<Pheet>::PerformanceCounters ppc;

	Time start, end;

	std::shared_ptr<sp::ShortestPaths> sp;

	{
		typename Pheet::Environment env(cpus, pc);

		pareto::Sets q(g, src);
		sp::PathPtr init(new sp::Path(src));

		check_time(start);
		Pheet::template finish<Algorithm<Pheet>>(init, &q, ppc);
		check_time(end);
		sp.reset(q.paths());
	}

	const double seconds = calculate_seconds(start, end);

	int npaths = 0;
	for (auto const& pair : sp->paths) {
		npaths += pair.second.size();
	}

	/* Headers. */

	std::cout << "test\t"
	          << "algorithm\t"
	          << "scheduler\t"
	          << "nodes\t"
	          << "edges\t"
	          << "paths\t"
	          << "cpus\t"
	          << "total_time\t";
	ppc.print_headers();
	std::cout << "comment\t";
	std::cout << std::endl;

	/* Values. */

	std::cout << "msp" << "\t";
	Algorithm<Pheet>::print_name();
	std::cout << "\t";
	Pheet::Environment::print_name();
	std::cout << "\t"
	          << g->node_count() << "\t"
	          << g->edge_count() << "\t"
	          << npaths << "\t"
	          << cpus << "\t"
	          << seconds << "\t";
	ppc.print_values();
	std::cout << comment << "\t";
	std::cout << std::endl;
}

} /* namespace pheet */

#endif /* MSPBENCHMARK_H_ */
