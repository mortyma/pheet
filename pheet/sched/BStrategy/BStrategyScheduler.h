/*
 * BStrategyScheduler.h
 *
 *  Created on: Mar 7, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BSTRATEGYSCHEDULER_H_
#define BSTRATEGYSCHEDULER_H_

#include "BStrategySchedulerPlace.h"
#include "BStrategySchedulerPerformanceCounters.h"
#include "BStrategySchedulerTaskStorageItem.h"

//#include "../../ds/StrategyTaskStorage/Local/LocalStrategyTaskStorage.h"
#include "../../ds/StrategyTaskStorage/CentralK/CentralKStrategyTaskStorage.h"
#include "../../ds/StrategyStealer/Basic/BasicStrategyStealer.h"
#include "base_strategies/LifoFifo/LifoFifoBaseStrategy.h"

namespace pheet {

template <class Pheet>
struct BStrategySchedulerState {
	BStrategySchedulerState();

	uint8_t current_state;
	typename Pheet::Barrier state_barrier;
	typename Pheet::Scheduler::Task* startup_task;
};

template <class Pheet>
BStrategySchedulerState<Pheet>::BStrategySchedulerState()
: current_state(0), startup_task(NULL) {

}


template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
class BStrategySchedulerImpl {
public:
	typedef typename Pheet::Backoff Backoff;
	typedef typename Pheet::MachineModel MachineModel;
	typedef BinaryTreeMachineModel<Pheet, MachineModel> InternalMachineModel;
	typedef BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT> Self;
	typedef SchedulerTask<Pheet> Task;
	template <typename F>
	using FunctorTask = SchedulerFunctorTask<Pheet, F>;
	typedef BStrategySchedulerTaskStorageItem<Pheet, Task, BStrategySchedulerPlaceStackElement> TaskStorageItem;
	typedef TaskStorageT<Pheet, TaskStorageItem> TaskStorage;
	typedef StealerT<Pheet, TaskStorage> Stealer;
	typedef BStrategySchedulerPlace<Pheet, 4> Place;
	typedef BStrategySchedulerState<Pheet> State;
	typedef FinishRegion<Pheet> Finish;
//	typedef BStrategySchedulerPlaceDescriptor<Pheet> PlaceDesc;
//	template <class Strategy>
//	using TaskDesc = BStrategySchedulerTaskDescriptor<Pheet, Strategy>;
	typedef BaseStrategyT<Pheet> BaseStrategy;
	typedef typename Place::PerformanceCounters PerformanceCounters;

	template <class NP>
	using BT = BStrategySchedulerImpl<NP, TaskStorageT, BaseStrategyT>;
	template <template <class, typename> class NewTS>
		using WithTaskStorage = Self;
	template <template <class, typename, typename> class NewTS>
		using WithPriorityTaskStorage = Self;
	template <template <class, typename, template <class, class> class> class NewTS>
		using WithStrategyTaskStorage = BStrategySchedulerImpl<Pheet, NewTS, BaseStrategyT>;;

	/*
	 * Uses complete machine
	 */
	BStrategySchedulerImpl();
	BStrategySchedulerImpl(PerformanceCounters& performance_counters);

	/*
	 * Only uses the given number of places
	 * (Currently no direct support for oversubscription)
	 */
	BStrategySchedulerImpl(procs_t num_places);
	BStrategySchedulerImpl(procs_t num_places, PerformanceCounters& performance_counters);
	~BStrategySchedulerImpl();

	static void print_name();

//	static void print_performance_counter_values(Place::PerformanceCounters performance_counters);

