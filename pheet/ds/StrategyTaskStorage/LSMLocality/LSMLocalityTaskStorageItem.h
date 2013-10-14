/*
 * LSMLocalityTaskStorageItem.h
 *
 *  Created on: Sep 18, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LSMLOCALITYTASKSTORAGEITEM_H_
#define LSMLOCALITYTASKSTORAGEITEM_H_

#include <atomic>

namespace pheet {

template <class Pheet, class Place, class Frame, class FrameReg, class BaseItem, class Strategy>
struct LSMLocalityTaskStorageItem : public BaseItem {
	typedef typename BaseItem::T T;

	LSMLocalityTaskStorageItem()
	:owner(nullptr), used_locally(false), frame(nullptr), last_phase(0) {
	}

	/*
	 * Is only safe to be called if the thread owns the item or is registered to the frame
	 */
	bool is_taken() {
		return last_phase.load(std::memory_order_acquire) != -1;
	}

	/*
	 * Is only safe to be called if the thread owns the item or is registered to the frame
	 */
	bool is_taken_or_dead() {
		return last_phase.load(std::memory_order_acquire) != -1 || strategy.dead_task();
	}

	T take() {
		ptrdiff_t expected = -1;
		if(last_phase.compare_exchange_strong(expected, frame.load(std::memory_order_relaxed)->get_take_phase(), std::memory_order_release, std::memory_order_acquire)) {
			this->taken.store(true, std::memory_order_relaxed);
			return this->data;
		}
		return nullable_traits<T>::null_value;
	}

	/*
	 * Local version of take_and_delete, called by owner
	 * Tries to take item and deletes it on success
	 */
	void take_and_delete() {
		ptrdiff_t expected = -1;
		if(last_phase.compare_exchange_strong(expected, frame.load(std::memory_order_relaxed)->get_take_phase(), std::memory_order_release, std::memory_order_acquire)) {
			this->taken.store(true, std::memory_order_relaxed);

			this->data.drop_item();
		}
	}

	Place* owner;
	Strategy strategy;
	bool used_locally;

	std::atomic<Frame*> frame;
	std::atomic<ptrdiff_t> last_phase;
};

template <class Item, class Frame>
struct LSMLocalityTaskStorageItemReuseCheck {
	bool operator() (Item const& item) const {
		Frame* f = item.frame.load(std::memory_order_relaxed);
		// item.taken is always set after last_phase is set, therefore use this for checks
		return item.taken && !item.used_locally && (f == nullptr || f->can_reuse(item.last_phase.load(std::memory_order_relaxed)));
	}
};

} /* namespace pheet */
#endif /* LSMLOCALITYTASKSTORAGEITEM_H_ */