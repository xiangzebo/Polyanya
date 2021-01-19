#ifndef ARC_H
#define ARC_H


#include <algorithm>
#include <string>
#include <type_traits>

namespace ch_gppc {

struct UnpackTripleGPPC{
  int source, target, mid;
};

struct ShortcutArcGPPC{
  int source, target, weight, mid;
};

struct ShortcutArcHeadGPPC{
  int target, weight, mid;
};

inline ShortcutArcHeadGPPC get_head(ShortcutArcGPPC a){
  return {a.target, a.weight, a.mid};
}

inline ShortcutArcGPPC make_arc(int source, ShortcutArcHeadGPPC head){
  return {source, head.target, head.weight, head.mid};
}


struct ArcGPPC{
  int source, target;
};

inline
bool operator==(ArcGPPC x, ArcGPPC y){
  return x.source == y.source && x.target == y.target;
}

inline
bool operator<(ArcGPPC x, ArcGPPC y){
  if(x.source != y.source)
    return x.source < y.source;
  if(x.target != y.target)
    return x.target < y.target;
  return false;
}

inline bool operator!=(ArcGPPC x, ArcGPPC y){ return !(x==y); }
inline bool operator> (ArcGPPC x, ArcGPPC y){ return   y< x ; }
inline bool operator<=(ArcGPPC x, ArcGPPC y){ return !(x> y); }
inline bool operator>=(ArcGPPC x, ArcGPPC y){ return !(x< y); }

struct ArcHeadGPPC{
  int target;
};

inline
bool operator==(ArcHeadGPPC x, ArcHeadGPPC y){
  return x.target == y.target;
}

inline
bool operator<(ArcHeadGPPC x, ArcHeadGPPC y){
  if(x.target != y.target)
    return x.target < y.target;
  return false;
}

inline bool operator!=(ArcHeadGPPC x, ArcHeadGPPC y){ return !(x==y); }
inline bool operator> (ArcHeadGPPC x, ArcHeadGPPC y){ return   y< x ; }
inline bool operator<=(ArcHeadGPPC x, ArcHeadGPPC y){ return !(x> y); }
inline bool operator>=(ArcHeadGPPC x, ArcHeadGPPC y){ return !(x< y); }


inline ArcHeadGPPC get_head(ArcGPPC a){
  return {a.target};
}

inline ArcGPPC make_arc(int source, ArcHeadGPPC head){
  return {source, head.target};
}

struct WeightedArcGPPC{
  int source, target, weight;
  int fat[7];
};

inline bool operator==(WeightedArcGPPC x, WeightedArcGPPC y){
  return x.source == y.source && x.target == y.target && x.weight == y.weight;
}

inline bool operator<(WeightedArcGPPC x, WeightedArcGPPC y){
  if(x.source != y.source)
    return x.source < y.source;
  if(x.target != y.target)
    return x.target < y.target;
  if(x.weight != y.weight)
    return x.weight < y.weight;
  return false;
}

inline bool operator!=(WeightedArcGPPC x, WeightedArcGPPC y){ return !(x==y); }
inline bool operator> (WeightedArcGPPC x, WeightedArcGPPC y){ return   y< x ; }
inline bool operator<=(WeightedArcGPPC x, WeightedArcGPPC y){ return !(x> y); }
inline bool operator>=(WeightedArcGPPC x, WeightedArcGPPC y){ return !(x< y); }

struct WeightedArcHeadGPPC{
  int target, weight;
};

inline WeightedArcHeadGPPC get_head(WeightedArcGPPC a){
  return {a.target, a.weight};
}

inline WeightedArcGPPC make_arc(int source, WeightedArcHeadGPPC head){
  return {source, head.target, head.weight};
}



template<class ArcGPPC>
ArcGPPC reverse_arc(ArcGPPC a){
  std::swap(a.source, a.target);
  return std::move(a);
}

struct ArcReverserGPPC{
  template<class ArcGPPC>
  ArcGPPC operator()(ArcGPPC a)const{
    return reverse_arc(std::move(a));
  }
};


template<class ArcGPPC>
struct type_of_arc_head{
  typedef typename std::decay<decltype(get_head(std::declval<ArcGPPC>()))>::type type;
};

template<class ArcHeadGPPC>
struct type_of_arc{
  typedef typename std::decay<decltype(make_arc(0, std::declval<ArcHeadGPPC>()))>::type type;
};

}

#endif

