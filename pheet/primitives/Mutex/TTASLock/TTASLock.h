/*
 * TASLock.h
 *
 *  Created on: 10.08.2011
 *      Author: Martin Wimmer
 *     License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef TTASLOCK_H_
#define TTASLOCK_H_

#include <iostream>
#include <chrono>
#include "../common/BasicLockGuard.h"

namespace pheet {

template <class Pheet>
class TTASLock {
public:
	typedef TTASLock<Pheet> Self;
	typedef BasicLockGuard<Pheet, Self> LockGuard;

	TTASLock();
	~TTASLock();

	void lock();
	bool try_lock();
	bool try_lock(long int time_ms);

	void unlock();

	static void print_name() {
		std::cout << "TTASLock";
	}
private:
	// Volatile needed to ensure the compiler doesn't optimize the while(locked) loop
	int volatile locked;
};

template <class Pheet>
TTASLock<Pheet>::TTASLock()
:locked(0) {

}

template <class Pheet>
TTASLock<Pheet>::~TTASLock() {
}

template <class Pheet>
void TTASLock<Pheet>::lock() {
	do {
		while(locked);
	}
	while(!INT_CAS(&locked, 0, 1));
}

template <class Pheet>
bool TTASLock<Pheet>::try_lock() {
	return (!locked) && INT_CAS(&locked, 0, 1);
}

template <class Pheet>
bool TTASLock<Pheet>::try_lock(long int time_ms) {
	auto start_time = std::chrono::high_resolution_clock::now();
	while(!INT_CAS(&locked, 0, 1)) {
		do {
			auto stop_time = std::chrono::high_resolution_clock::now();
			long int cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
			if(cur_time >= time_ms) {
				return false;
			}
		}while(locked);
	};
	return true;
}

template <class Pheet>
void TTASLock<Pheet>::unlock() {
	MEMORY_FENCE();
	locked = 0;
}

}

#endif /* TASLOCK_H_ */
