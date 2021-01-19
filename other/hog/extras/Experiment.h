#ifndef EXPERIMENT_H
#define EXPERIMENT_H

// Experimenth.h
//
// An object for holding experiments read from .scenario files
//
// NB: "scale" is misleading; it refers to the dimension of the
// scaled map, not the scaling factor!
//
// @author: Renee Jansen
// @created: 5/2/2006
//

#include <vector>
#include <string>
#include <cstring>

using std::string;

static const int kNoScaling = -1;

class Experiment{
	public:
		Experiment(int sx,int sy,int gx,int gy,int b, double d, string m);
		Experiment(int sx,int sy,int gx,int gy,int sizeX, int sizeY,int b, 
				double d, string m);
		virtual ~Experiment();

		inline int getStartX(){return startx;}
		inline int getStartY(){return starty;}
		inline int getGoalX(){return goalx;}
		inline int getGoalY(){return goaly;}
		inline int getBucket(){return bucket;}
		inline double getDistance(){return distance;}
		inline void getMapName(char* mymap){strcpy(mymap,map.c_str());}
		inline const char *getMapName() { return map.c_str(); }
		inline int getXScale(){return scaleX;}
		inline int getYScale(){return scaleY;}
		inline void setPrecision(int prec) { precision = prec; }
		inline int getPrecision() { return precision; }

		virtual  void print(std::ostream& out);

	private:
		int startx, starty, goalx, goaly;
		int scaleX;
		int scaleY;
		int bucket;
		double distance;
		string map;
		int precision;
};

#endif
