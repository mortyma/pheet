/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef STRATEGYMSPTASK_H_
#define STRATEGYMSPTASK_H_

#include "StrategyMspData.h"
#include "StrategyMspStrategy.h"
#include "Strategy2MspStrategy.h"
#include "Strategy2LinCombStrategy.h"
#include "lib/Graph/Edge.h"
#include "lib/Graph/Graph.h"
#include "lib/Pareto/Sets.h"

#include <pheet/misc/type_traits.h>
#include <pheet/ds/StrategyTaskStorage/KLSMLocality/KLSMLocalityTaskStorage.h>
#include <pheet/ds/StrategyTaskStorage/LSMLocality/LSMLocalityTaskStorage.h>
#include "pheet/ds/StrategyTaskStorage/ParetoLocality/ParetoLocalityTaskStorage.h"

namespace pheet
{

/**
 * Called by the scheduler for each selected path taken from the Pareto queue.
 *
 * The rough steps performed by a task are:
 * 1. Generate candidates.
 * 2. For each candidate c, insert it into the global Pareto set.
 *    If c is dominated, return.
 *    Otherwise, spawn a new task for c and mark all tasks as dead
 *    which have been pruned from the Pareto set by the insertion.
 *
 * Since there is no strictly defined sequence, this path is not necessarily
 * a Pareto optimum <-> this is a label correcting algorithm.
 *
 * We deviate from the Sanders algorithm in that parallelism
 * is only achieved by path; once a path has been selected and its corresponding
 * task has been spawned, that task executes sequentially (but in parallel
 * with other similar tasks) until finally spawning new potential tasks before
 * finishing.
 *
 * Furthermore, we lose some of the parallelism achieved by Sanders by
 * grouping candidates by node and doing an initial prune within each node set
 * in parallel before attempting to update the Pareto set. This causes a larger
 * amount of accesses for the global Pareto set. We are also unable to perform
 * bulk updates, since individual tasks do not communicate with each other.
 */

template <class Pheet>
class MspTask : public Pheet::Task
{
public:
	typedef StrategyMspStrategy<Pheet> Strategy;
	typedef MspPerformanceCounters<Pheet> PerformanceCounters;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	MspTask(const graph::Graph* graph,
	        const sp::PathPtr path,
	        pareto::Sets* sets,
	        PerformanceCounters& pc);

	void operator()();

	virtual void spawn(sp::PathPtr& p) = 0;

	static void set_k(size_t k);
	static void print_name();

protected:
	const graph::Graph* graph;
	const sp::PathPtr path;
	pareto::Sets* sets;
	PerformanceCounters& pc;
};


template <class Pheet>
void
MspTask<Pheet>::
print_name()
{
	std::cout << "Msp";
}

template <class Pheet>
MspTask<Pheet>::
MspTask(const graph::Graph* graph,
        const sp::PathPtr path,
        pareto::Sets* sets,
        PerformanceCounters& pc)
	: graph(graph), path(path), sets(sets), pc(pc)
{
}

template <class Pheet>
void
MspTask<Pheet>::
operator()()
{
	if (path->dominated()) {
		return;
	}

	pc.num_actual_tasks.incr();

	StrategyMspData& d = Pheet::template place_singleton<StrategyMspData>();
	d.clear();

	/* Generate candidates. */
	const graph::Node* head = path->head();

	d.candidates.reserve(head->out_edges().size());
	for (auto& e : head->out_edges()) {
		sp::PathPtr to(path->step(e));
		d.candidates.push_back(to);
	}

	/* Insert into the Pareto set. Mark dominated paths and spawn tasks for
	 * newly added paths. */

	sets->insert(d.candidates, d.added, d.removed);

	for (sp::PathPtr& p : d.removed) {
		p->set_dominated();
		pc.num_dead_tasks.incr();
	}

	for (sp::PathPtr& p : d.added) {
		spawn(p);
	}
}

template <class Pheet>
void
MspTask<Pheet>::
set_k(size_t k)
{
	Strategy::default_k = k;
}

/**
 * Using StrategyMspStrategy
 */
template <class Pheet>
class StrategyMspTask : public MspTask<Pheet>
{
public:
	StrategyMspTask(const graph::Graph* graph,
	                const sp::PathPtr path,
	                pareto::Sets* sets,
	                MspPerformanceCounters<Pheet>& pc)
		: MspTask<Pheet>(graph, path, sets, pc)
	{}

	static void print_name()
	{
		std::cout << "StrategyMsp";
	}

	virtual void spawn(sp::PathPtr& p)
	{
		// this is not very nice, but really the best we can do since we need
		// the type of this class in the spawn.
		typedef StrategyMspStrategy<Pheet> Strategy;
		typedef StrategyMspTask<Pheet> Self;
		Pheet::template spawn_s<Self>(Strategy(p), MspTask<Pheet>::graph, p,
		                              MspTask<Pheet>::sets, MspTask<Pheet>::pc);
	}

};


/**
 * Using Strategy2MspLinCombStrategy with different task storages.
 */
template <class Pheet, template <class, class> class TaskStorageT>
class Strategy2MspTaskLinComb : public MspTask<Pheet>
{
public:
	typedef Strategy2MspTaskLinComb<Pheet, TaskStorageT> Self;
	typedef Strategy2LinCombStrategy<Pheet, TaskStorageT> Strategy;
	typedef typename Strategy::TaskStorage TaskStorage;


	Strategy2MspTaskLinComb(const graph::Graph* graph,
	                        const sp::PathPtr path,
	                        pareto::Sets* sets,
	                        MspPerformanceCounters<Pheet>& pc)
		: MspTask<Pheet>(graph, path, sets, pc)
	{}

	static void print_name()
	{
		std::cout << "Strategy2Msp" << "<";
		TaskStorage::print_name();
		std::cout << ">";
	}

	virtual void spawn(sp::PathPtr& p)
	{
		Pheet::template spawn_s<Self>(Strategy(p), MspTask<Pheet>::graph, p,
		                              MspTask<Pheet>::sets, MspTask<Pheet>::pc);
	}
};


template <class Pheet>
using Strategy2MspLSMTask = Strategy2MspTaskLinComb<Pheet, LSMLocalityTaskStorage>;

template <class Pheet>
using Strategy2MspKLSMTask = Strategy2MspTaskLinComb<Pheet, KLSMLocalityTaskStorage>;

/**
 * Using Strategy2MspParetoStrategy with pareto task storage.
 */
template <class Pheet>
class Strategy2MspParetoTask : public MspTask<Pheet>
{
public:
	typedef Strategy2MspParetoTask<Pheet> Self;
	typedef Strategy2MspStrategy<Pheet, ParetoLocalityTaskStorage> Strategy;
	typedef typename Strategy::TaskStorage TaskStorage;


	Strategy2MspParetoTask(const graph::Graph* graph,
	                       const sp::PathPtr path,
	                       pareto::Sets* sets,
	                       MspPerformanceCounters<Pheet>& pc)
		: MspTask<Pheet>(graph, path, sets, pc)
	{}

	static void print_name()
	{
		std::cout << "Strategy2Msp" << "<";
		TaskStorage::print_name();
		std::cout << ">";
	}

	virtual void spawn(sp::PathPtr& p)
	{
		Pheet::template spawn_s<Self>(Strategy(p), MspTask<Pheet>::graph, p,
		                              MspTask<Pheet>::sets, MspTask<Pheet>::pc);
	}
};

} /* namespace pheet */

#endif /* STRATEGYMSPTASK_H_ */
