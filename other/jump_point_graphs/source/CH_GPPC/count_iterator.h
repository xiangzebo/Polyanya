#ifndef COUNT_ITERATOR_H
#define COUNT_ITERATOR_H
#include <type_traits>
#include <utility>
#include <iterator>

namespace ch_gppc {
class CountIterator{
public:
	typedef const int value_type; 
	typedef int difference_type;
	typedef const int* pointer;
	typedef int reference;
	typedef std::random_access_iterator_tag iterator_category;

private:
	int n;
public:
	CountIterator(){}
	explicit CountIterator(int n):n(n){}

	CountIterator&operator++(){
		++n;
		return *this;
	}

	CountIterator operator++(int){
		CountIterator ret = *this;
		++n;
		return std::move(ret);	
	}

	CountIterator&operator--(){
		--n;
		return *this;
	}

	CountIterator operator--(int){
		CountIterator ret = *this;
		--n;
		return std::move(ret);	
	}

	friend difference_type operator-(const CountIterator&l, const CountIterator&r){
		return l.n - r.n;
	}

	friend CountIterator operator-(const CountIterator&l, difference_type r){
		return CountIterator(l.n - r);
	}

	friend CountIterator operator+(const CountIterator&l, difference_type r){
		return CountIterator(l.n + r);
	}

	friend CountIterator operator+(difference_type l, const CountIterator&r){
		return CountIterator(l + r.n);
	}

	CountIterator&operator+=(difference_type o){
		n += o;
		return *this;
	}

	CountIterator&operator-=(difference_type o){
		n -= o;
		return *this;
	}

	friend bool operator==(const CountIterator&l, const CountIterator&r){return l.n == r.n;}
	friend bool operator!=(const CountIterator&l, const CountIterator&r){return l.n != r.n;}
	friend bool operator<=(const CountIterator&l, const CountIterator&r){return l.n <= r.n;}
	friend bool operator>=(const CountIterator&l, const CountIterator&r){return l.n >= r.n;}
	friend bool operator< (const CountIterator&l, const CountIterator&r){return l.n <  r.n;}
	friend bool operator> (const CountIterator&l, const CountIterator&r){return l.n >  r.n;}
	
	value_type operator[](difference_type o)const{
		return n + o;
	}

	value_type operator*()const{
		return n;
	}

	value_type*operator->()const{
		return &n;
	}
};
}
#endif

