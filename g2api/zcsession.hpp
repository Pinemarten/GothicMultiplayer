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
#include "zcinputcallback.hpp"

class zCCSManager;
class zCWorld;
class zCCamera;
class zCAICamera;
class zCVob;
class zCView;

class zCSession : public zCInputCallback
{
	zCCSManager* 	csMan;
	zCWorld*   		world;
	zCCamera*  		camera;   
	zCAICamera* 	aiCam;
	zCVob* 			camVob;   
	zCView*  		viewport;
public:
	//.text:005DEC80 ; public: virtual void __thiscall zCSession::CamInit(class zCVob *, class zCCamera *)
	/** Insert description. 
	* @usable Ingame only
	*/
	void CamInit(zCVob *, zCCamera *)
	{
		XCALL(0x005DEC80);
	};
	//.text:005DEA80 ; public: virtual void __thiscall zCSession::CamInit(void)
	/** Insert description. 
	* @usable Ingame only
	*/
	void CamInitOrginal(void)
	{
		XCALL(0x005DEA80);
	};
};
