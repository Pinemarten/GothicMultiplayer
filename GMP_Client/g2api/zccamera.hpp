/*////////////////////////////////////////////////////////////////////////////

This file is part of the G2Ext SDK headers.

//////////////////////////////////////////////////////////////////////////////

The G2Ext SDK headers

Copyright © 2009, 2010 by Paindevs and Patrick Vogel

All Rights reserved.

THE WORK (AS DEFINED BELOW) IS PROVIDED
UNDER THE TERMS OF THIS CREATIVE COMMONS
PUBLIC LICENSE ("CCPL" OR "LICENSE").
THE WORK IS PROTECTED BY COPYRIGHT AND/OR
OTHER APPLICABLE LAW. ANY USE OF THE WORK
OTHER THAN AS AUTHORIZED UNDER THIS LICENSE
OR COPYRIGHT LAW IS PROHIBITED.

BY EXERCISING ANY RIGHTS TO THE WORK PROVIDED
HERE, YOU ACCEPT AND AGREE TO BE BOUND BY THE
TERMS OF THIS LICENSE. TO THE EXTENT THIS
LICENSE MAY BE CONSIDERED TO BE A CONTRACT,
THE LICENSOR GRANTS YOU THE RIGHTS CONTAINED
HERE IN CONSIDERATION OF YOUR ACCEPTANCE OF
SUCH TERMS AND CONDITIONS.

Full license at http://creativecommons.org/licenses/by-nc/3.0/legalcode

// Modified by Gothic Multiplayer Team (pampi, skejt23, mecio)

/////////////////////////////////////////////////////////////////////////////*/

#pragma once
#ifndef __API_G2_ZCCAMERA_H__
#define __API_G2_ZCCAMERA_H__
#include "ztypes.hpp"
#include "macros.hpp"

class zCCamera
{	
public:
	char data_chunk_1[164];
	zMAT4 cameraMatrix;
	char data_chunk_2[2108];
	zCVob* camVob;
	char data_chunk_3[16];
public:
	void Activate(void){XCALL(0x0054A700);};
	void AddTremor(const zVEC3& pos, float radius, float time, const zVEC3& amplitude) 
	{
		XCALL(0x0054B660);
	};
	void SetFarClipZ(float z)
	{
		XCALL(0x0054B200);
	};
	void StopTremor(zVEC3& pos)
	{
		XCALL(0x0054B960);
	};
	void SetRenderScreenFade(zCOLOR col)
	{
		XCALL(0x0054BBC0);
	};
	void SetRenderCinemaScope(zCOLOR col)
	{
		XCALL(0x0054BC20);
	};
	void StopTremor(void)
	{
		XCALL(0x0054B650);
	};
	static zMAT4* GetMatrix(void)
	{
		return (zMAT4*)((int)(*(zCCamera**)0x008D7F94)+0x148);
	};
	zCVob* GetCameraVob()
	{
		return *(zCVob**)((DWORD)this+0x920);
	};
	static inline zCCamera* GetCamera(void)
	{
		return *(zCCamera**)0x008D7F94;
	};
};
#endif  //__API_G2_ZCCAMERA_H__