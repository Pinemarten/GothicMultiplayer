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
//
///////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "ztypes.hpp"
#include "macros.hpp"

class zCModelNodeInst;

class zCModelAni{
public:
	zSTRING& GetAniName(void){XCALL(0x0059D160);};
	int GetAniID(void){XCALL(0x006A9800);};
};

class zCModelAniActive
{
public:
	zCModelAni* GetCurrentAni()
	{
		return *(zCModelAni**)(this);
	}
	zCModelAni* GetCurrentAni1()
	{
		return *(zCModelAni**)(this+0x04);
	}
	zCModelAni* GetUnusualAnim()
	{
		return *(zCModelAni**)(this+0x88);
	}
	float GetProgressPercent(void){XCALL(0x00576C60);};
	void SetProgressPercent(float percent){XCALL(0x00576CA0);};
};

class zCModelTexAniState
{
public:
	int GetSkinTexture()
	{
		return *(int*)(this+0x08);
	}
	void SetSkinTexture(int tex)
	{
		*(int*)(this+0x08) = tex;
	}
};

class zCDontKnow
{
public:
	zCModelTexAniState* GetModelTexAniState()
	{
		return *(zCModelTexAniState**)(this);
	}
};

class zCModel
{
public:
	void AttachChildVobToNode(zCVob*, zCModelNodeInst*){XCALL(0x005806A0);};
	int ApplyModelProtoOverlay(const zSTRING & mdsname){XCALL(0x005787F0);};
	int HasAppliedModelProtoOverlay(const zSTRING & mdsname){XCALL(0x00578AC0);};
	void RemoveModelProtoOverlay(const zSTRING & mdsname){XCALL(0x00578C40);};
	zCModelNodeInst* SearchNode(const zSTRING & NodeName){XCALL(0x0057DFF0);};
	void ShowAniList(int){XCALL(0x00582770);};
	zSTRING GetVisualName(void){XCALL(0x0057DF60);};
	void StartAni(int a, int b){XCALL(0x0057B070);};
	void StartAni(const zSTRING& ani_string, int pB){XCALL(0x0057AF70);};
	void StartAni(class zCModelAni *, int pB){XCALL(0x0057AF70);};
	void StopAni(int ani_instance){XCALL(0x0057ABE0);};
	void StartAnimation(const zSTRING & ani_string){XCALL(0x005765E0);};
	void StopAnimation(const zSTRING & ani_string){XCALL(0x005765F0);};
	int IsAnimationActive(const zSTRING & ani_string){XCALL(0x00576690);};
	int IsAniActive(zCModelAni*){XCALL(0x00472F90);};
	int GetAniIDFromAniName(const zSTRING& ani_string){XCALL(0x00612070);};
	zCVob* GetAttachedNodeVob(zCModelNodeInst*){XCALL(0x00580870);};
	zMAT4 GetTrafoNodeToModel(zCModelNodeInst*){XCALL(0x0057A9C0);};
	void FadeOutAni(int pA){XCALL(0x0057EF50);};
	void FadeOutAnisLayerRange(int a, int b){XCALL(0x0057F1F0);};
	void StopAnisLayerRange(int pA, int pB)		{ XCALL(0x0057F240); };
	void UpdateAttachedVobs(void){XCALL(0x00580900);};
	zCModelAni *GetAniFromAniID(int id)			{ XCALL(0x00472F50); };
	zCModelAniActive* GetModelAniActive()
	{
		return *(zCModelAniActive**)(this+0x38);
	}
	zCDontKnow* GetDontKnow()
	{
		return *(zCDontKnow**)(this+0xBC);
	}
};




