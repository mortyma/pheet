/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "SequentialMsp.h"

#include <memory>

#include "pheet/pheet.h"
#include "pheet/sched/Synchroneous/SynchroneousScheduler.h"

using namespace graph;
using namespace sp;

namespace pheet
{

template <class Pheet>
SequentialMsp<Pheet>::
SequentialMsp(sp::PathPtr const path,
              pareto::Sets* sets,
              PerformanceCounters& pc)
	: path(path), sets(sets), pc(pc)
{
}

template <class Pheet>
void
SequentialMsp<Pheet>::
operator()()
{
	PathPtr init = path;
	m_queue.insert(init);

	sp::Paths candidates, added;

	while (!m_queue.empty()) {
		candidates.clear();
		added.clear();

		/* Retrieve our next optimal candidate path. */

		PathPtr p = m_queue.first();
		Node const* head = p->head();

		pc.num_actual_tasks.incr();

		/* For all outgoing edges <- head, generate candidates. */

		candidates.reserve(head->out_edges().size());
		for (auto& e : head->out_edges()) {
			sp::PathPtr q(p->step(e));
			candidates.push_back(q);
		}

		sets->insert(candidates, added);

		/* Add newly inserted candidates to our queue. */

		for (auto& p : added) {
			m_queue.insert(p);
		}
	}
}

template <class Pheet>
char const SequentialMsp<Pheet>::name[] = "Sequential Msp";

template <class Pheet>
void
SequentialMsp<Pheet>::
print_name()
{
	std::cout << name;
}

template class SequentialMsp < pheet::PheetEnv < pheet::SynchroneousScheduler,
                               pheet::SystemModel,
                               pheet::Primitives,
                               pheet::DataStructures,
                               pheet::ConcurrentDataStructures >>;

}
