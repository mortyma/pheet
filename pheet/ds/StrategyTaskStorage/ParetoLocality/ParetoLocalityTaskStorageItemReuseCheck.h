/*
 * Copyright Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef PARETOLOCALITYTASKSTORAGEITEMREUSECHECK_H_
#define PARETOLOCALITYTASKSTORAGEITEMREUSECHECK_H_

namespace pheet
{

template <class Item>
struct ParetoLocalityTaskStorageItemReuseCheck {
	bool operator()(Item const&) const
	{
		return false; //TODOMK: this is a dummy implementation
	}
};

} /* namespace pheet */
#endif /* PARETOLOCALITYTASKSTORAGEITEMREUSECHECK_H_*/
