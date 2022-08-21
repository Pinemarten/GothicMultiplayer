
/*
MIT License

Copyright (c) 2022 Gothic Multiplayer Team (pampi, skejt23, mecio)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "ztypes.hpp"
#include "macros.hpp"
#include "zcworld.hpp"

class oCWorld : public zCWorld
{
public:
	~oCWorld(){XCALL(0x0077EFE0);};
	void LoadWorld(const zSTRING& name, zTWorldLoadMode){ XCALL(0x0077FB40); };
	void DisposeVobs(void) { XCALL(0x00780BD0); };
	void DisposeWorld(void) { XCALL(0x00780D50); };
	void RemoveVob(zCVob*) { XCALL(0x007800C0); };
	void DisableVob(zCVob*) { XCALL(0x00780460); };
	zCVob* SearchVobByName(const zSTRING& name){XCALL(0x00780610);};
	static void DisableAllItemsInWorld()
	{
		zCListSort<oCItem>* ItemList = oCGame::GetGame()->GetWorld()->GetItemList();
		int size = ItemList->GetSize();
		oCWorld* World = oCGame::GetGame()->GetGameWorld();
		for(int i=0; i<size; i++){
			ItemList = ItemList->GetNext();
			oCItem* ItemInList = ItemList->GetData();
			World->DisableVob(ItemInList);
		}
	}
};