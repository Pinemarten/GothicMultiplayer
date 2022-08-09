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

class zCSoundFX
{
public:
	void SetLooping(int){XCALL(0x004F6610)};
};

class zCSndSys_MSS
{
public:
	int PlaySound(zCSoundFX*, int){XCALL(0x004EF7B0)};
	int PlaySound3D(const zSTRING & SoundName, zCVob* Obj, int, int = 0){XCALL(0x004F1060)};
	int PlaySound3D(zCSoundFX* , zCVob* Obj, int, int = 0){XCALL(0x004F10F0)};
	void StopAllSounds(void){XCALL(0x004F23C0)};
};

class zCSoundSystem : public zCSndSys_MSS
{
public:
	inline static zCSoundSystem* GetSoundSystem() { return *(zCSoundSystem**)0x0099B03C; };
	zCSoundFX* LoadSoundFX(const zSTRING & SoundName){XCALL(0x004ED960)};
};