//	static void print_performance_counter_headers();

	static Place* get_place();
	static procs_t get_place_id();
	Place* get_place_at(procs_t place_id);

	template<class CallTaskType, typename ... TaskParams>
		void finish(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void finish(F&& f, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void call(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void call(F&& f, TaskParams&& ... params);

	template<class CallTaskType, typename ... TaskParams>
		void spawn(TaskParams&& ... params);

	template<typename F, typename ... TaskParams>
		void spawn(F&& f, TaskParams&& ... params);

	template<class CallTaskType, class Strategy, typename ... TaskParams>
		void spawn_s(Strategy s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		void spawn_s(Strategy s, F&& f, TaskParams&& ... params);
/*
	template<class CallTaskType, class Strategy, typename ... TaskParams>
		void spawn_s(Strategy&& s, TaskParams&& ... params);

	template<class Strategy, typename F, typename ... TaskParams>
		void spawn_s(Strategy&& s, F&& f, TaskParams&& ... params);
*/
	static char const name[];
	static procs_t const max_cpus;

private:
	InternalMachineModel machine_model;

	Place** places;
	procs_t num_places;
	TaskStorage task_storage;

	State state;

	PerformanceCounters performance_counters;
};


template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
char const BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::name[] = "BStrategyScheduler";

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
procs_t const BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::max_cpus = std::numeric_limits<procs_t>::max() >> 1;

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::BStrategySchedulerImpl()
: num_places(machine_model.get_num_leaves()), task_storage(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, task_storage, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::BStrategySchedulerImpl(typename Place::PerformanceCounters& performance_counters)
: num_places(machine_model.get_num_leaves()), task_storage(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, task_storage, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::BStrategySchedulerImpl(procs_t num_places)
: num_places(num_places), task_storage(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, task_storage, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::BStrategySchedulerImpl(procs_t num_places, typename Place::PerformanceCounters& performance_counters)
: num_places(num_places), task_storage(num_places) {

	places = new Place*[num_places];
	places[0] = new Place(machine_model, task_storage, places, num_places, &state, performance_counters);
	places[0]->prepare_root();
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::~BStrategySchedulerImpl() {
	delete places[0];
	delete[] places;
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::print_name() {
	std::cout << name << "<";
	Place::TaskStorage::print_name();
	std::cout /*<< ", " << (int)CallThreshold*/ << ">";
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
typename BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::Place* BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::get_place() {
	return Place::get();
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
procs_t BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::get_place_id() {
	return Place::get()->get_id();
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
typename BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::Place* BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::get_place_at(procs_t place_id) {
	pheet_assert(place_id < num_places);
	return places[place_id];
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<class CallTaskType, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::finish(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template finish<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<typename F, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::finish(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->finish(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<class CallTaskType, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::spawn(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<typename F, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::spawn(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn(f, std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<class CallTaskType, class Strategy, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::spawn_s(Strategy s, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template spawn_s<CallTaskType>(std::move(s), std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<class Strategy, typename F, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::spawn_s(Strategy s, F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_s(std::move(s), f, std::forward<TaskParams&&>(params) ...);
}
/*
template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<class CallTaskType, class Strategy, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::spawn_s(Strategy&& s, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_s<CallTaskType>(std::forward<Strategy&&>(s), std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<class Strategy, typename F, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::spawn_s(Strategy&& s, F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->spawn_s(std::forward<Strategy&&>(s), f, std::forward<TaskParams&&>(params) ...);
}*/

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<class CallTaskType, typename ... TaskParams>
inline void BStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::call(TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->template call<CallTaskType>(std::forward<TaskParams&&>(params) ...);
}

template <class Pheet, template <class P, typename T, template <class, class> class> class TaskStorageT, template <class P> class BaseStrategyT>
template<typename F, typename ... TaskParams>
inline void BBStrategySchedulerImpl<Pheet, TaskStorageT, BaseStrategyT>::call(F&& f, TaskParams&& ... params) {
	Place* p = get_place();
	pheet_assert(p != NULL);
	p->call(f, std::forward<TaskParams&&>(params) ...);
}


template<class Pheet>
using BStrategyScheduler = BStrategySchedulerImpl<Pheet, CentralKStrategyTaskStorage, LifoFifoBaseStrategy>;

}

#endif /* BSTRATEGYSCHEDULER_H_ */
