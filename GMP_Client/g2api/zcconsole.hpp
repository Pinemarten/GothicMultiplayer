/*////////////////////////////////////////////////////////////////////////////
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
#ifndef __API_G2_ZCCONSOLE_H__
#define __API_G2_ZCCONSOLE_H__
#include "ztypes.h"
#include "macros.h"
#include "zcinputcallback.h"

class zCWorld;
class zCParser;

class zCConsole : public zCInputCallback
{
private:
	zSTRING unk;
	zSTRING unk2;
	zSTRING result;
	char m_datab[256]; // TODO: Get size
public:
	zCWorld* GetWorld(){XCALL(0x007823E0);};
	zCParser* GetParser(void){XCALL(0x00782400);};
	int IsVisible(void){XCALL(0x00783890);};
	void Toggle(void){XCALL(0x007837A0);};
	void Hide(void){XCALL(0x007836B0);};
	void Show(void){XCALL(0x00783460);};
	int	Evaluate(const zSTRING& in){XCALL(0x00784860);};
	void Eval(const zSTRING& in){XCALL(0x007844B0);};
	zSTRING& GetResult(void) { return this->result; };
	void Update(void) { XCALL(0x007838E0); };
	void AddEvalFunc(int (__cdecl *)(zSTRING& zCommand, zSTRING& zReturn)) { XCALL(0x00784F80); };
	int AutoCompletion(zSTRING& str){XCALL(0x00782C00);};
	void Register(const zSTRING& zCommand, const zSTRING& zDesc) { XCALL(0x00782AE0); };
	inline static zCConsole* GetConsole() { return (zCConsole*)0x00AB3860; };
};
#endif // __API_G2_ZCCONSOLE_H__