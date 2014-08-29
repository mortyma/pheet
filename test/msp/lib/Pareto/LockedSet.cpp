/*
 * Copyright Jakob Gruber, Martin Kalany 2013.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "LockedSet.h"

#include "KDSet.h"
#include "NaiveSet.h"

namespace pareto
{

template<class T>
LockedSet<T>::
LockedSet()
{
	m_set = new T();
}

template<class T>
LockedSet<T>::
LockedSet(sp::PathPtr& init)
{
	m_set = new T(init);
}


template<class T>
void
LockedSet<T>::
insert(sp::PathPtr& path,
       sp::Paths& added)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_set->insert(path, added);

}

template<class T>
sp::Paths
LockedSet<T>::
paths() const
{
	return m_set->paths();
}

template<class T>
LockedSet<T>::
~LockedSet()
{
	delete m_set;
}

template class LockedSet<KDSet>;
template class LockedSet<NaiveSet>;

}
