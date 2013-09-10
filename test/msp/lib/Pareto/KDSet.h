/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef KDSET_H_
#define KDSET_H_

#include "kdtree++/kdtree.hpp"
#include "Set.h"

namespace pareto
{

template <int Dims>
class KDSet : public Set
{
private:
	struct acc_t {
		typedef graph::weight_t result_type;
		graph::weight_t operator()(sp::PathPtr const& path, size_t k) const {
			return path->weight()[k];
		}
	};

public:
	typedef KDTree::KDTree<Dims, sp::PathPtr, acc_t> tree_t;

	KDSet();
	virtual ~KDSet();

	void insert(sp::PathPtr& path,
	            sp::Paths& added,
	            sp::Paths& removed) override;

	sp::Paths paths() const override;

private:
	tree_t* t;
};

}

#endif /* KDSET_H_ */
