#ifndef ARC_H
#define ARC_H

#include <algorithm>
#include <type_traits>

struct Arc{
	int source, target;
};

inline
bool operator==(Arc x, Arc y){
	return x.source == y.source && x.target == y.target;
}

inline
bool operator<(Arc x, Arc y){
	if(x.source != y.source)
		return x.source < y.source;
	if(x.target != y.target)
		return x.target < y.target;
	return false;
}

inline bool operator!=(Arc x, Arc y){ return !(x==y); }
inline bool operator> (Arc x, Arc y){ return   y< x ; }
inline bool operator<=(Arc x, Arc y){ return !(x> y); }
inline bool operator>=(Arc x, Arc y){ return !(x< y); }

struct ArcHead{
	int target;
};

inline
bool operator==(ArcHead x, ArcHead y){
	return x.target == y.target;
}

inline
bool operator<(ArcHead x, ArcHead y){
	if(x.target != y.target)
		return x.target < y.target;
	return false;
}

inline bool operator!=(ArcHead x, ArcHead y){ return !(x==y); }
inline bool operator> (ArcHead x, ArcHead y){ return   y< x ; }
inline bool operator<=(ArcHead x, ArcHead y){ return !(x> y); }
inline bool operator>=(ArcHead x, ArcHead y){ return !(x< y); }


inline ArcHead get_head(Arc a){
	return {a.target};
}

inline Arc make_arc(int source, ArcHead head){
	return {source, head.target};
}




struct WeightedArc{
	int source, target, weight;
	int fat[7];
};

inline bool operator==(WeightedArc x, WeightedArc y){
	return x.source == y.source && x.target == y.target && x.weight == y.weight;
}

inline bool operator<(WeightedArc x, WeightedArc y){
	if(x.source != y.source)
		return x.source < y.source;
	if(x.target != y.target)
		return x.target < y.target;
	if(x.weight != y.weight)
		return x.weight < y.weight;
	return false;
}

inline bool operator!=(WeightedArc x, WeightedArc y){ return !(x==y); }
inline bool operator> (WeightedArc x, WeightedArc y){ return   y< x ; }
inline bool operator<=(WeightedArc x, WeightedArc y){ return !(x> y); }
inline bool operator>=(WeightedArc x, WeightedArc y){ return !(x< y); }

struct WeightedArcHead{
	int target, weight;
};

inline WeightedArcHead get_head(WeightedArc a){
	return {a.target, a.weight};
}

inline WeightedArc make_arc(int source, WeightedArcHead head){
	return {source, head.target, head.weight};
}



template<class Arc>
Arc reverse_arc(Arc a){
	std::swap(a.source, a.target);
	return std::move(a);
}

struct ArcReverser{
	template<class Arc>
	Arc operator()(Arc a)const{
		return reverse_arc(std::move(a));
	}
};


template<class Arc>
struct type_of_arc_head{
	typedef typename std::decay<decltype(get_head(std::declval<Arc>()))>::type type;
};

template<class ArcHead>
struct type_of_arc{
	typedef typename std::decay<decltype(make_arc(0, std::declval<ArcHead>()))>::type type;
};


#endif

