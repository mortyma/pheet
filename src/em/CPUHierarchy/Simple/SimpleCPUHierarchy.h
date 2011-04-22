/*
 * SimpleCPUHierarchy.h
 *
 *  Created on: 13.04.2011
 *      Author: Martin Wimmer
 *     License: Pheet license
 */

#ifndef SIMPLECPUHIERARCHY_H_
#define SIMPLECPUHIERARCHY_H_

#include "../../../../settings.h"
#include "../../../misc/types.h"

#include "SimpleCPUHierarchyCPUDescriptor.h"

#include <vector>
#include <assert.h>

namespace pheet {


class SimpleCPUHierarchy {
public:
	typedef SimpleCPUHierarchyCPUDescriptor CPUDescriptor;

	SimpleCPUHierarchy(procs_t np);
	SimpleCPUHierarchy(procs_t* levels, procs_t num_levels);
	~SimpleCPUHierarchy();

	procs_t get_size();
	std::vector<SimpleCPUHierarchy*> const* get_subsets();
	std::vector<CPUDescriptor*> const* get_cpus();

private:
	SimpleCPUHierarchy(SimpleCPUHierarchy* parent, procs_t offset);

	procs_t level;
	procs_t num_levels;
	procs_t offset;
	procs_t* levels;
	std::vector<SimpleCPUHierarchy*> subsets;
	std::vector<CPUDescriptor*> cpus;
};

}

#endif /* SIMPLECPUHIERARCHY_H_ */