/*
 * TASLock.h
 *
 *  Created on: 22.04.2011
 *      Author: Martin Wimmer
 *     License: Pheet License
 */

#ifndef TASLOCK_H_
#define TASLOCK_H_

namespace pheet {

template <class Pheet>
class TASLock {
public:
	TASLock();
	~TASLock();

	void lock();
	bool try_lock();
	bool try_lock(long int time_ms);

	void unlock();
private:
	int locked;
};

template <class Pheet>
TASLock<Pheet>::TASLock()
:locked(0) {

}

template <class Pheet>
TASLock<Pheet>::~TASLock() {
}

template <class Pheet>
void TASLock<Pheet>::lock() {
	while(!INT_CAS(&locked, 0, 1)) {};
}

template <class Pheet>
bool TASLock<Pheet>::try_lock() {
	return INT_CAS(&locked, 0, 1);
}

template <class Pheet>
bool TASLock<Pheet>::try_lock(long int time_ms) {
	struct timeval begin;
	gettimeofday(&begin, NULL);
	long int begin_ms = (begin.tv_usec / 1000) + (begin.tv_sec * 1000);
	while(!INT_CAS(&locked, 0, 1)) {
		struct timeval current;
		gettimeofday(&current, NULL);
		long int current_ms = (current.tv_usec / 1000) + (current.tv_sec * 1000);
		if(current_ms - begin_ms >= time_ms) {
			return false;
		}
	};
	return true;
}

template <class Pheet>
void TASLock<Pheet>::unlock() {
	locked = 0;
}


}

#endif /* TASLOCK_H_ */
