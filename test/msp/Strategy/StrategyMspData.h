/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef STRATEGYMSPDATA_H_
#define STRATEGYMSPDATA_H_

#include "lib/ShortestPath/ShortestPaths.h"

namespace pheet
{

class StrategyMspData
{
public:
	void clear()
	{
		candidates.clear();
		added.clear();
	}

	sp::Paths candidates, added;
};


} /* namespace pheet */
#endif /* STRATEGYMSPDATA_H_ */
