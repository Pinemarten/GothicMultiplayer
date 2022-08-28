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
#include "zstring.hpp"

class oCNpc;
class oCItem;
class oCItemContainer;

class oCNpcInventory
{
public:
	oCItemContainer* inv;
	oCNpc*			owner;
	zBOOL			packAbility;
	//zCListSort<oCItem> {
	int				inventory_Compare;
	int				inventory_data;
	int				inventory_next;
	//}
	zSTRING			packString;
	int				maxSlots;

public:
	zINT IsOpen(void){XCALL(0x00709200);};
	int HandleEvent(int pA) { XCALL(0x0070E040) };
	int IsEmpty(int pA, int pB) { XCALL(0x0070D1A0) };
	oCItem* GetItem(int pA) { XCALL(0x0070C450) };
	oCItem* Insert(oCItem* pA) { XCALL(0x0070C730) };
	oCItem* IsIn(oCItem* pA, int pB) { XCALL(0x0070CA80) };
	oCItem* IsIn(zSTRING const& pA, int pB) { XCALL(0x0070CAC0) };
	oCItem* IsIn(int item_instance,int amount) { XCALL(0x0070CAF0) };
	oCItem* Remove(oCItem* pA, int pB) { XCALL(0x0070CE20) };
	oCItem* Remove(int pA,int pB) { XCALL(0x0070D080) };
	oCItem* Remove(zSTRING const& pA, int pB) { XCALL(0x0070D170) };
	oCItem* RemoveByPtr(oCItem* pA, int pB) { XCALL(0x0070CC70) };
	oCNpc* GetOwner() { XCALL(0x0070C330) };
	int GetSelectedItemNumber(){return *(int*)((DWORD)this+0x24);}
	zCView* GetInventoryWindow(){return *(zCView**)((DWORD)this+0x78);}
	int CanCarry(oCItem* pA) { XCALL(0x0070F4D0) };
	void CheckForEquippedItems(int pA) { XCALL(0x0070F390) };
	void ClearInventory() { XCALL(0x0070BDB0) };
	int GetAmount(int item_instance) { XCALL(0x0070C970) };
	int GetCategory(oCItem* pA) { XCALL(0x0070C690) };
	int GetNumItemsInCategory() { XCALL(0x0070C340) };
	int HandleTrade(int pA) { XCALL(0x0070DE00) };
	void PackAllItems(int pA) { XCALL(0x00710020) };
	void PackItemsInCategory(int pA) { XCALL(0x00710040) };
	void SetPackAbility(int pA) { XCALL(0x0070F600) };
	void SetOwner(class oCNpc *) { XCALL(0x000070C320) };
	void UnpackAllItems() { XCALL(0x00710030) };
	void UnpackCategory() { XCALL(0x0070F620) };
	void UnpackItemsInCategory() { XCALL(0x00710A20) };
	oCNpcInventory() { XCALL(0x0070BAF0) };
	~oCNpcInventory() { XCALL(0x0070BC00) };
	void Close() { XCALL(0x0070C2F0) };
	void Open(int pA, int pB, int pC) { XCALL(0x0070BF10) };
	void Remove(oCItem* pA) { XCALL(0x0070CBE0) };
	oCItemContainer* GetItemContainer() { return this->inv;};
	oCItem* GetSelectedItem(void){XCALL(0x007092C0);};
	static bool IsHeroInventoryOpened() { return *(bool*)0x009A43F0;};
};
