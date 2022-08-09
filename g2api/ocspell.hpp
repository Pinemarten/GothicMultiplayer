//////////////////////////////////////////////////////////////////////////////
// 
// The G2Ext SDK headers are being published under the 
// Mozilla Public License Version 1.1 (the "License"); you may not use this 
// file except in compliance with the License.
// You may obtain a copy of the License at http://www.mozilla.org/MPL/
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
// 
// Developed by Christoffer Anselm and Patrick Vogel
// All rights reserved.
//
// G2Ext SDK headers © 2009 by Paindevs and Patrick Vogel
//
// Modified by Gothic Multiplayer Team (pampi, skejt23, mecio)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __API_G2_OCSPELL_H__
#define __API_G2_OCSPELL_H__

#include "ztypes.hpp"

class oCSpell
{
private:
	char m_data[1024];
public:
	int Cast(void){XCALL(0x00485360)};
	int CastSpecificSpell(void){XCALL(0x00486960)};
	void Open(void){XCALL(0x004858A0)};
	void Close(void){XCALL(0x004859A0)};
	void Invest(void){XCALL(0x004850D0)};
	void SetEnabled(int){XCALL(0x004847F0)};
	int GetEnabled(void){XCALL(0x00484920)};
	int GetSpellStatus(void){XCALL(0x00486660)};
	oCNpc* GetCaster(void){return *(oCNpc**)((DWORD)this+0x34);}
	oCNpc* GetTarget(void){return *(oCNpc**)((DWORD)this+0x38);}
	zCVob* GetVob(void){XCALL(0x00486470)};
	int GetSpellID(void){XCALL(0x00486480)};
	zSTRING GetName(void){XCALL(0x004864B0)};
	zSTRING GetSpellInstanceName(int){XCALL(0x00484150)};
	void InitValues(int SpellId){XCALL(0x00484020)};
	void SetReleaseStatus(void){XCALL(0x00486670)};
	void SetSpellInfo(int){XCALL(0x00486940)};
	void SetSpellStatus(int status){*(int*)((DWORD)this+0x50) = status;}
	void Setup(zCVob* Caster, zCVob* Target, int){XCALL(0x00484930)};
	static oCSpell* _CreateNewInstance(void)
	{
		XCALL(0x00487550);
	}
};

class oCMag_Book
{
private:
	char _data[64];
public:
	oCSpell* GetSelectedSpell(void){XCALL(0x00477780);};
	int GetSelectedSpellNr(void){XCALL(0x004777D0);};
	oCSpell* GetSpell(int){XCALL(0x00479BC0);};
	int Register(oCSpell*, int){XCALL(0x00475AD0);};
	void Spell_Cast(void){XCALL(0x004767A0);};
	void Spell_Open(int){XCALL(0x004777E0);};
	void Spell_Setup(oCNpc*, zCVob*, int){XCALL(0x004763A0);};
	void Open(int){XCALL(0x00476F60);};
	int DoOpen(void){XCALL(0x00478460);};
	void Close(int){XCALL(0x00477270);};
	int IsIn(int){XCALL(0x00476220);};
	int GetNoOfSpells(void){XCALL(0x00479B60);};
	oCItem* GetSpellItem(int){XCALL(0x00479BE0);};
	oCItem* GetSpellItem(oCSpell*){XCALL(0x00479C00);};
	oCSpell* GetSpell(oCItem* pA, int & pB){XCALL(0x00479B70);};
	oCSpell* GetSpellByKey(int){XCALL(0x00479C60);};
	int GetKeyByItem(oCItem*){XCALL(0x00479D30);};
	void KillSelectedSpell(void){XCALL(0x00477A90);};
	void StopSelectedSpell(void){XCALL(0x00477910);};
	oCSpell* CreateNewSpell(int){XCALL(0x00475E50);};
	void StartCastEffect(zCVob* pA, const zVEC3& pB){XCALL(0x00476E60)};
	int SetFrontSpell(int){XCALL(0x004789C0)};
};

#endif //__API_G2_OCSPELL_H__