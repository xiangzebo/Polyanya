#ifndef RANGE_H
#define RANGE_H
#include <iterator>
#include <type_traits>
#include <vector>

// Taken from Ben Strasser's GPPC'2014 entry.

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

#endif

