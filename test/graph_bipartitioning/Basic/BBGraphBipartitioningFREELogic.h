/*
 * BBGraphBipartitioningFREELogic.h
 *
 *  Created on: Jan 13, 2012
 *      Author: Martin Wimmer, JLT
 *	   License: Boost Software License 1.0 (BSL1.0)
 */

#ifndef BBGRAPHBIPARTITIONINGFREELOGIC_H_
#define BBGRAPHBIPARTITIONINGFREELOGIC_H_

#include "pheet/pheet.h"
#include "../graph_helpers.h"

#include <string.h>
#include <algorithm>

namespace pheet {

template <class Pheet, class SubProblem>
class BBGraphBipartitioningFREELogic {
public:
	typedef BBGraphBipartitioningFREELogic<Pheet, SubProblem> Self;
	typedef typename SubProblem::Set Set;

	BBGraphBipartitioningFREELogic(SubProblem* sub_problem);
	BBGraphBipartitioningFREELogic(SubProblem* sub_problem, Self const& other);
	~BBGraphBipartitioningFREELogic();

	void init_root();
	size_t get_next_vertex();
	size_t get_cut();
	size_t get_lower_bound();
	size_t get_estimate();
	size_t get_upper_bound();
	void update(uint8_t set, size_t pos);
	void update_data(uint8_t set, size_t pos);
	void bulk_update(uint8_t set, Set positions);
	
	size_t get_minnode(uint8_t set);
	size_t get_lowdeg_lower();
	size_t cc_w();

	bool no_edges();
	void assign_deltabound();

	bool can_complete();
	void complete_solution();

	static void print_name();
private:
	SubProblem* sub_problem;

	size_t cut;
	size_t lb;
	size_t nv;
	size_t ub;
	size_t est;
	size_t contrib_sum;
	size_t lb_ub_contrib;

