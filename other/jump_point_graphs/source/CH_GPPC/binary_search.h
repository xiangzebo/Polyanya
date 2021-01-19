#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include <cassert>

namespace ch_gppc {
//! The predicate p maps [begin, end) onto true or false. The function
//! assumes there is a position mid such that [begin,mid) is false and
//! [mid,end) is true. The function then returns mid. If [begin,mid) is false
//! then end is returned. In all other cases the behaviour is undefined.
template<class Iter, class Pred>
Iter binary_find_first_true(Iter begin, Iter end, Pred p){
	if(begin == end)
		return end;
	if(!p(*(end-1)))
		return end;

	while(end - begin > 1){
		Iter mid = begin + (end-begin-1)/2;

		if(p(*mid))
			end = mid+1;
		else
			begin = mid+1;
	}
	return begin;
}

//! The predicate p maps [begin, end) onto true or false. The function
//! assumes there is a position mid such that [begin,mid] is true and
//! (mid,end) is false. The function then returns mid. If
//! no such mid exists then the behaviour is undefined.
template<class Iter, class Pred>
Iter binary_find_last_true(Iter begin, Iter end, Pred p){
	assert(begin != end);
	assert(p(*begin));

	while(end - begin > 1){
		Iter mid = begin + (end-begin)/2;

		if(p(*mid))
			begin = mid;
		else
			end = mid;
	}
	return begin;
}
}

#endif

