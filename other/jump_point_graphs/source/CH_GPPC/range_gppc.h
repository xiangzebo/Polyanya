#ifndef RANGE_GPPC_H
#define RANGE_GPPC_H
#include <iterator>
#include <type_traits>
#include <vector>
#include "map_iterator.h"
#include "count_iterator.h"

namespace ch_gppc {
//! Stores a pair of iterators that form a range. 
//! \note The elements are not copied but referenced. If the owning container
//!       is destroyed then the range is invalidated.
template<class Iter>
struct Range{
	const Iter&begin()const{return begin_;}
	const Iter&end()const{return end_;}

	Iter begin_, end_;
};

//! Meta-function that maps a range onto the element type over which the range iterates. 
template<class Range>
struct type_of_range_element{
	typedef typename std::decay<decltype(*std::begin(std::declval<Range>()))>::type type;
};

//! Meta-function that maps a range onto the underlying iterator type. 
template<class Range>
struct type_of_range_iterator{
	typedef typename std::decay<decltype(std::begin(std::declval<Range>()))>::type type;
};

//! Returns the number of elements in a range. If the underlying iterators
//! are random access (for example from a vector) then this is a constant time
//! operation. Otherwise it is linear.
template<class Range>
auto range_size(const Range&range)
	->typename std::iterator_traits<typename std::decay<decltype(std::begin(range))>::type>::difference_type
{
	return std::distance(std::begin(range), std::end(range));
}

//! Takes two iterators and returns a range.
//! \note The elements are not copied but referenced.
template<class Iter>
Range<Iter>make_range(Iter begin, Iter end){
	return {begin, end};
}

//! Takes a range and creates another range that traverses the elements in 
//! reversed order. For example [1,4,5,0,3] is transformed into [3,0,5,4,1].
//! \note The elements are not copied but referenced.
template<class R>
auto reverse_range(const R&r)
	->Range<std::reverse_iterator<typename std::decay<decltype(std::begin(r))>::type>>{
	typedef std::reverse_iterator<typename std::decay<decltype(std::begin(r))>::type> Iter;
	return {Iter(std::end(r)), Iter(std::begin(r))};
}

//! Takes a range and and a function map and creates another range that 
//! elements after applying map. For example [1,4,5,0,3] is transformed into
//! [-2,-5,-6,-1,-4] if map is [](int x){return -(x+1);}
//! \note The elements are not copied but referenced.
//! \note The elements can not be modified from the mapped range.
template<class MapFunc, class R>
auto map_range(const R&r, const MapFunc&map)
	->Range<MapIterator<MapFunc, typename std::decay<decltype(std::begin(r))>::type>>{
	typedef MapIterator<MapFunc, typename std::decay<decltype(std::begin(r))>::type> Iter;
	return {Iter(map, std::begin(r)), Iter(map, std::end(r))};
}

//! Creates a range over the integers [0, end).
inline
Range<CountIterator>count_range(int end){
	return {CountIterator(0), CountIterator(end)};
}

//! Creates a range over the integers [begin, end).
inline
Range<CountIterator>count_range(int begin, int end){
	if(end < begin)
		end = begin;
	return {CountIterator(begin), CountIterator(end)};
}

//! Copies the elements of a range and stores the copies in a vector.
template<class Range>
std::vector<typename type_of_range_element<Range>::type>range_to_vector(const Range&range){
	return {std::begin(range), std::end(range)};
}

//! Creates a range that references the elements in a vector. This differs from a plain 
//! std::vector<T> in that passing the range by value does not copy the elements.
template<class T>
Range<typename std::vector<T>::iterator>vector_to_reference_range(std::vector<T>&vec){
	return {vec.begin(), vec.end()};
}

template<class T>
Range<typename std::vector<T>::const_iterator>vector_to_reference_range(const std::vector<T>&vec){
	return {vec.begin(), vec.end()};
}

/*
//! Filter iterator adapter.
//! \note The adapted iterators are at most forward iterators.
//! \warning There is a strong error-prone requirement on the position of the end iterator. 
//!          The end iterator must point to a position past a non-filtered element 
//!          (if the sequence is not empty). If the base iterators are bidirectional 
//!          then three argument constructor can be used to automatically move 
//!          the end iterator to the correct position.
template<class KeepPredicate, class BaseIterator>
class FilterIterator{
public:
	typedef typename std::iterator_traits<BaseIterator>::value_type value_type; 
	typedef typename std::iterator_traits<BaseIterator>::reference reference;
	typedef std::common_type<
		typename std::iterator_traits<BaseIterator>::iterator_category,
		std::forward_iterator_tag
	>::type iterator_category;

	typename std::decay<KeepPredicate>::type keep_predicate;
	BaseIterator base;

	FilterIterator(KeepPredicate keep_predicate, BaseIterator base):
		keep_predicate(std::move(keep_predicate)), 
		base(std::move(base)){

	}

	FilterIterator(KeepPredicate keep_predicate, const BaseIterator&begin, BaseIterator end):
		keep_predicate(std::move(keep_predicate)), 
		base(std::move(end)){
		while(begin != base){
			BaseIterator prev = base;
			--prev;
			if(!keep_predicate(*prev))
				base = prev;
			else
				break;
		}
	}

	FilterIterator&operator++(){
		while(!keep_predicate(base))
			++base;
		++base;
		return *this;
	}

	FilterIterator operator++(int){
		FilterIterator ret = *this;
		++*this;
		return std::move(ret);	
	}

	friend bool operator==(const FilterIterator&l, const FilterIterator&r){return l.base == r.base;}
	friend bool operator!=(const FilterIterator&l, const FilterIterator&r){return !(l == r);}
	
	value_type&operator*()const{
		while(!keep_predicate(base))
			++base;
		return *base;
	}

	value_type*operator->()const{
		return base;
	}
};

//! Filters out elements from a range. 
//! \note The adapted iterators are at most forward iterators.
//! \note The base iterators must be at least bidirectional.
//! \warning Filtered ranges can not be chained.
template<class R, class KeepPredicate>
Range<FilterIterator<KeepPredicate, typename type_of_range_iterator<Range>::type>>filter_range(const R&range, KeepPredicate p)
{
	typedef FilterIterator<KeepPredicate, typename type_of_range_iterator<Range>::type> Iter
	return {Iter(keep_predicate, std::begin(range)), Iter(keep_predicate, std::begin(range), std::end(range))};
}*/
}
#endif

