/*
 * SynchroneousMixedModeScheduler.h
 *
 *  Created on: 06.04.2011
 *      Author: mwimmer
 */

#ifndef SYNCHRONEOUSMIXEDMODESCHEDULER_H_
#define SYNCHRONEOUSMIXEDMODESCHEDULER_H_

#include "SynchroneousMixedModeTask.h"
//#include "SynchroneousMixedModeSchedulerTaskExecutionContext.h"

class SynchroneousMixedModeScheduler;

class SynchroneousMixedModeSchedulerTaskExecutionContext {
public:
	SynchroneousMixedModeSchedulerTaskExecutionContext(SynchroneousMixedModeScheduler *sched);
	virtual ~SynchroneousMixedModeSchedulerTaskExecutionContext();

	template<class CallTaskType, typename ... TaskParams>
		void finish(TaskParams ... params);

	template<class CallTaskType, typename ... TaskParams>
		void call(TaskParams ... params);

	template<class CallTaskType, typename ... TaskParams>
		void spawn(TaskParams ... params);

	template<class CallTaskType, typename ... TaskParams>
		void local_finish(TaskParams ... params);

private:
	SynchroneousMixedModeScheduler *sched;
};

class SynchroneousMixedModeScheduler {
public:
	typedef SynchroneousMixedModeTask<SynchroneousMixedModeScheduler> Task;
	typedef SynchroneousMixedModeSchedulerTaskExecutionContext TaskExecutionContext;

	SynchroneousMixedModeScheduler();
	virtual ~SynchroneousMixedModeScheduler();

	template<class CallTaskType, typename ... TaskParams>
	void finish(TaskParams ... params);

private:
	TaskExecutionContext tec;
};

SynchroneousMixedModeScheduler::SynchroneousMixedModeScheduler()
: tec(this){

}

SynchroneousMixedModeScheduler::~SynchroneousMixedModeScheduler() {

}

template<class CallTaskType, typename ... TaskParams>
void SynchroneousMixedModeScheduler::finish(TaskParams ... params) {
	CallTaskType task(params ...);
	task.execute(tec);
}


template<class CallTaskType, typename ... TaskParams>
void SynchroneousMixedModeSchedulerTaskExecutionContext::finish(TaskParams ... params) {
	sched->finish<CallTaskType>(params ...);
}

template<class CallTaskType, typename ... TaskParams>
void SynchroneousMixedModeSchedulerTaskExecutionContext::call(TaskParams ... params) {
	sched->finish<CallTaskType>(params ...);
}

template<class CallTaskType, typename ... TaskParams>
void SynchroneousMixedModeSchedulerTaskExecutionContext::spawn(TaskParams ... params) {
	sched->finish<CallTaskType>(params ...);
}

template<class CallTaskType, typename ... TaskParams>
void SynchroneousMixedModeSchedulerTaskExecutionContext::local_finish(TaskParams ... params) {
	sched->finish<CallTaskType>(params ...);
}

#endif /* SYNCHRONEOUSMIXEDMODESCHEDULER_H_ */
