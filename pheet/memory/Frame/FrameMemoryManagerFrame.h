/*
 * FrameMemoryManagerFrame.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FRAMEMEMORYMANAGERFRAME_H_
#define FRAMEMEMORYMANAGERFRAME_H_

namespace pheet {

template <class Pheet>
class FrameMemoryManagerFrame {
public:
	FrameMemoryManagerFrame()
	:phase(0), _phase_change_required(false), items(0), reusable_items(0), owner_reg(0) {
		registered[0].store(0, std::memory_order_relaxed);
		registered[1].store(-1, std::memory_order_relaxed);
	}

	~FrameMemoryManagerFrame() {}


	bool can_progress_phase() {
		size_t p = phase.load(std::memory_order_relaxed);

		// Check status of previous phase
		s_procs_t ro = registered[(p&1)^1].load(std::memory_order_relaxed);
		if(ro == -1) {
			// Previous phase has already been finished
			return true;
		}
		else if(ro == 0) {
			// We can try to finalize the previous phase
			if(registered[(p&1)^1].compare_exchange_strong(ro, -1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
				// Previous phase is finalized now
				return true;
			}
		}
		return false;
	}

	void progress_phase() {
		size_t p = phase.load(std::memory_order_relaxed);
		pheet_assert(registered[(p&1)^1].load(std::memory_order_relaxed) == -1);

		// First reopen the position for the new phase
		registered[(p&1)^1].store(0, std::memory_order_release);
		// Then release new phase id (reopening of position needs to have happened before)
		phase.store((p+1) & wraparound, std::memory_order_release);

		reusable_items = 0;
		_phase_change_required.store(false, std::memory_order_relaxed);
	}

	void register_reusable() {
		++reusable_items;
		if(reusable_items >= (items >> 1)) {
			_phase_change_required.store(true, std::memory_order_relaxed);
		}
	}

	bool can_reuse(ptrdiff_t last_phase) {
		pheet_assert(last_phase >= 0);

		size_t p = phase.load(std::memory_order_relaxed);
		if(((size_t)(last_phase + 1) & wraparound) == p) {
			if(can_progress_phase()) {
				// Old phase has been closed
				return true;
			}
			return false;
		}
		else if((size_t)last_phase == p) {
			if(phase_change_required() && can_progress_phase()) {
				progress_phase();

				if(can_progress_phase()) {
					return true;
				}
			}
			return false;
		}

		return true;
	}

	size_t register_place() {
		while(true) {
			size_t p = phase.load(std::memory_order_relaxed);
			s_procs_t r = registered[p&1].load(std::memory_order_acquire);

			pheet_assert(r >= -1);

			while(r == -1 || !registered[p&1].compare_exchange_weak(r, r + 1, std::memory_order_acquire, std::memory_order_acquire)) {
				p = phase.load(std::memory_order_relaxed);
				r = registered[p&1].load(std::memory_order_acquire);
			}

			// Check if the phase is still the same
			size_t p2 = phase.load(std::memory_order_acquire);
			if((p2 & 1) == (p & 1)) {
				// As long as the new phase uses the same slot we are fine
				return p2;
			}
			deregister_place(p);
		}
	}

	/*
	 * Wait-free variant
	 */
	bool try_register_place(size_t& ret_phase) {
		size_t p = phase.load(std::memory_order_relaxed);
		s_procs_t r = registered[p&1].load(std::memory_order_acquire);

		pheet_assert(r >= -1);

		if(r == -1 || !registered[p&1].compare_exchange_weak(r, r + 1, std::memory_order_acquire, std::memory_order_acquire)) {
			return false;
		}
		size_t p2 = phase.load(std::memory_order_acquire);
		if((p2 & 1) == (p & 1)) {
			ret_phase = p2;
			return true;
		}
		deregister_place(p);
		return false;
	}

	void deregister_place(size_t phase) {
		registered[phase&1].fetch_sub(1, std::memory_order_acq_rel);
	}

	/*
	 * Uppermost bit is never used, so it is safe to be casted to a signed type as well
	 */
	size_t get_phase() {
		return phase.load(std::memory_order_relaxed);
	}

	bool phase_change_required() {
		return _phase_change_required.load(std::memory_order_relaxed);
	}

	void item_added() {
		++items;
		++owner_reg;
	}

private:
	std::atomic<s_procs_t> registered[2];
	std::atomic<size_t> phase;
	std::atomic<bool> _phase_change_required;

	size_t items;
	size_t reusable_items;
	size_t owner_reg;

	static size_t wraparound;
};


/*
 * Making the wraparound below the highest bit simplifies wraparound and allows for safe casting to
 * signed types
 */
template <class Pheet>
size_t FrameMemoryManagerFrame<Pheet>::wraparound = std::numeric_limits<size_t>::max() >> 1;


template <class Frame>
struct FrameMemoryManagerFrameReuseCheck {
	bool operator() (Frame const& frame) const {
		return !frame.phase_change_required();
	}
};

} /* namespace pheet */
#endif /* FRAMEMEMORYMANAGERFRAME_H_ */
