/*
 * $Id: heap.cpp,v 1.6 2006/11/29 17:40:14 nathanst Exp $
 *
 * This file is part of HOG.
 *
 * HOG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * HOG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOG; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/ 

// HOG File

#include <iostream>
#include "fpUtil.h"
#include "heap.h"
#include "aStar3.h"

heap::heap(int s, bool minheap)
{
  count = 0;
  _elts.reserve(s);
  this->minheap = minheap;
}

heap::~heap()
{
}

unsigned int heap::size()
{
	return _elts.size();
}

/**
 * Add object into heap.
 */
void heap::add(graph_object *val)
{
  val->key = count;
  _elts.push_back(val);
  count++;
  heapifyUp(val->key);
}

/**
 * Indicate that the key for a particular object has decreased.
 */
void heap::decreaseKey(graph_object *val)
{
	if(minheap)
		heapifyUp(val->key);
	else
		heapifyDown(val->key);
}

// increase key for the object 'val' and reorder the heap accordingly
void heap::increaseKey(graph_object* val)
{
	if(minheap)
		heapifyDown(val->key);
	else
		heapifyUp(val->key);
}

/**
 * Returns true if the object is in the heap.
 */
bool heap::isIn(graph_object *val)
{
  if (val->key < _elts.size() &&
			(_elts[val->key] == val))
		return true;
  return false;
}

/**
 * Remove the item with the highest priority from the heap & re-heapify.
 */
graph_object *heap::remove()
{
  if (empty())
		return 0;
  count--;
  graph_object *ans = _elts[0];
  _elts[0] = _elts[count];
  _elts[0]->key = 0;
  _elts.pop_back();
  heapifyDown(0);

  return ans;
}

// return a pointer to the object on the top of the heap but do not remove it
graph_object* heap::peek()
{
	return _elts[0];
}

/**
 * Returns true if no items are in the heap.
 */
bool heap::empty()
{
  return count == 0;
}

void heap::heapifyUp(int index)
{
  if (index == 0) return;
  int parent = (index-1)/2;

  if (rotate(_elts[parent], _elts[index]))
	{
    graph_object *tmp = _elts[parent];
    _elts[parent] = _elts[index];
    _elts[index] = tmp;
    _elts[parent]->key = parent;
    _elts[index]->key = index;
    heapifyUp(parent);
  }
}

// reorders the subheap with elts_[index] as its root
void heap::heapifyDown(int index)
{
  int child1 = index*2+1;
  int child2 = index*2+2;
  int which;
	
  // find smallest (or largest, depending on heap type) child
  if (child1 >= count)
    return;
  else if (child2 >= count)
    which = child1;
  else if (rotate(_elts[child1], _elts[child2]))
    which = child2;
  else
    which = child1;

  if (rotate(_elts[index], _elts[which]))
	{
    graph_object *tmp = _elts[which];
    _elts[which] = _elts[index];
    _elts[index] = tmp;
    _elts[which]->key = which;
    _elts[index]->key = index;
    heapifyDown(which);
  }
}

// returns true if:
//   minheap is true and the priority of second < first
//   minheap is false and the priority of second > first
bool heap::rotate(graph_object* first, graph_object* second)
{
  if(minheap)
  {
	if (lessThan(second, first)) // NB: arg order (swapped)
	  return true;
	return false;
  }
  else
  {
	  if(greaterThan(second, first)) // NB: arg order 
		  return true;
	  return false;
  }
}

// returns true if key(first) < key(second)
bool heap::lessThan(graph_object* first, graph_object* second)
{
	return fless(first->getKey(), second->getKey());
}

// returns true if key(first) > key(second)
bool heap::greaterThan(graph_object* first, graph_object* second)
{
	return fgreater(first->getKey(), second->getKey());
}
