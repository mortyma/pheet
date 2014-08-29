/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "Less.h"
#include "NaiveSet.h"

using namespace sp;

namespace pareto
{

NaiveSet::
NaiveSet()
{
}

NaiveSet::
NaiveSet(PathPtr& init)
{
	m_set.insert(init);
}

void
NaiveSet::
insert(sp::PathPtr& path,
       sp::Paths& added)
{
	less dominates;

	for (auto i = m_set.begin(); i != m_set.end();) {
		PathPtr const& rhs = *i;
		if (dominates(path.get(), rhs.get())) {
			rhs.get()->set_dominated();
			m_set.erase(i++);
		} else if (dominates(rhs.get(), path.get())) {
			return;
		} else {
			i++;
		}
	}

	added.push_back(path);
	m_set.insert(path);

}

sp::Paths
NaiveSet::
paths() const
{
	return sp::Paths(m_set.begin(), m_set.end());
}

}
