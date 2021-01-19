#include "NoInsertionPolicy.h"
#include "graph.h"

NoInsertionPolicy::NoInsertionPolicy()
	: InsertionPolicy()
{
}

NoInsertionPolicy::~NoInsertionPolicy()
{
}

node*
NoInsertionPolicy::insert(node* n) throw(std::invalid_argument)
{
	return n;
}

void 
NoInsertionPolicy::remove(node* n)
	throw(std::runtime_error)
{
	return;
}
