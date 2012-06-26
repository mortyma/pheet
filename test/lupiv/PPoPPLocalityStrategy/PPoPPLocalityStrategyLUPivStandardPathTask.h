/*
 * LocalityStrategyLUPivStandardPathTask.h
 *
 *  Created on: Jan 4, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef PPOPPLOCALITYSTRATEGYLUPIVSTANDARDPATHTASK_H_
#define PPOPPLOCALITYSTRATEGYLUPIVSTANDARDPATHTASK_H_

#include "../helpers/LUPivPivotTask.h"
#include "../LocalityStrategy/LocalityStrategyLUPivMMTask.h"
#include "PPoPPLUPivLocalityStrategy.h"

#include <algorithm>

extern "C" {
void dtrsm_(char *side, char *uplo, char* transa, char* diag, int *m, int *n, double* alpha, double* a, int* lda, double* b, int* ldb);
}

namespace pheet {

template <class Pheet, int BLOCK_SIZE>
class PPoPPLocalityStrategyLUPivStandardPathTask : public Pheet::Task {
public:
	PPoPPLocalityStrategyLUPivStandardPathTask(typename Pheet::Place** owner_info, typename Pheet::Place** block_owners, double* a, double* lu_col, int* pivot, int m, int lda, int n);
	virtual ~PPoPPLocalityStrategyLUPivStandardPathTask();

	virtual void operator()();

private:
	typename Pheet::Place** owner_info;
	typename Pheet::Place** block_owners;
	double* a;
	double* lu_col;
	int* pivot;
	int m;
	int lda;
	int n;
};

template <class Pheet, int BLOCK_SIZE>
PPoPPLocalityStrategyLUPivStandardPathTask<Pheet, BLOCK_SIZE>::PPoPPLocalityStrategyLUPivStandardPathTask(typename Pheet::Place** owner_info, typename Pheet::Place** block_owners, double* a, double* lu_col, int* pivot, int m, int lda, int n)
: owner_info(owner_info), block_owners(block_owners), a(a), lu_col(lu_col), pivot(pivot), m(m), lda(lda), n(n) {

}

template <class Pheet, int BLOCK_SIZE>
PPoPPLocalityStrategyLUPivStandardPathTask<Pheet, BLOCK_SIZE>::~PPoPPLocalityStrategyLUPivStandardPathTask() {

}

template <class Pheet, int BLOCK_SIZE>
void PPoPPLocalityStrategyLUPivStandardPathTask<Pheet, BLOCK_SIZE>::operator()() {
	pheet_assert(n <= BLOCK_SIZE);

	// Apply pivot to column
	Pheet::template
		call<LUPivPivotTask<Pheet> >(a, pivot, std::min(m, BLOCK_SIZE), lda, n);

	int k = std::min(std::min(m, n), BLOCK_SIZE);

	{	// Apply TRSM to uppermost block
		char side = 'l';
		char uplo = 'l';
		char transa = 'n';
		char diag = 'u';
		double alpha = 1.0;
		dtrsm_(&side, &uplo, &transa, &diag, &k, &k, &alpha, lu_col, &lda, a, &lda);
	}

	// Apply matrix multiplication to all other blocks
	int num_blocks = m / BLOCK_SIZE;

	for(int i = 1; i < num_blocks; ++i) {
		int pos = i * BLOCK_SIZE;
		Pheet::template
			spawn_s<LocalityStrategyLUPivMMTask<Pheet> >(
					PPoPPLUPivLocalityStrategy<Pheet>(block_owners[i], 4, 2),
					block_owners + i,
					lu_col + pos, a, a + pos, k, lda);
	}
}

}

#endif /* PPOPPLOCALITYSTRATEGYLUPIVSTANDARDPATHTASK_H_ */
