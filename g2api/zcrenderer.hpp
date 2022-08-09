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
#include "ztypes.hpp"
#include "macros.hpp"
#include "dx7sdk/inc/d3d.h"
#include "dx7sdk/inc/ddraw.h"
#include "dx7sdk/inc/dinput.h"


class zCVob;
class oCWorld;
class zCCamera;
struct zTRenderContext
{
	zCVob* Vob;
	oCWorld* World;
	zCCamera* Cam;
};

class zCRnd_D3D
{
public:
	int GetScreenMode(void){XCALL(0x00658FB0);};
};

/** Gothic's Direct3D 7 renderer class. */
class zCRenderer : public zCRnd_D3D
{
private:
	char m_data[1024];
public:
	/** This method returns the current DirectDraw instance*/
	inline static IDirectDraw7* GetDirectDraw(void) { return *(IDirectDraw7**)0x009FC9EC; };
	/** This method returns the current Direct3D7 instance*/
	inline static IDirect3D7* GetDirect3D(void) { return *(IDirect3D7**)0x009FC9F0; };
	/** This method returns the current Direct3D device instance*/
	inline static IDirect3DDevice7* GetDirect3DDevice(void) { return *(IDirect3DDevice7**)0x009FC9F4; };
	/** This method returns the current Direct Input device instance*/
	inline static IDirectInputDevice7A* GetDirectInputDevice(void) { return *(IDirectInputDevice7A**)0x008D1D64; };
	/** This method returns the current zCRenderer instance*/
	inline static zCRenderer* GetRenderer()	{ return *(zCRenderer**)0x00982F08; };
};
