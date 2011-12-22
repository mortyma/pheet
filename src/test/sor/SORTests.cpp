/*
* SORTests.cpp
*
*  Created on: 5.12.2011
*      Author: Daniel Cederman
*     License: Pheet license
*/

#include "SORTests.h"
#include "SORTest.h"
#include "PartitionMatrix/SORRun.h"

#include "../test_schedulers.h"
#include <iostream>

namespace pheet {

	template <class Test>
	void SORTests::test()
	{
#ifdef SOR_TEST
		for(size_t itr = 0; itr < sizeof(sor_test_iterations)/sizeof(sor_test_iterations[0]); itr++) {
			for(size_t sl = 0; sl < sizeof(sor_test_slices)/sizeof(sor_test_slices[0]); sl++) {
				for(size_t o = 0; o < sizeof(sor_test_omega)/sizeof(sor_test_omega[0]); o++) {
					for(size_t ms = 0; ms < sizeof(sor_test_maxtrissize)/sizeof(sor_test_maxtrissize[0]); ms++) {
						for(size_t c = 0; c < sizeof(sor_test_cpus)/sizeof(sor_test_cpus[0]); c++) {
							SORTest<Test> iart(sor_test_cpus[c],sor_test_maxtrissize[ms],sor_test_maxtrissize[ms],sor_test_slices[sl],sor_test_omega[o],sor_test_iterations[itr]);
							iart.run_test();
						}
					}
				}
			}
		}
#endif
	}

	void SORTests::run_test()
	{
#ifdef SOR_TEST
		if(sor_test) {
			test<SORRun<PrimitiveHeapPriorityScheduler> >();
		}
#endif
	}

}
