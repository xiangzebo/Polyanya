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
#include "common.h"


#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif

class Vector2;

/** \brief Vector
*
* This class represents a vector in 2D
*/
class Vector2
{
public:

#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Constructor
*
*/
	CLASS_DECLSPEC
	Vector2(const double x_,const double y_,const double z_):
		valX(x_),
		valY(y_),
		valZ(z_)
	{
	}
/** \brief Default constructor
*
* The vector is initialized to (0,0,0)
*/
	CLASS_DECLSPEC
	Vector2():valX(0),valY(0),valZ(0)
	{
	}
/** \brief Copy constructor
*
* Create a copy of vector v_
*/
	CLASS_DECLSPEC
	Vector2(const Vector2& v_):
		valX(v_.x()),
		valY(v_.y()),
		valZ(v_.z())
	{
	}

#else

/** \brief Constructor
*
*/
	CLASS_DECLSPEC
	Vector2(const double x_,const double y_):
		valX(x_),
		valY(y_)
	{
	}
/** \brief Default constructor
*
* The vector is initialized to (0,0)
*/
	CLASS_DECLSPEC
	Vector2():valX(0),valY(0)
	{
	}
/** \brief Copy constructor
*
* Create a copy of vector v_
*/
	CLASS_DECLSPEC
	Vector2(const Vector2& v_):
		valX(v_.x()),
		valY(v_.y())
	{
	}
#endif

	CLASS_DECLSPEC
	~Vector2()
	{
	}

/** \brief Get the x-value
*/
	CLASS_DECLSPEC
	double x() const
	{
		return valX;
	}
/** \brief Get the y-value
*/
	CLASS_DECLSPEC
	double y() const
	{
		return valY;
	}

#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Get the z-value.
*/
	CLASS_DECLSPEC
	double z() const
	{
		return valZ;
	}
#endif

#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Set the values
*/
	void set(const double x_,const double y_,const double z_)
	{
		valX=x_;
		valY=y_;
		valZ=z_;
	}
#else
/** \brief Set the values
*/
	void set(const double x_,const double y_)
	{
		valX=x_;
		valY=y_;
	}
#endif


/** \brief Get the length of the vector
*/
CLASS_DECLSPEC
double length() const
{
#if GEOM_PSEUDO3D==GEOM_TRUE
	double x2(x()*x());
	double y2(y()*y());
	double z2(z()*z());
	return sqrt(x2+y2+z2);

#else
	double x2(x()*x());
	double y2(y()*y());
	return sqrt(x2+y2);
#endif
}

/** \brief Multiply two vectors
*/

#if GEOM_PSEUDO3D==GEOM_TRUE
CLASS_DECLSPEC
double operator*(const Vector2& other) const
{
	return x()*other.x()+y()*other.y()+z()*other.z();
}
#else
CLASS_DECLSPEC
double operator*(const Vector2& other) const
{
	return x()*other.x()+y()*other.y();
}
#endif


/** \brief Multiply by a scalar value
*/
#if GEOM_PSEUDO3D==GEOM_TRUE
CLASS_DECLSPEC
Vector2 operator*(double val) const
{
	double prodX(x()*val);
	double prodY(y()*val);
	double prodZ(z()*val);
	return Vector2(prodX,prodY,prodZ);
}
#else
CLASS_DECLSPEC
Vector2 operator*(double val) const
{
	double prodX(x()*val);
	double prodY(y()*val);
	return Vector2(prodX,prodY);

}
#endif

/** \brief Divide by a scalar value
*/
#if GEOM_PSEUDO3D==GEOM_TRUE
CLASS_DECLSPEC
Vector2 operator/(double val) const
{
	double qX(x()/val);
	double qY(y()/val);
	double qZ(z()/val);
	return Vector2(qX,qY,qZ);
}
#else
CLASS_DECLSPEC
Vector2 operator/(double val) const
{
	double qX(x()/val);
	double qY(y()/val);
	return Vector2(qX,qY);

}
#endif


protected:
	double valX;
	double valY;
#if GEOM_PSEUDO3D==GEOM_TRUE
	double valZ;
#endif
};

CLASS_DECLSPEC
inline std::ostream &operator<<(std::ostream &stream, const Vector2& vec)
{
#if GEOM_PSEUDO3D==GEOM_TRUE
	stream << "Vector2: "<<vec.x()<<", "<<vec.y()<<", "<<vec.z();
#else
	stream << "Vector2: "<<vec.x()<<", "<<vec.y();
#endif
	return stream;
}

#if GEOM_PSEUDO3D==GEOM_TRUE
/** \brief Cross product
*/
CLASS_DECLSPEC
inline Vector2 crossProduct(const Vector2& vec0,const Vector2& vec1)
{

	double x=vec0.y() * vec1.z() - vec0.z() * vec1.y();
	double y=vec0.z() * vec1.x() - vec0.x() * vec1.z();
	double z=vec0.x() * vec1.y() - vec0.y() * vec1.x();
	return Vector2(x,y,z);
}
#endif

/** \brief Normalize the vector
*/
CLASS_DECLSPEC
inline Vector2 normalize(const Vector2& other)
{
	double len(other.length());
#if GEOM_PSEUDO3D==GEOM_TRUE
	if(len>0)
	{
		return Vector2(other.x()/len,other.y()/len,other.z()/len);
	}
	else
	{
		std::cout<<"warning: normalize(const Vector2& other), Null length vector!"<<std::endl;
		return Vector2(0,0,0);
	}
#else
	if(len>0)
	{

		return Vector2(other.x()/len,other.y()/len);
	}
	else
	{
		std::cout<<"warning: normalize(const Vector2& other), Null length vector!"<<std::endl;
		return Vector2(0,0);
	}
#endif
}



} // (namespace)