	size_t* weights[2];
	size_t* contributions;
	size_t* free_edges; // JLT - number of free edges (into sets[2]) for each node
	size_t* scanned[2];
	size_t* seen[2];
	size_t* fweight[2];
	size_t deg0; // number of free nodes with no edges
	size_t max_free; // largest degree of free node (approx)
};

template <class Pheet, class SubProblem>
BBGraphBipartitioningFREELogic<Pheet, SubProblem>::BBGraphBipartitioningFREELogic(SubProblem* sub_problem)
  : sub_problem(sub_problem), cut(0), lb(0), nv(0), contrib_sum(0) {
  for (int i=0; i<2; i++) {
    weights[i] = new size_t[sub_problem->size];
    memset(weights[i], 0, sizeof(size_t)*sub_problem->size); // don't like
    scanned[i] = new size_t[sub_problem->size];
    seen[i]  = new size_t[sub_problem->size];
    fweight[i] = new size_t[sub_problem->size];
  }
  contributions = new size_t[sub_problem->size];
  free_edges = new size_t[sub_problem->size];
  
  deg0 = 0; max_free = 0;
  size_t best_contrib = 0;
  size_t best_contrib_i = 0;
  for(size_t i = 0; i < sub_problem->size; ++i) {
    contributions[i] = 0;
    for(size_t j = 0; j < sub_problem->graph[i].num_edges; ++j) {
      contributions[i] += sub_problem->graph[i].edges[j].weight;
    }
    contrib_sum += contributions[i];
    if(contributions[i] > best_contrib) {
      best_contrib = contributions[i];
			best_contrib_i = i;
    }
    free_edges[i] = sub_problem->graph[i].num_edges; // all edges free
    if (free_edges[i]>max_free) max_free = free_edges[i];
    else if (free_edges[i]==0) deg0++;

    for (size_t s=0; s<2; s++) {
      scanned[s][i] = 0;
      seen[s][i]  = 0;
      fweight[s][i] = 0;
    }
  }
  nv = best_contrib_i;
  contrib_sum >>= 1;
}

template <class Pheet, class SubProblem>
BBGraphBipartitioningFREELogic<Pheet, SubProblem>::BBGraphBipartitioningFREELogic(SubProblem* sub_problem, Self const& other)
  : sub_problem(sub_problem), cut(other.cut), lb(other.lb), nv(other.nv), contrib_sum(other.contrib_sum), deg0(other.deg0), max_free(other.max_free) {
    for (int i=0; i<2; i++) {
      weights[i] = new size_t[sub_problem->size];
      memcpy(weights[i], other.weights[i], sizeof(size_t)*sub_problem->size);
      scanned[i] = new size_t[sub_problem->size];
      memcpy(scanned[i], other.scanned[i], sizeof(size_t)*sub_problem->size);
      seen[i] = new size_t[sub_problem->size];
      memcpy(seen[i], other.seen[i], sizeof(size_t)*sub_problem->size);
      fweight[i] = new size_t[sub_problem->size];
      memcpy(fweight[i], other.fweight[i], sizeof(size_t)*sub_problem->size);
    }
    contributions = new size_t[sub_problem->size];
    memcpy(contributions, other.contributions, sizeof(size_t)*sub_problem->size);
    free_edges = new size_t[sub_problem->size];
    memcpy(free_edges,other.free_edges,sizeof(size_t)*sub_problem->size);
}

template <class Pheet, class SubProblem>
BBGraphBipartitioningFREELogic<Pheet, SubProblem>::~BBGraphBipartitioningFREELogic() {
  for (int i=0; i<2; i++) {
    delete[] weights[i];
    delete[] scanned[i];
    delete[] seen[i];
    delete[] fweight[i];
  }
  delete[] contributions;
  delete[] free_edges;
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::init_root() {

}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_next_vertex() {
	pheet_assert(sub_problem->sets[2].test(nv));
	return nv;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_cut() {
	return cut;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_lower_bound() {
	return cut + lb;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_lowdeg_lower() {
  size_t subrem[2];
  subrem[0] = sub_problem->k-sub_problem->sets[0].count();
  subrem[1] = (sub_problem->size-sub_problem->k)-sub_problem->sets[1].count();

  size_t largest_subset = (subrem[0] > subrem[1]) ? subrem[0] : subrem[1];

  if (max_free<largest_subset) return cut + lb; else return 0;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_estimate() {
	pheet_assert(est + contrib_sum >= lb);
	return get_cut() + est + contrib_sum;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_upper_bound() {
	return get_cut() + ub + contrib_sum;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::get_minnode(uint8_t set) {
  // only one free

  size_t w = sub_problem->sets[2]._Find_first();
  
  size_t mincut, sumcut = 0;
  size_t v = sub_problem->sets[2]._Find_next(w);
  while(v != sub_problem->sets[2].size()) {
    sumcut += weights[set][v];
    v = sub_problem->sets[2]._Find_next(v);
  }
  mincut = sumcut+weights[set^1][w]+fweight[set][w];

  v = sub_problem->sets[2]._Find_first();
  while(v != sub_problem->sets[2].size()) {
    sumcut += weights[set][v];
    v = sub_problem->sets[2]._Find_next(v);
    if (v!=sub_problem->sets[2].size()) {
      sumcut -= weights[set][v];
      if (sumcut+weights[set^1][v]+fweight[set][v]<mincut) {
	mincut = sumcut+weights[set^1][v]+fweight[set][v];
	w = v;
      }
    }
  }

  return w;
}

template <class Pheet, class SubProblem>
size_t BBGraphBipartitioningFREELogic<Pheet, SubProblem>::cc_w() {
  // determine smallest weight edge in cc with more edges than missing in smallest subset
	size_t largest_w = std::numeric_limits<size_t>::max();

  size_t h, t;  // head and tail of queue
  size_t c = 0; // component number
  size_t cs;    // component size
  size_t queue[sub_problem->sets[2].count()];
  int cc[sub_problem->size];

  size_t subrem[2];
  subrem[0] = sub_problem->k-sub_problem->sets[0].count();
  subrem[1] = (sub_problem->size-sub_problem->k)-sub_problem->sets[1].count();

  size_t largest_subset = (subrem[0] > subrem[1]) ? subrem[0] : subrem[1];

  size_t v, v0 = sub_problem->sets[2]._Find_first();
  
  size_t minw;

  v = v0;
  while (v!=sub_problem->sets[2].size()) {
    cc[v] = -1;
    v = sub_problem->sets[2]._Find_next(v);
  }

  h = 0; t = 0;
  v = v0;
  while (v!=sub_problem->sets[2].size()) {
    if (cc[v]==-1) {
      // new component
      cc[v] = c; queue[t++] = v; cs = 1;
      minw = largest_w;
      while (h<t) {
	size_t u = queue[h++];
	for(size_t i = 0; i<sub_problem->graph[u].num_edges; i++) {
	  size_t w = sub_problem->graph[u].edges[i].target;
	  if (sub_problem->sets[2].test(w)) {
	    if (sub_problem->graph[u].edges[i].weight<minw) 
	      minw = sub_problem->graph[u].edges[i].weight;
	    if (cc[w]==-1) {
	      cc[w] = c;
	      cs++; // could break already here
	      queue[t++] = w;
	    } else pheet_assert(cc[w]==c);
	  }
	}
      }
      if (cs>largest_subset) {
	//std::cout<<minw<<':'<<cs<<'\n';
	return minw;
      }

      c++; // next component
    }
    v = sub_problem->sets[2]._Find_next(v);
  }

  return 0;
 }

template <class Pheet, class SubProblem>
bool BBGraphBipartitioningFREELogic<Pheet, SubProblem>::no_edges() {
  return (deg0 == sub_problem->sets[2].count());
 }

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::update(uint8_t set, size_t pos) {
	pheet_assert((set & 1) == set);
	pheet_assert(pos < sub_problem->size);
	pheet_assert(!sub_problem->sets[set].test(pos));
	pheet_assert(sub_problem->sets[2].test(pos));

	sub_problem->sets[2].set(pos, false);
	sub_problem->sets[set].set(pos);

	update_data(set, pos);
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::update_data(uint8_t set, size_t pos) {
	cut += weights[set ^ 1][pos];

	size_t f, j;
	size_t subrem[2];
	subrem[0] = sub_problem->k-sub_problem->sets[0].count();
	subrem[1] = (sub_problem->size-sub_problem->k)-sub_problem->sets[1].count();

	pheet_assert(subrem[0]+subrem[1]==sub_problem->sets[2].count());

	if (free_edges[pos]==0) deg0--;
	for(size_t i = 0; i < sub_problem->graph[pos].num_edges; ++i) {
		size_t v = sub_problem->graph[pos].edges[i].target;

		weights[set][v] += sub_problem->graph[pos].edges[i].weight;

		if (sub_problem->sets[2].test(v)) {
		  pheet_assert(contributions[v] >= sub_problem->graph[pos].edges[i].weight);

		  contributions[v] -= sub_problem->graph[pos].edges[i].weight;
		  contrib_sum -= sub_problem->graph[pos].edges[i].weight;

		  if (--free_edges[v]==0) deg0++;

		  for (size_t s=0; s<2; s++) {
		if (sub_problem->graph[pos].edges[i].reverse<scanned[s][v]) {
		  pheet_assert(fweight[s][v]>=sub_problem->graph[pos].edges[i].weight);

		  fweight[s][v] -= sub_problem->graph[pos].edges[i].weight;
		  seen[s][v]--;
		} else {
		  f = seen[s][v]; j = scanned[s][v];
		  while (f>0&&f+subrem[s]>free_edges[v]) { // note >
			j--;
			if (sub_problem->sets[2].test(sub_problem->graph[v].edges[j].target)) {

			  fweight[s][v] -= sub_problem->graph[v].edges[j].weight;
			  f--;
			}
		  }
		  seen[s][v] = f; scanned[s][v] = j;
		}
		  }
		}
	}

	//if (deg0==sub_problem->sets[2].count()) std::cout<<'*';

	size_t nf =  sub_problem->sets[2].count();
	ptrdiff_t delta[nf];

	size_t di = 0;
	lb = 0;
	est = 0;
	ub = 0;
	lb_ub_contrib = 0;
	size_t v, v0 = sub_problem->sets[2]._Find_first();
	nv = v0;
	size_t best_contrib = 0;

	v = v0;
	while(v != sub_problem->sets[2].size()) {
	ptrdiff_t d = (ptrdiff_t)weights[1][v] - (ptrdiff_t)weights[0][v];
	if(std::abs(d) + contributions[v] >= best_contrib) {
	  best_contrib = std::abs(d) + contributions[v];
	  nv = v;
	}

	delta[di++] = d;
	lb += std::min(weights[0][v], weights[1][v]);
	est += std::min(weights[0][v], weights[1][v]);
	ub += std::max(weights[0][v], weights[1][v]);

	v = sub_problem->sets[2]._Find_next(v);
	}
	std::sort(delta, delta + di);

	//size_t pivot = (/*sub_problem->size - */sub_problem->k) - sub_problem->sets[0].count();
	size_t pivot = subrem[0];
	if(pivot < di && delta[pivot] < 0) {
	do {
	  lb += -delta[pivot];
	  est += -delta[pivot];
	  ub -= -delta[pivot];
	  ++pivot;
	} while(pivot < di && delta[pivot] < 0);
	}
	else if(pivot > 0 && delta[pivot - 1] > 0) {
	do {
	  --pivot;
	  lb += delta[pivot];
	  est += delta[pivot];
	  ub -= delta[pivot];
	} while(pivot > 0 && delta[pivot - 1] > 0);
	}

	ptrdiff_t gamma[nf];
	size_t gi = 0;

	size_t ss = (subrem[0] < subrem[1]) ? 0 : 1;
	size_t fw = 0;

	v = v0;
	if (max_free>=subrem[ss]) { // trigger
	max_free = 0;
	while(v != sub_problem->sets[2].size()) {
	  pheet_assert(free_edges[v]<nf);

	  if (free_edges[v]>max_free) max_free = free_edges[v];

	  for (size_t s=0; s<2; s++) {
	if (subrem[s]>0) { // JLT: update should only be called when this is true
	  f = seen[s][v]; j = scanned[s][v];
	  //comment in next two lines to deactivate incremental computation
	  //f = 0; j = 0;
	  //fweight[s][v] = 0;
	  while (f+subrem[s]<=free_edges[v]) { // note <=
		if (sub_problem->sets[2].test(sub_problem->graph[v].edges[j].target)) {
		  fweight[s][v] += sub_problem->graph[v].edges[j].weight;
		  f++;
		}
		j++;
	  }
	  seen[s][v] = f; scanned[s][v] = j;
	}
	  }

	  fw += fweight[1-ss][v];
	  size_t g = fweight[ss][v]-fweight[1-ss][v];
	  pheet_assert(g>=0);
	  if (g>0) gamma[gi++] = g;
	  //if (g>0) std::cout<<g<<'#'<<'\n';

	  v = sub_problem->sets[2]._Find_next(v);
	}

	if (nf-gi<subrem[ss]) {
	  std::sort(gamma,gamma+gi);
	  for (size_t i=0; i<subrem[ss]-(nf-gi); i++) fw += gamma[i];
	}
	fw >>= 1;
	lb += fw;
	}

	if (max_free<subrem[1-ss]) {
	  if (cut+lb<=sub_problem->get_global_upper_bound()&&cut+lb+1000>sub_problem->get_global_upper_bound()) {
		  // How do I transform this?
		  lb += +cc_w();
/*		  if (get_lowdeg_lower()+cc_w(1000)>=sub_problem->get_global_upper_bound()) {
			  return; // actually irrelevant
		  }*/
		  //std::cout<<'#';
			// trigger
			// cc_w should go here
	  }
	}

	//est += fw;
	// Is this correct (MW?)
	ub -= fw;
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::bulk_update(uint8_t set, Set positions) {
  size_t v = positions._Find_first();
  while(v != positions.size()) {
    //update(set, v); // JLT: can be done cheaper
    sub_problem->sets[2].set(v, false);
    sub_problem->sets[set].set(v);

    cut += weights[set ^ 1][v];
	    
    for(size_t i = 0; i < sub_problem->graph[v].num_edges; ++i) {
      size_t w = sub_problem->graph[v].edges[i].target;
	    
      weights[set][w] += sub_problem->graph[v].edges[i].weight;
    }
    v = positions._Find_next(v);
  }

  lb = 0;
  est = 0;
  ub = 0;
  lb_ub_contrib = 0;
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::assign_deltabound() {
  size_t subrem[2];
  subrem[0] = sub_problem->k-sub_problem->sets[0].count();
  subrem[1] = (sub_problem->size-sub_problem->k)-sub_problem->sets[1].count();

  size_t nf =  sub_problem->sets[2].count();
  VertexDelta delta[nf];

  size_t di = 0;
  size_t v = sub_problem->sets[2]._Find_first();
  while(v != sub_problem->sets[2].size()) {
    delta[di].delta = (ptrdiff_t)weights[1][v] - (ptrdiff_t)weights[0][v];
    delta[di].target = v;
    di++;

    v = sub_problem->sets[2]._Find_next(v);
  }
  std::sort(delta, delta + di, delta_compare());

  size_t i;
  // assign to set 0
  for (i=0; i<subrem[0]; i++) {
    v = delta[i].target;
    sub_problem->sets[2].set(v, false);
    sub_problem->sets[0].set(v);

    cut += weights[1][v];
    
    // not needed
    for(size_t i = 0; i < sub_problem->graph[v].num_edges; ++i) {
      size_t w = sub_problem->graph[v].edges[i].target;
      weights[0][w] += sub_problem->graph[v].edges[i].weight;
      pheet_assert(!sub_problem->sets[2].test(w));
      if (sub_problem->sets[2].test(w)) std::cout<<"TROUBLE!!";
    }
  }
  // assign to set 1
  for (; i<di; i++) {
    v = delta[i].target;
    sub_problem->sets[2].set(v, false);
    sub_problem->sets[1].set(v);

    cut += weights[0][v];
    
    // not needed
    for(size_t i = 0; i < sub_problem->graph[v].num_edges; ++i) {
      size_t w = sub_problem->graph[v].edges[i].target;
      weights[1][w] += sub_problem->graph[v].edges[i].weight;
      pheet_assert(!sub_problem->sets[2].test(w));
      if (sub_problem->sets[2].test(w)) std::cout<<"TROUBLE!!";
    }
  }
 }


template <class Pheet, class SubProblem>
bool BBGraphBipartitioningFREELogic<Pheet, SubProblem>::can_complete() {
	return ((sub_problem->sets[0].count() == sub_problem->k-1) ||
			(sub_problem->sets[1].count() == (sub_problem->size - sub_problem->k)-1)) ||
			no_edges();
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::complete_solution() {
	size_t s;
	if(sub_problem->sets[0].count() == sub_problem->k-1) {
		s = 0;
	}
	else if(sub_problem->sets[1].count() == (sub_problem->size - sub_problem->k)-1) {
		s = 1;
	}
	else {
		pheet_assert(no_edges());

		assign_deltabound();
		return;
	}

	//std::cout<<'#';

	size_t v = get_minnode(s);

	update(s,v);

	sub_problem->sets[1-s] |= sub_problem->sets[2];
	Set tmp = sub_problem->sets[2];
	sub_problem->sets[2].reset();
	bulk_update(1-s, tmp);
}

template <class Pheet, class SubProblem>
void BBGraphBipartitioningFREELogic<Pheet, SubProblem>::print_name() {
	std::cout << "FREELogic";
}

}

#endif /* BBGRAPHBIPARTITIONINGFREELOGIC_H_ */