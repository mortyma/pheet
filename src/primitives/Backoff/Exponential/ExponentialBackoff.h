/*
 * ExponentialBackoff.h
 *
 *  Created on: 18.04.2011
 *   Author(s): Martin Wimmer
 *     License: Ask author
 */

#ifndef EXPONENTIALBACKOFF_H_
#define EXPONENTIALBACKOFF_H_

#include <stdlib.h>
#include <time.h>
#include <algorithm>

/*
 *
 */
namespace pheet {

template <class Pheet, unsigned int MIN_BACKOFF = 100, unsigned int MAX_BACKOFF = 100000>
class ExponentialBackoff : protected Pheet {
public:
	ExponentialBackoff();
	~ExponentialBackoff();

	void backoff();
private:
	unsigned int limit;

//	static thread_local boost::mt19937 rng;
};
/*
template <unsigned int MIN_BACKOFF, unsigned int MAX_BACKOFF>
thread_local boost::mt19937 ExponentialBackoff<MIN_BACKOFF, MAX_BACKOFF>::rng;
*/
template <class Pheet, unsigned int MIN_BACKOFF, unsigned int MAX_BACKOFF>
ExponentialBackoff<Pheet, MIN_BACKOFF, MAX_BACKOFF>::ExponentialBackoff() {
	limit = MIN_BACKOFF;
}

template <class Pheet, unsigned int MIN_BACKOFF, unsigned int MAX_BACKOFF>
ExponentialBackoff<Pheet, MIN_BACKOFF, MAX_BACKOFF>::~ExponentialBackoff() {
}

template <class Pheet, unsigned int MIN_BACKOFF, unsigned int MAX_BACKOFF>
void ExponentialBackoff<Pheet, MIN_BACKOFF, MAX_BACKOFF>::backoff() {
//	boost::uniform_int<unsigned int> rnd_gen(0, limit);
	unsigned int sleep = rand_int(limit); //rnd_gen(rng);

	timespec delay;
	delay.tv_sec = (time_t)0;
	delay.tv_nsec = sleep;
	// let's sleep
	(void)nanosleep(&delay, (timespec *)NULL);

	limit = std::min(limit + sleep, MAX_BACKOFF);
}

template <class Pheet>
class StandardExponentialBackoff : public ExponentialBackoff<Pheet, 100, 100000> {

};

}

#endif /* EXPONENTIALBACKOFF_H_ */
