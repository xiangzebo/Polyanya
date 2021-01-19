#pragma once
#include <string>


#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif

enum MsgType
{
	MSG_PROGRESS,
	MSG_WARNING
};

/**  \brief %MsgBase
*
* MsgBase is a base class from which message subscriber classes (for
* example widgets, progress bars, ...) can be derived which then
* receive messages (progress, warnings, ...) from Fade.
*
*/
class CLASS_DECLSPEC MsgBase
{
public:
	MsgBase(){};
	virtual ~MsgBase(){}


/** \brief update
*
* This method must be defined in derived message subscriber classes. It
* is automatically called everytime Fade has a message of type \e msgType.
*
*/
	virtual void update(MsgType msgType,const std::string& s,double d)=0;
};


} // Namespace

