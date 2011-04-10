/*
 * SynchroneousMixedModeScheduler.cpp
 *
 *  Created on: 10.04.2011
 *      Author: mwimmer
 */

#include "SynchroneousMixedModeScheduler.h"

SynchroneousMixedModeScheduler::SynchroneousMixedModeScheduler()
: tec(this){

}

SynchroneousMixedModeScheduler::~SynchroneousMixedModeScheduler() {

}


SynchroneousMixedModeSchedulerTaskExecutionContext::SynchroneousMixedModeSchedulerTaskExecutionContext(SynchroneousMixedModeScheduler *sched)
: sched(sched){

}

SynchroneousMixedModeSchedulerTaskExecutionContext::~SynchroneousMixedModeSchedulerTaskExecutionContext() {

}
