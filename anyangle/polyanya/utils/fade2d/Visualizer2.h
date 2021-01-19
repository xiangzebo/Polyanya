// (c) 2010 Geom e.U. Bernhard Kornberger, Graz/Austria. All rights reserved.
//
// This file is part of the Fade2D library. You can use it for your personal
// non-commercial research. Licensees holding a commercial license may use this
// file in accordance with the Commercial License Agreement provided
// with the Software.
//
// This software is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
// THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Please contact the author if any conditions of this licensing are not clear
// to you.
//
// Author: Bernhard Kornberger, bkorn (at) geom.at
// http://www.geom.at


#pragma once


#include "Point2.h"
#include "Circle2.h"
#include "Segment2.h"
#include "Color.h"
#include "Label.h"
#include "Bbox2.h"


#include "common.h"
#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif


/** \brief Postscript writer
*
* \image html visualizer.jpg "Figure 1: Example output of the Visualizer"
* \image latex visualizer.eps "Example output of the Visualizer" width=12cm
*
* Visualizer2 is a generally usable postscript writer. It supports Point2, Segment2, Circle2, Label and Color
*/
class Visualizer2
{
public:


/** \brief Constructor
*
* @param filename_ is the name of the postscript file to be written
*/
	CLASS_DECLSPEC
	Visualizer2(const std::string filename_);

	CLASS_DECLSPEC
	~Visualizer2();


/** \brief Add a Segment2 object to the visualization
*/
	CLASS_DECLSPEC
	void addObject(const Segment2& seg,const Color& c);

/** \brief Add a vector of Segment2 objects to the visualization
*/
	CLASS_DECLSPEC
	void addObject(const std::vector<Segment2>& vSegments,const Color& c);

/** \brief Add a vector of Triangle2 objects to the visualization
*/
	CLASS_DECLSPEC
	void addObject(const std::vector<Triangle2>& vT,const Color& c);

/** \brief Add a Circle2 object to the visualization
*/
	CLASS_DECLSPEC
	void addObject(const Circle2& circ,const Color& c);
/** \brief Add a Point2 object to the visualization
*/
	CLASS_DECLSPEC
	void addObject(const Point2& pnt,const Color& c);
/** \brief Add a Triangle2 object to the visualization
*/
	CLASS_DECLSPEC
	void addObject(const Triangle2& tri,const Color& c);

/** \brief Add a Triangle2* vector to the visualization
*/
	CLASS_DECLSPEC
	void addObject(const std::vector<Triangle2*> vT,const Color& c);

/** \brief Add a Label object to the visualization
*/
	CLASS_DECLSPEC
	void addObject(const Label& lab,const Color& c);

/** \brief Add a header line to the visualization
*/
	CLASS_DECLSPEC
	void addHeaderLine(const std::string& s);


/** \brief Finish and write the postscript file
*
* @note This method \e must be called at the end when all the objects have been added.
*/
	CLASS_DECLSPEC
	void writeFile();

protected:
	std::ofstream outFile;
	std::vector<std::pair<Segment2,Color> > vSegments;
	std::vector<std::pair<Circle2,Color> > vCircles;
	std::vector<std::pair<Point2,Color> > vPoints;
	std::vector<std::pair<Triangle2,Color> > vTriangles;
	std::vector<std::pair<Label,Color> > vLabels;
	void writeHeaderLines();

	int updateCtr;
	Bbox2 bbox;
	bool bFill;

	Point2 scaledPoint(const Point2 &p);
	double scaledDouble(const double &d);
	void changeColor(float r,float g,float b,float linewidth,bool bFill);
	void changeColor(const Color& c);
	void writeHeader(std::string title);
	void writeFooter();
	void writeLabel(Label l);
	void writeLine(const Point2& pSource,const Point2& pTarget);
	void writeTriangle(const Triangle2* pT,bool bFill_);
	void writePoint(Point2& p1_,float size);
	void writeCircle(const Point2& p1_,double radius);
	void periodicStroke();
	void setRange();
	Color lastColor;
	std::string filename;
	std::vector<std::string> vHeaderLines;
};


} // (namespace)
