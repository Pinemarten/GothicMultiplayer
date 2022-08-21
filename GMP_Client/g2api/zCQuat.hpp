/*****************************************************************************
**																			**
**						   Gothic Multiplayer Team.							**
**																		   	**
**				   Copyright (C) 2012 - All Rights Reserved				   	**
**																			**
******************************************************************************
**																			**
**	PROJECT:		Gothic Multiplayer										**
**																			**
**	FILE:			ZenGin\Types\zCQuat.h					   				**
**																			**
**	CREATED:		27/05/12												**
**																			**
**	DEVELOPERS:		skejt23(skejt23@gmail.com)								**
**					Pampi													**
**																			**
*****************************************************************************/

#pragma warning (disable : 4244)

#ifndef _ZCQUAT_H
#define _ZCQUAT_H

#include <math.h>
#include "ztypes.hpp"

// Gothic Quaternion Class
class zCQuat
{
public:
	union {
		struct {
			zVEC3 v; 
			float s; 
		};
		struct { float element[4]; };
	};
public:
	zCQuat(){};
	zCQuat(float real, float x, float y, float z): s(real), v(x,y,z) {}
	zCQuat(float real, const zVEC3 & i): s(real), v(i) {}

	void Matrix4ToQuat(const zMAT4 & matrix){XCALL(0x00518560);};
	void QuatToMatrix4(const zMAT4 & matrix){XCALL(0x00518360);};
};

#endif
