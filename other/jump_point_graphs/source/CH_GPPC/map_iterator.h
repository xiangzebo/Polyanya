#ifndef MAP_ITERATOR_H
#define MAP_ITERATOR_H
#include <type_traits>
#include <utility>
#include <iterator>

namespace ch_gppc {
template<class MapFunc, class BaseIterator>
class MapIterator{
public:
	typedef 
		typename std::add_const<
			decltype(
				(std::declval<MapFunc>())(
					std::declval<typename std::iterator_traits<BaseIterator>::value_type>()
				)
			)
		>::type 
	value_type; 
	typedef typename std::iterator_traits<BaseIterator>::difference_type difference_type;
	typedef typename std::iterator_traits<BaseIterator>::pointer pointer;
	typedef typename std::iterator_traits<BaseIterator>::reference reference;
	typedef typename std::iterator_traits<BaseIterator>::iterator_category iterator_category;

	typename std::decay<MapFunc>::type map;
	BaseIterator base;

private:
	mutable typename std::remove_const<value_type>::type value;
public:
	MapIterator(MapFunc map, BaseIterator base):map(std::move(map)), base(std::move(base)){}

	MapIterator&operator++(){
		++base;
		return *this;
	}

	MapIterator operator++(int){
		MapIterator ret = *this;
		++base;
		return std::move(ret);	
	}

	MapIterator&operator--(){
		--base;
		return *this;
	}

	MapIterator operator--(int){
		MapIterator ret = *this;
		--base;
		return std::move(ret);	
	}

	friend difference_type operator-(const MapIterator&l, const MapIterator&r){
		return l.base - r.base;
	}

	friend MapIterator operator-(const MapIterator&l, difference_type r){
		return MapIterator(l.map, l.base - r);
	}

	friend MapIterator operator+(const MapIterator&l, difference_type r){
		return MapIterator(l.map, l.base + r);
	}

	friend MapIterator operator+(difference_type l, const MapIterator&r){
		return MapIterator(r.map, l + r.base);
	}

	MapIterator&operator+=(difference_type o){
		base += o;
		return *this;
	}

	MapIterator&operator-=(difference_type o){
		base -= o;
		return *this;
	}

	friend bool operator==(const MapIterator&l, const MapIterator&r){return l.base == r.base;}
	friend bool operator!=(const MapIterator&l, const MapIterator&r){return l.base != r.base;}
	friend bool operator<=(const MapIterator&l, const MapIterator&r){return l.base <= r.base;}
	friend bool operator>=(const MapIterator&l, const MapIterator&r){return l.base >= r.base;}
	friend bool operator< (const MapIterator&l, const MapIterator&r){return l.base <  r.base;}
	friend bool operator> (const MapIterator&l, const MapIterator&r){return l.base >  r.base;}
	
	value_type&operator[](difference_type o)const{
		value = map(base[o]);
		return value;
	}

	value_type&operator*()const{
		value = map(*base);
		return value;
	}

	value_type*operator->()const{
		value = map(*base);
		return &value;
	}
};

}
#endif

