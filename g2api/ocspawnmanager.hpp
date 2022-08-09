
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
#include "zcview.hpp"
#include "zcobject.hpp"

class oCSpawnManager
{
public:
	oCSpawnManager(){XCALL(0x00777730);};
	static void SetInsertRange(float p1){XCALL(0x00777820);};
	static float GetInsertRange(void){XCALL(0x00777830);};
	static void SetRemoveRange(float p1){XCALL(0x00777840);};
	static float GetRemoveRange(void){XCALL(0x00777850);};
	static void SetSpawnTime(float p1){XCALL(0x00777860);};
	static float GetSpawnTime(void){XCALL(0x00777870);};
	void ClearList(void){XCALL(0x00777880);};
	int IsSpawningEnabled(void){XCALL(0x00777A20);};
	void SetSpawningEnabled(int enabled){XCALL(0x00777A30);};
	void SpawnImmediately(int p1){XCALL(0x00777A40);};
	void CheckInsertNpcs(void){XCALL(0x00777BE0);};
	void ShowDebugInfo(void){XCALL(0x00777C70);};
	void CheckInsertNpc(void){XCALL(0x007780B0);};
	void InitCameraPos(void){XCALL(0x007788C0);};
	int InsertNpc(class oCNpc *, const zVEC3 & pA){XCALL(0x00778920);};
	oCNpc* SummonNpc(int instance, const zVEC3 & position, float p2){XCALL(0x00778A20);};
	oCNpc* SpawnNpc(int p1, zSTRING & pA, float p2){XCALL(0x00778B20);};
	void SpawnNpc(class oCNpc *, zSTRING & pA, float p1){XCALL(0x00778BA0);};
	void SpawnNpc(class oCNpc *, const zVEC3 & pA, float p1){XCALL(0x00778E70);};
	void CheckForInvalidDialogCamPos(float p1, class oCNpc *){XCALL(0x00778FD0);};
	int UseDeadNpcRoutinePos(class oCNpc *){XCALL(0x007791B0);};
	int CanDeadNpcBeRemoved(class oCNpc *){XCALL(0x00779260);};
	int CheckRemoveNpc(class oCNpc *){XCALL(0x007792E0);};
	void DeleteNpc(class oCNpc *){XCALL(0x00779690);};
	void DeleteAllSummoned(void){XCALL(0x00779720);};
	void Archive(zCArchiver & pA){XCALL(0x007797F0);};
	void Unarchive(zCArchiver & pA){XCALL(0x00779890);};
};