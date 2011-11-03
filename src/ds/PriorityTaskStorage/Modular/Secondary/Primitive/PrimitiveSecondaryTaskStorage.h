/*
 * PrimitiveSecondaryTaskStorage.h
 *
 *  Created on: 21.09.2011
 *      Author: mwimmer
 */

#ifndef PRIMITIVESECONDARYTASKSTORAGE_H_
#define PRIMITIVESECONDARYTASKSTORAGE_H_

namespace pheet {

template <typename TT, template <typename S> class Primary>
class PrimitiveSecondaryTaskStorage {
public:
	typedef TT T;
	typedef struct{
		void print_headers() {}
		void print_values() {}
	} PerformanceCounters;

	PrimitiveSecondaryTaskStorage(Primary<TT>* primary);
	PrimitiveSecondaryTaskStorage(Primary<TT>* primary, PerformanceCounters& perf_count);
	~PrimitiveSecondaryTaskStorage();

	TT steal();

	static void print_name();

private:
	Primary<TT>* primary;

	PerformanceCounters perf_count;

	static T const null_element;
};

template <typename TT, template <typename S> class Primary>
TT const PrimitiveSecondaryTaskStorage<TT, Primary>::null_element = nullable_traits<TT>::null_value;

template <typename TT, template <typename S> class Primary>
inline PrimitiveSecondaryTaskStorage<TT, Primary>::PrimitiveSecondaryTaskStorage(Primary<T>* primary)
: primary(primary) {

}

template <typename TT, template <typename S> class Primary>
inline PrimitiveSecondaryTaskStorage<TT, Primary>::PrimitiveSecondaryTaskStorage(Primary<T>* primary, PerformanceCounters& perf_count)
: primary(primary), perf_count(perf_count) {

}

template <typename TT, template <typename S> class Primary>
inline PrimitiveSecondaryTaskStorage<TT, Primary>::~PrimitiveSecondaryTaskStorage() {

}

template <typename TT, template <typename S> class Primary>
TT PrimitiveSecondaryTaskStorage<TT, Primary>::steal() {
	typename Primary<T>::iterator best;
	prio_t best_prio = 0;

	for(typename Primary<T>::iterator i = primary->begin(); i != primary->end(); ++i) {
		if(!primary->is_taken(i)) {
			prio_t tmp_prio = primary->get_steal_priority(i);
			if(tmp_prio > best_prio) {
				best = i;
				best_prio = tmp_prio;
			}
		}
	}

	if(best_prio == 0) {
		return null_element;
	}
	// We don't care if taking fails, this is just stealing, which is allowed to fail
	return primary->take(best);
}

template <typename TT, template <typename S> class Primary>
void PrimitiveSecondaryTaskStorage<TT, Primary>::print_name() {
	std::cout << "PrimitiveSecondaryTaskStorage";
}

}

#endif /* PRIMITIVESECONDARYTASKSTORAGE_H_ */
