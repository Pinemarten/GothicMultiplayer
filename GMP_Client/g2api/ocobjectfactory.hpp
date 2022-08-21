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

#include "ocnpc.hpp"
#include "ocitem.hpp"
#include "zcworld.hpp"
#include "zfile.hpp"

class zCObjectFactory
{
public:
	zFILE_VDFS* CreateZFile(const zSTRING&){XCALL(0x005AB940);};
};

class oCObjectFactory : public zCObjectFactory
{
public:
	oCItem* CreateItem(zINT instance){XCALL(0x0076FDE0);};
	oCNpc* CreateNpc(zINT instance){XCALL(0x0076FD20);};
	zCWorld* CreateWorld(void){XCALL(0x0076FCA0);};
	inline static oCObjectFactory* GetFactory(void) { return *(oCObjectFactory**)0x008D8DF0; };
};
