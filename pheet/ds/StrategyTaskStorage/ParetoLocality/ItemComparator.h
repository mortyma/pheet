/*
 * Copyright Martin Kalany 2014.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
namespace pheet
{
template<class Item>
class ItemComparator
{
public:
	ItemComparator(size_t d) : m_d(d) {	}

	/**
	 * Returns true iff priority value of itemA at dimension d < priority value
	 * of itemB at dimension d
	 */
	bool operator()(Item* itemA, Item* itemB)
	{
		return itemA->strategy()->priority_at(m_d) < itemB->strategy()->priority_at(m_d);
	}

private:
	size_t m_d;
};

}
