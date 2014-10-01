/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef Strategy2ParetoStrategy_H_
#define Strategy2ParetoStrategy_H_

#include "lib/ShortestPath/Path.h"
#include "Less.h"
#include "pheet/ds/StrategyTaskStorage/ParetoLocality/PriorityVector.h"

#define NR_DIMENSIONS (3)

namespace pheet
{

template <class Pheet, template <class, class> class TaskStorageT>
class Strategy2ParetoStrategy
	: public Pheet::Environment::BaseStrategy
{
public:
	typedef Strategy2ParetoStrategy<Pheet, TaskStorageT> Self;
	typedef typename Pheet::Environment::BaseStrategy BaseStrategy;
	typedef TaskStorageT<Pheet, Self> TaskStorage;
	typedef typename TaskStorage::Place TaskStoragePlace;
	typedef typename Pheet::Place Place;

	Strategy2ParetoStrategy();
	Strategy2ParetoStrategy(sp::PathPtr path);
	Strategy2ParetoStrategy(Self& other);
	Strategy2ParetoStrategy(Self&& other);

	/**
	 * Returns true if this has higher or equal priority than other (i.e., return
	 * false iff other has strictly lower priority than this)
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

private:
	sp::PathPtr path;
	less<NR_DIMENSIONS> dominates;
	PriorityVector<NR_DIMENSIONS>* w;
};

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
Strategy2ParetoStrategy()
{
}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
Strategy2ParetoStrategy(sp::PathPtr path)
	: path(path)
{
	w = new PriorityVector<NR_DIMENSIONS>(path);
}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
Strategy2ParetoStrategy(Self& other)
	: BaseStrategy(other),
	  path(other.path)
{
	w = new PriorityVector<NR_DIMENSIONS>(path);
}

template <class Pheet, template <class, class> class TaskStorageT>
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
Strategy2ParetoStrategy(Self&& other)
	: BaseStrategy(other),
	  path(other.path)
{
	w = new PriorityVector<NR_DIMENSIONS>(path);
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
prioritize(Self& other) const
{
	/* The linear comibination of weight vector values is only possible if
	 * we are using non-negative ints for each dimension, since
	 * this->weight_sum > other->weight_sum => this != dominates other
	 * This is not possible for a more general weight vector with different
	 * domains and priority function for each dimension.
	 */
	//return this->path->weight_sum() <= other.path->weight_sum();

	//return true if this has higher or equal priority than other (i.e., return
	//false iff other has strictly lower priority than this)
	return dominates(this->w, other.w);

}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
less_priority(size_t dim, size_t other_val)
const
{
	assert(dim < w->dimensions());
	return other_val < w->at(dim);
}
template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
greater_priority(size_t dim, size_t other_val) const
{
	assert(dim < w->dimensions());
	return other_val > w->at(dim);
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
equal_priority(size_t dim, size_t other_val) const
{
	return !less_priority(dim, other_val) && !greater_priority(dim, other_val);
}

template <class Pheet, template <class, class> class TaskStorageT>
inline size_t
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
priority_at(size_t dimension) const
{
	assert(dimension < w->dimensions());
	return w->at(dimension);
}

template <class Pheet, template <class, class> class TaskStorageT>
inline size_t
Strategy2ParetoStrategy<Pheet, TaskStorageT>::nr_dimensions() const
{
	return NR_DIMENSIONS;
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
dead_task()
{
	return path->dominated();
}

template <class Pheet, template <class, class> class TaskStorageT>
inline bool
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
can_call(TaskStoragePlace*)
{
	return false;
}

template <class Pheet, template <class, class> class TaskStorageT>
typename Strategy2ParetoStrategy<Pheet, TaskStorageT>::Self&
Strategy2ParetoStrategy<Pheet, TaskStorageT>::
operator=(Strategy2ParetoStrategy<Pheet, TaskStorageT>::Self && other)
{
	path = other.path;
	w = other.w;
	return *this;
}

} /* namespace pheet */

#endif /* Strategy2ParetoStrategy_H_ */

