/*
 * StrategySchedulerTaskStorageItem.h
 *
 *  Created on: Mar 29, 2012
 *      Author: mwimmer
 *	   License: Ask Author
 */

#ifndef STRATEGYSCHEDULERTASKSTORAGEITEM_H_
#define STRATEGYSCHEDULERTASKSTORAGEITEM_H_

namespace pheet {

template <class Pheet, typename Task, typename StackElement>
struct StrategySchedulerTaskStorageItem {
	StrategySchedulerTaskStorageItem();

	Task* task;
	StackElement* stack_element;

	bool operator==(PrioritySchedulerTaskStorageItem<Pheet> const& other) const;
	bool operator!=(PrioritySchedulerTaskStorageItem<Pheet> const& other) const;
};


template <class Pheet, typename Task, typename StackElement>
StrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::StrategySchedulerTaskStorageItem()
: task(NULL), stack_element(NULL) {

}

template <class Pheet, typename Task, typename StackElement>
bool StrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::operator==(PrioritySchedulerTaskStorageItem<Pheet> const& other) const {
	return other.task == task;
}

template <class Pheet, typename Task, typename StackElement>
bool StrategySchedulerTaskStorageItem<Pheet, Task, StackElement>::operator!=(PrioritySchedulerTaskStorageItem<Pheet> const& other) const {
	return other.task != task;
}


template <class Pheet, typename Task, typename StackElement>
class nullable_traits<StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> > {
public:
	static StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> const null_value;
};

template <class Pheet, typename Task, typename StackElement>
StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> const nullable_traits<StrategySchedulerTaskStorageItem<Pheet, Task, StackElement> >::null_value;


}

#endif /* STRATEGYSCHEDULERTASKSTORAGEITEM_H_ */
