#ifndef GRAPH_DEFINITIONS_H
#define GRAPH_DEFINITIONS_H

#include "GraphDefinitions.h"
#include <vector>
#include <string>
#include <stdint.h>
#include <limits>

#include <algorithm>
#include <type_traits>

// Pure abstract class for weighted, directed graphs.
// This is not intended to be used as source code, but rather to check if templating requirements are satisfied.

// Each node has an integer id. The graph can be queried to

// TODO: Make this consistent throughout the source code (added very late).
typedef uint32_t nodeId;
const nodeId kNonNode = std::numeric_limits<nodeId>::max();
//typedef int32_t nodeId;
//const nodeId kNonNode = -1;

typedef nodeId arcId;
const arcId kNonArc = kNonNode;

// TODO: Make this consistent throughout the source code (added very very late).
typedef double Distance;
const Distance kMaxDistance = std::numeric_limits<Distance>::max()/10.0;
//const Distance kEpsDistance = 0.001;
const Distance kEpsDistance = 0.0009765625; // 1/1024

/*
inline bool fless(double a, double b) { return (a < b - kEpsDistance); }
inline bool fgreater(double a, double b) { return (a > b + kEpsDistance); }
inline bool fequal(double a, double b)
{ return (a >= b - kEpsDistance) && (a <= b+kEpsDistance); }

inline double min(double a, double b) { return fless(a, b)?a:b; }
inline double max(double a, double b) { return fless(a, b)?b:a; }
*/

/* Don't want to overload double. Maybe have a special struct for distance?
inline bool operator==(Distance x, Distance y){ return fabs(x-y) < 0.0001; }
inline bool operator> (Distance x, Distance y){ return x - 0.0001 > y;     }
inline bool operator< (Distance x, Distance y){ return x < y - 0.0001;     }
inline bool operator!=(Distance x, Distance y){ return !(x == y); }
inline bool operator<=(Distance x, Distance y){ return !(x >  y); }
inline bool operator>=(Distance x, Distance y){ return !(x <  y); }
*/

// Arcs (taken from Ben Strasser's GPPC code)
struct Arc{
  Arc()
      : source(kNonNode),
        target(kNonNode) {
  }
  Arc(nodeId s, nodeId t)
      : source(s),
        target(t) {
  }
  nodeId source, target;
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

struct ArcHead {
  ArcHead()
      : target(kNonNode) {
  }
  ArcHead(nodeId t)
      : target(t) {
  }
  nodeId target;
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
  return ArcHead(a.target);
}
inline Arc make_arc(nodeId source, ArcHead head){
  return Arc(source, head.target);
}

// Weighted Arc
struct WeightedArc{
  WeightedArc()
      : source(kNonNode),
        target(kNonNode),
        weight(kMaxDistance) {
  }

  WeightedArc(nodeId s, nodeId t, Distance w)
      : source(s),
        target(t),
        weight(w) {
  }
  nodeId source, target;
  Distance weight;
  // int fat[7]; // Why was this here?
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
  WeightedArcHead()
      : target(kNonNode),
        weight(kMaxDistance) {
  }

  WeightedArcHead(nodeId t, Distance w)
      : target(t),
        weight(w) {
  }
  //FIXME: Hacky solution only used at the moment for visualization.
  nodeId GetMid() {return -1;}

  nodeId target;
  Distance weight;
};

static std::ostream& operator <<(std::ostream & out, const WeightedArcHead & a) {
//  out << a.target <<"\t" << a.weight;
//  return out;

  char buffer [1000];
  sprintf(buffer, "(target: %7u, weight: %8.3f)", a.target, a.weight);
  std::string s(buffer);
  out << s;
  return out;
}

inline WeightedArcHead get_head(WeightedArc a){
  return WeightedArcHead(a.target, a.weight);
}

inline WeightedArc make_arc(int source, WeightedArcHead head){
  return WeightedArc(source, head.target, head.weight);
}

// Contraction Arc
struct ContractionArc{
  ContractionArc()
      : source(kNonNode),
        target(kNonNode),
        weight(kMaxDistance),
        mid(kNonNode),
        hops(0) {
  }
  ContractionArc(nodeId s, nodeId t, Distance w, nodeId m, int h)
      : source(s),
        target(t),
        weight(w),
        mid(m),
        hops(h) {
  }
  nodeId source, target;
  Distance weight;
  nodeId mid;
  int hops;
};

inline bool operator==(ContractionArc x, ContractionArc y){
  return x.source == y.source && x.target == y.target && x.weight == y.weight;
}

inline bool operator<(ContractionArc x, ContractionArc y){
  if(x.source != y.source)
    return x.source < y.source;
  if(x.target != y.target)
    return x.target < y.target;
  if(x.weight != y.weight)
    return x.weight < y.weight;
  return false;
}

inline bool operator!=(ContractionArc x, ContractionArc y){ return !(x==y); }
inline bool operator> (ContractionArc x, ContractionArc y){ return   y< x ; }
inline bool operator<=(ContractionArc x, ContractionArc y){ return !(x> y); }
inline bool operator>=(ContractionArc x, ContractionArc y){ return !(x< y); }

struct ContractionArcHead{
  ContractionArcHead()
      : target(kNonNode),
        weight(kMaxDistance),
        mid(kNonNode),
        hops(0) {
  }

