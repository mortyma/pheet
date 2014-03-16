/*
 * StrategyBBGraphBipartitioningStrategy.h
 *
 *  Created on: Mar 14, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPBBGRAPHBIPARTITIONINGSTRATEGY2_H_
#define PPOPPBBGRAPHBIPARTITIONINGSTRATEGY2_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/UserDefinedPriority/UserDefinedPriority.h>

#include <pheet/ds/StrategyTaskStorage/LSMLocality/LSMLocalityTaskStorage.h>

namespace pheet {

template <class Pheet, class SubProblem>
  class PPoPPBBGraphBipartitioningStrategy2 : public Pheet::Environment::BaseStrategy {
public:
    typedef PPoPPBBGraphBipartitioningStrategy2<Pheet,SubProblem> Self;
    typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

    typedef LSMLocalityTaskStorage<Pheet, Self> TaskStorage;
    typedef typename TaskStorage::Place TaskStoragePlace;

    typedef typename Pheet::Place Place;

    PPoPPBBGraphBipartitioningStrategy2()
    :place(nullptr) {}

    PPoPPBBGraphBipartitioningStrategy2(SubProblem* sub_problem)
    :place(Pheet::get_place()),
     sub_problem(sub_problem),
     estimate(sub_problem->get_estimate()),
     uncertainty(sub_problem->get_estimate() - sub_problem->get_lower_bound()),
     treat_local(estimate < sub_problem->get_global_upper_bound())
    {}

    PPoPPBBGraphBipartitioningStrategy2(Self& other)
    : BaseStrategy(other),
      place(other.place),
      sub_problem(other.sub_problem),
      estimate(other.estimate),
      uncertainty(other.uncertainty),
      treat_local(other.treat_local) {}

    PPoPPBBGraphBipartitioningStrategy2(Self&& other)
    : BaseStrategy(other),
      place(other.place),
      sub_problem(other.sub_problem),
      estimate(other.estimate),
      uncertainty(other.uncertainty),
      treat_local(other.treat_local) {}


	~PPoPPBBGraphBipartitioningStrategy2() {}

	Self& operator=(Self&& other) {
		place = other.place;
		sub_problem = other.sub_problem;
		estimate = other.estimate;
		uncertainty = other.uncertainty;
		treat_local = other.treat_local;
		return *this;
	}

	inline bool prioritize(Self& other) {
		Place* p = Pheet::get_place();
		if(this->treat_local || this->place == p) {
			return estimate < other.estimate;
		}
		else if(other.treat_local || other.place == p) {
			return false;
		}
		return uncertainty > other.uncertainty;
	}

	bool dead_task() {
		return sub_problem->get_lower_bound() == sub_problem->get_global_upper_bound();
	}

	/*
	 * Checks whether spawn can be converted to a function call
	 */
	inline bool can_call(TaskStoragePlace* p) {
		size_t ub = sub_problem->get_global_upper_bound();
		size_t lb = sub_problem->get_old_lower_bound();
		size_t est = sub_problem->get_old_estimate();
		// Only call if only little work and chances are it won't change anything
		return p->size() > 1 && lb * 0.3 + est * 0.7 >= ub;
	}

	static void print_name() {
		std::cout << "Strategy2";
	}
private:
	Place* place;
	SubProblem* sub_problem;
    size_t estimate;
    size_t uncertainty;
    bool treat_local;
};

}

#endif /* STRATEGYBBGRAPHBIPARTITIONINGSTRATEGY2_H_ */
