/*
 * LUPivLocalityStrategy.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPLUPIVLOCALITYSTRATEGY_H_
#define PPOPPLUPIVLOCALITYSTRATEGY_H_

#include <pheet/pheet.h>
#include <pheet/sched/strategies/BaseStrategy.h>

namespace pheet {

template <class Pheet>
  class PPoPPLUPivLocalityStrategy : public Pheet::Environment::BaseStrategy {
public:
	typedef PPoPPLUPivLocalityStrategy<Pheet> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;

	PPoPPLUPivLocalityStrategy(typename Pheet::Place* last_owner);
	PPoPPLUPivLocalityStrategy(Self& other);
	PPoPPLUPivLocalityStrategy(Self&& other);
	~PPoPPLUPivLocalityStrategy();

	inline bool prioritize(Self& other);


private:
	typename Pheet::Place* last_owner;
};

template <class Pheet>
PPoPPLUPivLocalityStrategy<Pheet>::PPoPPLUPivLocalityStrategy(typename Pheet::Place* last_owner)
  : last_owner(last_owner) {

}

template <class Pheet>
PPoPPLUPivLocalityStrategy<Pheet>::PPoPPLUPivLocalityStrategy(Self& other)
  : BaseStrategy(other),last_owner(other.last_owner) {

}

template <class Pheet>
PPoPPLUPivLocalityStrategy<Pheet>::PPoPPLUPivLocalityStrategy(Self&& other)
  : BaseStrategy(other),last_owner(other.last_owner) {

}

template <class Pheet>
PPoPPLUPivLocalityStrategy<Pheet>::~PPoPPLUPivLocalityStrategy() {

}


template <class Pheet>
  inline bool PPoPPLUPivLocalityStrategy<Pheet>::prioritize(Self& other) {
  
  /*  procs_t distancetothis = Pheet::get_place()->get_distance(last_owner);
  procs_t distancetoother = Pheet::get_place()->get_distance(other.last_owner);

  if(distancetothis != distancetoother)
  return distancetothis < distancetoother;*/
  return BaseStrategy::prioritize(other);

}

//template <class Pheet>
//inline prio_t PPoPPLUPivLocalityStrategy<Pheet>::get_pop_priority(size_t task_id) {
  /*	typename Pheet::TaskExecutionContext* owner = Pheet::get_context();
	procs_t max_d = owner->get_max_distance();
	procs_t d = owner->get_distance(last_owner);
	return ((max_d - d) << 4) + base_pop_priority;*/
//	return (base_pop_priority << 6) + task_id;
//}

//template <class Pheet>
//inline prio_t PPoPPLUPivLocalityStrategy<Pheet>::get_steal_priority(size_t task_id, typename Pheet::Scheduler::StealerDescriptor& desc) {
//	procs_t max_d = desc.get_max_distance();
//	procs_t d = desc.get_distance_to(last_owner);
//	return ((((max_d - d) << 4) + base_steal_priority) << 16) - (task_id % (1 << 16));
//}

/*template <class Pheet>
inline BaseStrategy<Pheet>* PPoPPLUPivLocalityStrategy<Pheet>::clone() {
	return new Self(*this);
	}*/

}

#endif /* PPOPPLUPIVLOCALITYSTRATEGY_H_ */