  ContractionArcHead(nodeId t, Distance w, nodeId m, int h)
      : target(t),
        weight(w),
        mid(m),
        hops(h) {
  }
  nodeId target;
  Distance weight;
  nodeId mid;
  int hops;
};

inline ContractionArcHead get_head(ContractionArc a){
  return ContractionArcHead(a.target, a.weight, a.mid, a.hops);
}

inline ContractionArc make_arc(int source, ContractionArcHead head){
  return ContractionArc(source, head.target, head.weight, head.mid, head.hops);
}

struct UnpackTriple{
  int source, target, mid;
};

struct ShortcutArc{
  ShortcutArc()
      : source(kNonNode),
        target(kNonNode),
        weight(kMaxDistance),
        mid(kNonNode) {
  }
  ShortcutArc(nodeId s, nodeId t, Distance w, nodeId m)
      : source(s),
        target(t),
        weight(w),
        mid(m) {
  }
  nodeId source;
  nodeId target;
  Distance weight;
  nodeId mid;
};

inline bool operator==(ShortcutArc x, ShortcutArc y){
  return x.source == y.source && x.target == y.target && x.weight == y.weight;
}

inline bool operator<(ShortcutArc x, ShortcutArc y){
  if(x.source != y.source)
    return x.source < y.source;
  if(x.target != y.target)
    return x.target < y.target;
  if(x.weight != y.weight)
    return x.weight < y.weight;
  return false;
}

inline bool operator!=(ShortcutArc x, ShortcutArc y){ return !(x==y); }
inline bool operator> (ShortcutArc x, ShortcutArc y){ return   y< x ; }
inline bool operator<=(ShortcutArc x, ShortcutArc y){ return !(x> y); }
inline bool operator>=(ShortcutArc x, ShortcutArc y){ return !(x< y); }

struct ShortcutArcHead{
  ShortcutArcHead()
      : target(kNonNode),
        weight(kMaxDistance),
        mid(kNonNode) {
  }

  ShortcutArcHead(nodeId t, Distance w, nodeId m)
      : target(t),
        weight(w),
        mid(m) {
  }

  //FIXME: Hacky solution only used at the moment for visualization.
  nodeId GetMid() {return mid;}

  nodeId target;
  Distance weight;
  nodeId mid;
};

inline ShortcutArcHead get_head(ShortcutArc a){
  return ShortcutArcHead(a.target, a.weight, a.mid);
}

inline ShortcutArc make_arc(int source, ShortcutArcHead head){
  return ShortcutArc(source, head.target, head.weight, head.mid);
}

struct Shortcut2Arc{
  Shortcut2Arc()
      : source(kNonNode),
        target(kNonNode),
        weight(kMaxDistance),
        first(kNonArc),
        second(kNonArc){
  }
  Shortcut2Arc(nodeId s, nodeId t, Distance w, arcId f, arcId sc)
      : source(s),
        target(t),
        weight(w),
        first(f),
        second(sc) {
  }
  nodeId source;
  nodeId target;
  Distance weight;
  arcId first, second;
};

inline bool operator==(Shortcut2Arc x, Shortcut2Arc y){
  return x.source == y.source && x.target == y.target && x.weight == y.weight;
}

inline bool operator<(Shortcut2Arc x, Shortcut2Arc y){
  if(x.source != y.source)
    return x.source < y.source;
  if(x.target != y.target)
    return x.target < y.target;
  if(x.weight != y.weight)
    return x.weight < y.weight;
  return false;
}

inline bool operator!=(Shortcut2Arc x, Shortcut2Arc y){ return !(x==y); }
inline bool operator> (Shortcut2Arc x, Shortcut2Arc y){ return   y< x ; }
inline bool operator<=(Shortcut2Arc x, Shortcut2Arc y){ return !(x> y); }
inline bool operator>=(Shortcut2Arc x, Shortcut2Arc y){ return !(x< y); }

struct Shortcut2ArcHead{
  Shortcut2ArcHead()
      : target(kNonNode),
        weight(kMaxDistance),
        first(kNonArc),
        second(kNonArc) {
  }

  Shortcut2ArcHead(nodeId t, Distance w, arcId f, arcId sc)
      : target(t),
        weight(w),
        first(f),
        second(sc) {
  }

  //FIXME: Hacky solution only used at the moment for visualization.
  nodeId GetMid() {return -1;}

  nodeId target;
  Distance weight;
  arcId first, second;
};

inline Shortcut2ArcHead get_head(Shortcut2Arc a){
  return Shortcut2ArcHead(a.target, a.weight, a.first, a.second);
}

inline Shortcut2Arc make_arc(int source, Shortcut2ArcHead head){
  return Shortcut2Arc(source, head.target, head.weight, head.first, head.second);
}


template<class Arc>
Arc reverse_arc(Arc a){
  std::swap(a.source, a.target);
  return a; //std::move(a);
}

struct ArcReverser{
  template<class Arc>
  Arc operator()(Arc a)const{
    return reverse_arc(a);  //(std::move(a));
  }
};

template<class Arc>
void convert_arc(Arc & from, Arc & to) {
  to = from;
}

inline
void convert_arc(ShortcutArc & from, WeightedArc & to) {
  to.source = from.source;
  to.target = from.target;
  to.weight = from.weight;
}

template<class Arc>
struct type_of_arc_head{
  typedef typename std::decay<decltype(get_head(std::declval<Arc>()))>::type type;
};

template<class ArcHead>
struct type_of_arc{
  typedef typename std::decay<decltype(make_arc(0, std::declval<ArcHead>()))>::type type;
};


#endif
