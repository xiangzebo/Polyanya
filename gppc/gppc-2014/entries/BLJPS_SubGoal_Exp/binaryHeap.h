/**
  \file BinaryHeap.h
  \author Eduard Igushev visit <www.igushev.com> e-mail: <first name at last name dot com>
  \brief Binary Heap C++ implementation
 
  Warranty and license
  The implementation is provided “as it is” with no warranty.
  Any private and commercial usage is allowed.
  Keeping the link to the source is required.
  Any feedback is welcomed :-)
*/
 
#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H
 
#include <vector>
#include <iterator>
#include <cmath>
#include "Node.h"
 

class BinaryHeap {
  std::vector<Node*> _heap;
 
public:
  BinaryHeap() {}
  BinaryHeap(Node* first, Node* last);
  ~BinaryHeap() {}
 
  void Insert(Node*);
  bool InsertSmaller(Node*);
  Node* PopMax();
  const Node* Max() const { return _heap.front(); }
  unsigned Count() const { return _heap.size(); }
  void clear(){_heap.clear();}
  bool hasSmallerElement(float element)
  {
	 for (unsigned int i =0;i<_heap.size();i++)
		 if (_heap[i]->total<element)
			 return true;
	 return false;
  }
  int getHeapCapacity()
  {
	  return _heap.capacity();
  }
private:
  typedef std::vector<Node*>::size_type _heap_size_t;
 
  void _buildHeap();
  void _shiftDown(_heap_size_t node);
  void _shiftUp(_heap_size_t node);
};
 
BinaryHeap::BinaryHeap(Node* first, Node* last)
{
  _heap_size_t n = std::distance(first, last);
  _heap.reserve(n);
  for (Node* in = first; in != last; ++in)
    _heap.push_back(in);
  _buildHeap();
}
 

void BinaryHeap::Insert(Node* value)
{
  _heap.push_back(value);
  _shiftUp(_heap.size()-1);
}

bool BinaryHeap::InsertSmaller(Node* value)
{
  for (unsigned int i =0;i<_heap.size();i++)
	  if (value->pos.x == _heap[i]->pos.x && _heap[i]->pos.y == value->pos.y)
	  {
		  if (value->total<_heap[i]->total)
		  {
			 // _heap.erase(_heap.begin()+i);
			 // Insert(value);
			  _heap[i]=value;
				_shiftUp(i);

			  return true;
		  }
		  else
			  return false;
	  }
 return false;

}
Node* BinaryHeap::PopMax()
{
  Node* result = _heap.front();
  _heap.front() = _heap.back();
  _heap.pop_back();
  _shiftDown(0);
  return result;
}
 

void BinaryHeap::_buildHeap()
{
  for (_heap_size_t i = _heap.size()>>1; i >= 0; --i)
    _shiftDown(i);
}
 

void BinaryHeap::_shiftDown(_heap_size_t node)
{
  _heap_size_t left_child = (node<<1)+1;
  _heap_size_t right_child = (node<<1)+2;
 
  _heap_size_t replace = node;
  if (right_child < _heap.size())
  {
    bool left = _heap[right_child]->total > _heap[left_child]->total;
    if (left && _heap[node]->total > _heap[left_child]->total)
      replace = left_child;
    else if (!left && _heap[node]->total > _heap[right_child]->total)
      replace = right_child;
  }
  else if (left_child < _heap.size())
  {
    if (_heap[node]->total > _heap[left_child]->total)
      replace = left_child;
  }
 
  if (replace == node)
    return;
  std::swap(_heap[node], _heap[replace]);
  _shiftDown(replace);
}
 

void BinaryHeap::_shiftUp(_heap_size_t node)
{
  if (node == 0)
    return;
  _heap_size_t parent = (node - 1) >> 1;
  //_heap_size_t parent2 = std::floor((node-1.0f)/2.0f);

  if (_heap[node]->total > _heap[parent]->total)
    return;
  std::swap(_heap[node], _heap[parent]);
  _shiftUp(parent);
}
 
#endif