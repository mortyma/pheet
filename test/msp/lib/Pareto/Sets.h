/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __SETS_H
#define __SETS_H

#include <unordered_map>

#include "Set.h"
#include "test/msp/lib/Graph/Graph.h"

namespace pareto
{

/**
 * Maps one pareto set to each node of a graph g
 */
class Sets
{
public:

	/** The pareto set attached to
	 * the source node src is initialized with the null-vector (this is the
	 * shortest path from src to src)
	 */
	Sets(graph::Graph const* g, graph::Node const* src);
	virtual ~Sets();

	/**
	 * For p <- paths, insert e into the pareto set attached to p->head()
	 *
	 * After execution, the added vector contains all p <- paths which have been
	 * added (= which were not dominated).
	 * Likewise, the removed vector contains all p <- paths which were removed from
	 * the current Sets.
	 *
	 * Note that in the current implementation, the intersection
	 * of removed and added paths may be non-empty, i.e., added may contain
	 * dominated paths (to see this, assume that we
	 * have an empty set and two paths p1 and p2, where p1 is dominated by p2,
	 * are inserted (in that order). First, p1 is inserted into added, since it
	 * is not dominated by any path in the set. Next, when p2 is inserted, p1
	 * is inserted into removed since it is dominated by p2; p2 is then inserted
	 * into added).
	 */
	void insert(sp::Paths& paths,
	            sp::Paths& added,
	            sp::Paths& removed);

	sp::ShortestPaths* shortest_paths() const;

private:
	std::unordered_map<graph::Node const*, Set*> map;

};

}

#endif /*  __SETS_H*/
