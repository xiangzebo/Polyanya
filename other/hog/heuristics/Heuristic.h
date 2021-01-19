#ifndef HEURISTIC_H
#define HEURISTIC_H

class node;
class Heuristic
{
	public:
		Heuristic() { }
		virtual ~Heuristic() { }

		virtual double h(node* first, node* second) const = 0;
};

#endif
