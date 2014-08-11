/*
 * Copyright Jakob Gruber 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef STRATEGY2MSPSTRATEGY_H_
#define STRATEGY2MSPSTRATEGY_H_

#include "lib/ShortestPath/Path.h"
#include "pheet/ds/StrategyTaskStorage/ParetoLocality/PriorityVector.h"

#define NR_DIMENSIONS (3)

namespace pheet
{

template <class Pheet, template <class, class> class TaskStorageT>
class Strategy2MspStrategy
	: public Pheet::Environment::BaseStrategy
{
public:
	typedef Strategy2MspStrategy<Pheet, TaskStorageT> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;
	typedef TaskStorageT<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	Strategy2MspStrategy();
	Strategy2MspStrategy(sp::PathPtr path);
	Strategy2MspStrategy(Self& other);
	Strategy2MspStrategy(Self&& other);

	/**
	 * Returns true if other has higher priority than this and false otherwise.
	 */
	inline bool prioritize(Self& other) const;

	inline bool dead_task();
	inline bool can_call(TaskStoragePlace*);

	Self& operator=(Self && other);

	//The following methods are required by the ParetoLocalityTaskStorage
	/**
	 * Returns true if at dimension dim this has less priority than
	 * other_val
	 */
	inline bool less_priority(size_t dim, size_t other_val) const;

	/**
	 * Returns true if at dimension dim this has greater priority than
	 * other_val
	 */
	inline bool greater_priority(size_t dim, size_t other_val) const;

	/**
	 * Returns true if at dimension dim this has equal priority as
	 * other_val
	 */
	inline bool equal_priority(size_t dim, size_t other_val) const;

	inline size_t priority_at(size_t dimension) const;
	inline size_t nr_dimensions() const;

	//The following method and static member are required by the KLSMLocalityTaskStorage
	inline size_t get_k();
	static size_t default_k;

private:
	sp::PathPtr path;
	PriorityVector<NR_DIMENSIONS>* w;
};

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2MspStrategy<Pheet, TaskStorageT>::
Strategy2MspStrategy()
{
}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2MspStrategy<Pheet, TaskStorageT>::
Strategy2MspStrategy(sp::PathPtr path)
	: path(path)
{
	w = new PriorityVector<NR_DIMENSIONS>(path);
}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2MspStrategy<Pheet, TaskStorageT>::
Strategy2MspStrategy(Self& other)
	: BaseStrategy(other),
	  path(other.path)
{
	w = new PriorityVector<NR_DIMENSIONS>(path);
}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2MspStrategy<Pheet, TaskStorageT>::
Strategy2MspStrategy(Self&& other)
	: BaseStrategy(other),
	  path(other.path)
{
	w = new PriorityVector<NR_DIMENSIONS>(path);
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2MspStrategy<Pheet, TaskStorageT>::
prioritize(Self& other) const
{
	/* TODOMK The linear comibination of weight vector values is only possible
	 * because we are using non-negative ints for each dimension, since
	 * this->weight_sum > other->weight_sum => this != dominates other
	 * This would not be possible for a more general weight vector with different
	 * domains and priority function for each dimension.
	 */
	//return true if other has (strictly) higher priority than this
	return this->path->weight_sum() <= other.path->weight_sum() ;
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2MspStrategy<Pheet, TaskStorageT>::
less_priority(size_t dim, size_t other_val)
const //TODOMK: rename to something like dominates, dominated by...
{
	assert(dim < w->dimensions());
	//TODOMK: < should be specified somewhere
	return other_val < w->at(dim);
}
template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2MspStrategy<Pheet, TaskStorageT>::
greater_priority(size_t dim, size_t other_val) const
{
	assert(dim < w->dimensions());
	return other_val > w->at(dim);
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2MspStrategy<Pheet, TaskStorageT>::
equal_priority(size_t dim, size_t other_val) const
{
	return !less_priority(dim, other_val) && !greater_priority(dim, other_val);
}

template <class Pheet, template <class, class> class TaskStorageT>
inline size_t
Strategy2MspStrategy<Pheet, TaskStorageT>::
priority_at(size_t dimension) const
{
	assert(dimension < w->dimensions());
	return w->at(dimension);
}

template <class Pheet, template <class, class> class TaskStorageT>
inline size_t
Strategy2MspStrategy<Pheet, TaskStorageT>::nr_dimensions() const
{
	return NR_DIMENSIONS;
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2MspStrategy<Pheet, TaskStorageT>::
dead_task()
{
	return path->dominated();
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2MspStrategy<Pheet, TaskStorageT>::
can_call(TaskStoragePlace*)
{
	return false;
}

template <class Pheet, template <class, class> class TaskStorageT>
typename Strategy2MspStrategy<Pheet, TaskStorageT>::Self&
Strategy2MspStrategy<Pheet, TaskStorageT>::
operator=(Strategy2MspStrategy<Pheet, TaskStorageT>::Self && other)
{
	path = other.path;
	w = other.w;
	return *this;
}

template <class Pheet, template <class, class> class TaskStorageT>
inline size_t
Strategy2MspStrategy<Pheet, TaskStorageT>::
get_k()
{
	return default_k;
}

//TODOMK: is this a good value?
template <class Pheet, template <class, class> class TaskStorageT>
size_t Strategy2MspStrategy<Pheet, TaskStorageT>::default_k = 1024;

} /* namespace pheet */

#endif /* STRATEGY2MSPSTRATEGY_H_ */

