/*
 * CentralKStrategyTaskStorageDataBlock.h
 *
 *  Created on: 25.10.2012
 *      Author: mwimmer
 */

#ifndef CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_H_
#define CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_H_

#include "CentralKStrategyTaskStorageItem.h"
#include <pheet/misc/atomics.h>

namespace pheet {

template <class Pheet, class Place, typename TT, size_t BlockSize, size_t Tests>
class CentralKStrategyTaskStorageDataBlock {
public:
	typedef CentralKStrategyTaskStorageDataBlock<Pheet, Place, TT, BlockSize, Tests> Self;

	typedef CentralKStrategyTaskStorageItem<Pheet, Place, TT> Item;

	CentralKStrategyTaskStorageDataBlock()
	:offset(0), next(nullptr), active_threads(0), active(false) {
		for(size_t i = 0; i < BlockSize; ++i) {
			data[i] = nullptr;
		}
	}
	~CentralKStrategyTaskStorageDataBlock();

	bool put(size_t* head, size_t* tail, Item* item) {
		// Take care not to break correct wraparounds when changing anything
		size_t k = item->strategy->get_k();
		size_t next_offset = offset + BlockSize;
		size_t old_tail = *tail;
		size_t cur_tail = old_tail;
		size_t array_offset = old_tail - offset;

		if(array_offset >= BlockSize) {
			// Next block
			return false;
		}

		while(old_tail != next_offset) {
			size_t cur_k = std::min(k, next_offset - old_tail);

			size_t to_add = Pheet::rand_int<size_t>(cur_k - 1);
			size_t i_limit = to_add + std::min(Tests, cur_k);
			for(size_t i = to_add; i != i_limit; ++i) {
				size_t wrapped_i = i % cur_k;
				if(data[array_offset + wrapped_i] == nullptr) {
					item->position = old_tail + wrapped_i;
					if(PTR_CAS(&(data[array_offset + wrapped_i], nullptr, item))) {
						if(!verify(head, item->position)) {
							data[array_offset + wrapped_i] = nullptr;
							size_t old_pos = old_tail + wrapped_i;
							if(!SIZET_CAS(item->position, old_pos, old_pos + 1)) {
								// Item got eliminated by other thread, success
								// I think linearization point is when item was put into array
								return true;
							}
						}
						else {
							return true;
						}
					}
				}
			}
		}

	}

	bool is_reusable() const {
		return !active;
	}

	void init_first(size_t num_threads) {
		pheet_assert(!active);
		this->active_threads = num_threads;
		this->active = true;
	}

	void deregister() {
		size_t old = SIZET_FETCH_AND_SUB(&active_threads, 1);
		pheet_assert(old > 0);
		if(old == 1) {
			for(size_t i = 0; i < BlockSize; ++i) {
				if(data[i] != nullptr) {
					delete data[i]->strategy;
					data[i]->strategy = nullptr;
					data[i] = nullptr;
				}
			}
			active = false;
			next = nullptr;
		}
	}

	void add_block(Self* block, size_t num_places) {
		block->active_threads = num_places;
		block->active = true;

		Self* pred = this;
		while(true) {
			if(pred->next == nullptr) {
				block->offset = pred->offset + BlockSize;

				if(PTR_CAS(&(pred->next), nullptr, block)) {
					break;
				}
			}
			pred = pred->next;
		}

	}

	bool in_block(size_t position) {
		return position - offset < BlockSize;
	}

	Item* get_item(size_t position) {
		pheet_assert(position - offset < BlockSize);
		return data[position - offset];
	}

	Self* get_next() {
		return next;
	}

	Item* take_rand_filled(size_t position) {
		// Take care not to break correct wraparounds when changing anything
		size_t array_offset = position - offset;

		pheet_assert(array_offset < BlockSize);
		size_t max = BlockSize - array_offset;

		size_t to_add = Pheet::rand_int<size_t>(cur_k - 1);
		size_t limit = to_add + std::min(max, Tests);

		for(size_t i = to_add; i < limit; ++i) {
			size_t index = array_offset + (i % limit);

			Item* item = data[index];
			if(item != nullptr) {
				size_t g_index = offset + index;
				if(item->position == g_index) {
					size_t k = item->strategy->get_k();
					// If k is smaller than the distance to position, tail must have been updated in the meantime.
					// We can't just take this item in this case or we might violate k-ordering
					if((i % limit) < k) {
						if(SIZET_CAS(&(item->position), g_index, g_index + 1)) {
							return item;
						}
					}
				}
			}
		}
		return nullptr;
	}
//	void verify(Item* item, size_t position);

private:
	void reuse(size_t offset, size_t num_threads) {
		pheet_assert(is_reusable());
		this->offset = offset;
		active = true;
	}

	Item* data[BlockSize];

	size_t offset;
	Self* next;

	size_t active_threads;
	bool active;
};

template <class DataBlock>
struct CentralKStrategyTaskStorageDataBlockReuseCheck {
	bool operator()(DataBlock const& block) const {
		return block.is_reusable();
	}
};

} /* namespace pheet */
#endif /* CENTRALKSTRATEGYTASKSTORAGEDATABLOCK_H_ */