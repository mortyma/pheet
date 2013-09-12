/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "LockedSet.h"
#include "Sets.h"

namespace pareto
{

Sets::
Sets(graph::Graph const* g)
{
	for (auto node : g->nodes()) {
		Set* set = new LockedSet();
		map.insert(std::pair<graph::Node const*, Set*>(node, set));
	}
}

Sets::
~Sets()
{
	for (auto it = map.begin(); it != map.end(); ++it) {
		delete it->second;
	}

}

void
Sets::
insert(sp::Paths& paths,
       sp::Paths& added,
       sp::Paths& removed)
{
	for (auto & p : paths) {
		graph::Node const* node = p->head();
		map[node]->insert(p, added, removed);
	}
}

sp::ShortestPaths*
Sets::
shortest_paths() const
{
	sp::ShortestPaths* sp = new sp::ShortestPaths;

	for (const auto & p : map) {
		sp->paths[p.first] = p.second->paths();
	}

	return sp;
}

}