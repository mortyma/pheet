/*
 * SsspTests.h
 *
 *  Created on: Jul 16, 2012
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef SSSPTESTS_H_
#define SSSPTESTS_H_

#include "../init.h"
#include "SsspTest.h"

namespace pheet {

class SsspTests {
public:
	SsspTests();
	~SsspTests();

	void run_test();

private:
	template<class Pheet, template <class P> class Partitioner>
	void run_algorithm();
};

template <class Pheet, template <class P> class Partitioner>
void SsspTests::run_algorithm() {
	typename Pheet::MachineModel mm;
	procs_t max_cpus = std::min(mm.get_num_leaves(), Pheet::Environment::max_cpus);

	for(size_t t = 0; t < sizeof(sssp_test_types)/sizeof(sssp_test_types[0]); t++) {
		for(size_t pr = 0; pr < sizeof(sssp_test_problems)/sizeof(sssp_test_problems[0]); pr++) {
			for(size_t k = 0; k < sizeof(sssp_test_k)/sizeof(sssp_test_k[0]); k++) {
				bool max_processed = false;
				procs_t cpus;
				for(size_t c = 0; c < sizeof(sssp_test_cpus)/sizeof(sssp_test_cpus[0]); c++) {
					cpus = sssp_test_cpus[c];
					if(cpus >= max_cpus) {
						if(!max_processed) {
							cpus = max_cpus;
							max_processed = true;
						}
						else {
							continue;
						}
					}
					for(size_t s = 0; s < sizeof(sssp_test_seeds)/sizeof(sssp_test_seeds[0]); s++) {
						SsspTest<Pheet, Partitioner> gbt(cpus, sssp_test_types[t], sssp_test_k[k],
								sssp_test_problems[pr].n,
								sssp_test_problems[pr].p,
								sssp_test_problems[pr].max_w,
								sssp_test_seeds[s]);
						gbt.run_test();
					}
				}
			}
		}
	}
}

} /* namespace pheet */
#endif /* SSSPTESTS_H_ */
