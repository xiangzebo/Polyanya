// Entrance.h
//
// Data structure describing entrances between two adjacent clusters
//
// @author: dharabor
// @created: 18/06/2010

#ifndef ENTRANCE_H
#define ENTRANCE_H

enum EntranceType { HORIZONTAL, VERTICAL };

class Entrance
{
	public:
		Entrance();
		Entrance(int x, int y, int length, EntranceType type);
		~Entrance();

		double x;
		double y;
		unsigned int length;
		EntranceType type;
};

#endif 
