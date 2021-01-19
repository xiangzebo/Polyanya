
// (c) 2010 Geom e.U. Bernhard Kornberger, Graz/Austria. All rights reserved.
//
// This file is part of the Fade2D library. You can use it for your personal
// non-commercial, non-military research. Redistribution with your software
// is allowed if you clearly point out that your software is linked with 
// Fade and that any commercial use requires a commercial Fade license.

// This software is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
// THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Please contact the author if any conditions of this licensing are not clear 
// to you.
// 
// Author: Bernhard Kornberger, bkorn (at) geom.at 
//         C++ Freelancer
// http://www.geom.at/products/fade2d/

#pragma once 
#include "Fade_2D.h"
 
namespace{
#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_LIC=GEOM_FADE25D;
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_LIC=GEOM_FADE2D;
#endif

struct License
{
	License()
	{
	GEOM_LIC::setLic(
		"[NonCommercialResearch]",
		"[LicType,student],[2D,1e6],[25D,5e4],[MeshGen,5e4],[SegCheck,5e4]",
		"[LF:F/C]",
		"5303992d",
		"6e923f87");
	}
};
License lic;
}
