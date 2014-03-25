/*
 * FrameMemoryManagerPlaceSingleton.h
 *
 *  Created on: Mar 24, 2014
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef FRAMEMEMORYMANAGERPLACESINGLETON_H_
#define FRAMEMEMORYMANAGERPLACESINGLETON_H_

#include <unordered_map>

#include <pheet/memory/ItemReuse/ItemReuseMemoryManager.h>
#include "FrameMemoryManagerFrame.h"
#include "FrameMemoryManagerFrameLocalView.h"

namespace pheet {

template <class Pheet>
class FrameMemoryManagerPlaceSingleton {
public:
	typedef FrameMemoryManagerFrame<Pheet> Frame;
	typedef FrameMemoryManagerFrameLocalView<Pheet> LV;

	typedef ItemReuseMemoryManager<Pheet, Frame, FrameMemoryManagerFrameReuseCheck<Frame> > FrameMemoryManager;

	FrameMemoryManagerPlaceSingleton() {}
	~FrameMemoryManagerPlaceSingleton() {}

	void reg(Frame* frame, size_t& phase) {
		LV& reg = frame_regs[frame];
		phase = frame->get_phase();

		while(!reg.try_reg(frame, phase)) {
			phase = frame->get_phase();
		}
	}

	bool try_reg(Frame* frame, size_t& phase) {
		LV& reg = frame_regs[frame];
		phase = frame->get_phase();

		return reg.try_reg(frame, phase);
	}

	void rem_reg(Frame* frame, size_t phase) {
		LV& reg = frame_regs[frame];

		reg.rem_reg(frame, phase);
		if(reg.empty()) {
			frame_regs.erase(frame);
		}
	}

	Frame* next_frame() {
		return &(frames.acquire_item());
	}

private:
	FrameMemoryManager frames;
	std::unordered_map<Frame*, LV> frame_regs;
};

} /* namespace pheet */
#endif /* FRAMEMEMORYMANAGERPLACESINGLETON_H_ */
