/*/////////////////////////////////////////////////////////////////////////////
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
#ifndef __OCVIEWPROGRESSBAR_H_INCLUDED__
#define __OCVIEWPROGRESSBAR_H_INCLUDED__

#include "ztypes.h"
#include "macros.h"

class oCViewProgressBar : public zCView
{
private:
	char _data[0x200];
public:
	void Draw(void){XCALL(0x0046F300);};
	void HandleChange(void){XCALL(0x0046F190);};
	void Init(void){XCALL(0x0046ED40);};
	void ResetRange(void){XCALL(0x0046ED40);};
	void ResetRange(int pA, zSTRING & pB){XCALL(0x0046EEC0);};
	void ResetRange(int pA, int pB){XCALL(0x0046F340);};
	oCViewProgressBar(int x1, int y1, int x2, int y2, zTviewID ItemID = VIEW_ITEM){XCALL(0x0046E9D0);};
};
#endif  //__OCVIEWPROGRESSBAR_H_INCLUDED__