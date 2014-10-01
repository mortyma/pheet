/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef STRATEGY2LINCOMBSTRATEGY_H_
#define STRATEGY2LINCOMBSTRATEGY_H_

#include "lib/ShortestPath/Path.h"
#include "Less.h"

namespace pheet
{

template <class Pheet, template <class, class> class TaskStorageT>
class Strategy2LinCombStrategy
	: public Pheet::Environment::BaseStrategy
{
public:
	typedef Strategy2LinCombStrategy<Pheet, TaskStorageT> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;
	typedef TaskStorageT<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	Strategy2LinCombStrategy();
	Strategy2LinCombStrategy(sp::PathPtr path);
	Strategy2LinCombStrategy(Self& other);
	Strategy2LinCombStrategy(Self&& other);

	/**
	 * Returns true if this has higher or equal priority than other (i.e., return
	 * false iff other has strictly lower priority than this)
	 */
	inline bool prioritize(Self& other) const;

	inline bool dead_task();
	inline bool can_call(TaskStoragePlace*);

	Self& operator=(Self && other);

	//The following method and static member are required by the KLSMLocalityTaskStorage
	inline size_t get_k();
	static size_t default_k;

private:
	sp::PathPtr path;
	less<NR_DIMENSIONS> dominates;
};

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
Strategy2LinCombStrategy()
{
}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
Strategy2LinCombStrategy(sp::PathPtr path)
	: path(path)
{}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
Strategy2LinCombStrategy(Self& other)
	: BaseStrategy(other),
	  path(other.path)
{}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
Strategy2LinCombStrategy(Self&& other)
	: BaseStrategy(other),
	  path(other.path)
{}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
prioritize(Self& other) const
{
	/* The linear comibination of weight vector values is only possible
	 * because we are using non-negative ints for each dimension, since
	 * this->weight_sum > other->weight_sum => this != dominates other
	 * This would not be possible for a more general weight vector with different
	 * domains and priority function for each dimension.
	 */
	return this->path->weight_sum() <= other.path->weight_sum();
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
dead_task()
{
	return path->dominated();
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
can_call(TaskStoragePlace*)
{
	//we don't call conversion, as this might lead to stack overflows
	return false;
}

template <class Pheet, template <class, class> class TaskStorageT>
typename Strategy2LinCombStrategy<Pheet, TaskStorageT>::Self&
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
operator=(Strategy2LinCombStrategy<Pheet, TaskStorageT>::Self && other)
{
	path = other.path;
	return *this;
}

template <class Pheet, template <class, class> class TaskStorageT>
inline size_t
Strategy2LinCombStrategy<Pheet, TaskStorageT>::
get_k()
{
	return default_k;
}

//TODOMK: is this a good value?
template <class Pheet, template <class, class> class TaskStorageT>
size_t Strategy2LinCombStrategy<Pheet, TaskStorageT>::default_k = 1024;

} /* namespace pheet */

#endif /* STRATEGY2LINCOMBSTRATEGY_H_ */

