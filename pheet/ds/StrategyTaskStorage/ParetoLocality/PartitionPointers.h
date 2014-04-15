/*
 * Copyright Jakob Gruber, Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#ifndef PARTITION_POINTERS_H_
#define PARTITION_POINTERS_H_

#include <vector>
#include "PivotQueue.h"
#include "VirtualArray.h"

namespace pheet
{

template<class Item>
class PartitionPointers
{
public:
	typedef typename VirtualArray<Item*>::VirtualArrayIterator VAIt;

	PartitionPointers(PivotQueue* pivot_queue, VAIt start, VAIt dead, VAIt end)
		: m_pivot_queue(pivot_queue), m_first(start), m_last(start), m_dead(dead), m_end(end)
	{
		m_idx.push_back(std::make_pair(start, nullptr));
	}

	~PartitionPointers()
	{
		while (m_idx.size() > 1) {
			m_pivot_queue->release(m_idx.back().second);
			m_idx.pop_back();
		}
	}

	/**
	 * Fall back using the previous partition as last partition, if there is one.
	 *
	 * The first partition pointer always points to the start of the first
	 * partition, i.e., index 0. Thus, if we have 1 exactly one partition pointer
	 * left, we cannot fall back.
	 */
	bool fall_back()
	{
		pheet_assert(m_pivot_queue->size() >= m_idx.size() - 1);
		if (m_idx.size() == 1) {
			return false;
		}
		m_dead = m_last;
		//reduce reference count on pivot element used for that partition step
		//Note: first partition pointer is always index 0 and is not associated
		//with a pivot element
		m_pivot_queue->release(m_idx.back().second);
		//remove the partition pointer
		m_idx.pop_back();
		pheet_assert(m_idx.size() > 0);
		m_last = m_idx.back().first;
		return true;
	}

	size_t size() const
	{
		return m_idx.size();
	}

	VAIt first() const
	{
		return m_first;
	}

	VAIt last() const
	{
		return m_last;
	}

	VAIt dead_partition() const
	{
		return m_dead;
	}

	void dead_partition(VAIt val)
	{
		m_dead = val;
	}

	void decrease_dead()
	{
		--m_dead;
	}

	VAIt end() const
	{
		return m_end;
	}

	void increment_end()
	{
		m_end++;
	}

	//TODO: rename idx to something like idx_in_data
	void add(VAIt idx, PivotElement* pivot)
	{
		m_idx.push_back(std::make_pair(idx, pivot));
		assert(m_pivot_queue->refcnt(m_idx.size() - 2) > 0);
		m_last = idx;
	}

	std::pair<VAIt, PivotElement*> get(size_t idx)
	{
		assert(idx < m_idx.size());
		return m_idx[idx];
	}

private:
	PivotQueue* m_pivot_queue;
	std::vector<std::pair<VAIt, PivotElement*>> m_idx; //TODO: rename
	/* start of first partition */
	VAIt const m_first;
	/* start of last partition (excluding dead items) */
	VAIt m_last;
	/* Dead items are stored right of the right-most partition, i.e., at the
	 * very end */
	VAIt m_dead;
	/* end of block. Points to (last+1) item, i.e., last item is at end-1 */
	VAIt m_end;

};

} /* namespace pheet */

#endif /* PARTITION_POINTERS_H_ */
