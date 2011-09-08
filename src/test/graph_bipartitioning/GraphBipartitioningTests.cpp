/*
 * GraphBipartitioningTests.cpp
 *
 *  Created on: 07.09.2011
 *   Author(s): Martin Wimmer
 *     License: Ask author
 */

#include "GraphBipartitioningTests.h"
#include "BranchBound/BranchBoundGraphBipartitioning.h"

#include "../test_schedulers.h"

namespace pheet {

GraphBipartitioningTests::GraphBipartitioningTests() {

}

GraphBipartitioningTests::~GraphBipartitioningTests() {

}

void SortingTests::run_test() {
	std::cout << "----" << std::endl;

	this->run_partitioner<BranchBoundGraphBipartitioning<DefaultMixedModeScheduler> >();
	this->run_partitioner<BranchBoundGraphBipartitioning<DefaultBasicScheduler> >();
	this->run_partitioner<BranchBoundGraphBipartitioning<DefaultSynchroneousScheduler> >();
}

}
