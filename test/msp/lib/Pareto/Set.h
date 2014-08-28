/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __SET_H
#define __SET_H

#include "lib/ShortestPath/Path.h"
#include "lib/ShortestPath/ShortestPaths.h"

namespace pareto
{

/**
 * Interface for Pareto set operations.
 */
class Set
{
public:
	/**
	 * Attempts to inserts path into the set.
	 *
	 * If path has been added successfully, added contains path on return. Otherwise
	 * added will not have changed.
	 * removed contains all paths found to be dominated by path. If path was added,
	 * these are all paths currently in the set dominated by path. Otherwise,
	 * removed is not changed (to see this, assume p1 and p2 are in the set,
	 * where p1 is dominated by path and p2 dominates path. Since the
	 * dominates-relation is transitive, this is a contradiction: p1 would have
	 * been removed as soon as p2 was added).
	 */
	virtual void insert(sp::PathPtr& path,
	                    sp::Paths& added,
	                    sp::Paths& removed) = 0;

	virtual sp::Paths paths() const = 0;

	virtual ~Set() { }
};

}

#endif /* __SET_H */
