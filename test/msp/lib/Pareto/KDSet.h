/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef KDSET_H_
#define KDSET_H_

#include "KDTree.h"
#include "Set.h"

namespace pareto
{

class KDSet : public Set
{
public:

	KDSet();

	KDSet(sp::PathPtr& init);

	void insert(sp::PathPtr& path,
	            sp::Paths& added,
	            sp::Paths& removed) override;

	sp::Paths paths() const override;

private:
	KDTree t;
};

}

#endif /* KDSET_H_ */