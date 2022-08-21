
/*
MIT License

Copyright (c) 2022 Gothic Multiplayer Team.

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

#include "ocgame.hpp"
#include "ocnpc.hpp"
#include "zcview.hpp"
#include "zcworld.hpp"
#include "zCCsPlayer.h"
#include "zcvob.hpp"
#include "ocmob.hpp"
#include "zstring.hpp"
#include "zccamera.hpp"
#include "zcoption.hpp"
#include "ocviewstatusbar.hpp"
#include "InjectMage.h"
#include "utils.h"

#define gLogStatistics *(int*)(0x008C2B50)
#define gStatusReadGameOptions *(int*)(0x008B21D4)
#define zOPT_SEC_OPTIONS *(zSTRING*)(0x008CD6E0)
#define show_Focus *(int*)(0x008B21D8)
#define show_FocusItm *(int*)(0x008B21DC)
#define show_FocusMob *(int*)(0x008B21E0)
#define show_FocusNpc *(int*)(0x008B21E4)
#define show_FocusBar *(int*)(0x008B21E8)
#define s_bTargetLocked *(int*)(0x00AB2680)


void oCGame::InstallPatches()
{
	void* ptr = DisMember(sizeof(void (oCGame::*)()),  &oCGame::UpdatePlayerStatus);
	CallPatch(0x006C89D8, (DWORD)ptr, 0);
};

void oCGame::UpdatePlayerStatus()
{
	if(!oCNpc::GetHero()) return;
	zCView* screen = zCView::GetScreen();
	oCNpc* npc = oCNpc::GetHero();
	screen->RemoveItem(hpBar);
	screen->RemoveItem(swimBar);
	screen->RemoveItem(manaBar);
	screen->RemoveItem(focusBar);
	if(!showPlayerStatus) return;
	zCWorld* wrld = GetWorld();
	if(wrld){
		if(wrld->GetCutscenePlayer()->GetPlayingGlobalCutscene()) return;
	}
	int cha = 0;
	if(npc->GetInventory()->IsOpen())
	{
		oCItem* SelectedItem = oCNpc::GetHero()->GetInventory()->inv->GetSelectedItem();
		if(SelectedItem){
			int nut = SelectedItem->nutrition;
			if(!nut){
				int change = SelectedItem->change_atr[0];
				int ch = 3;
				do
				{
					if(!(change-12))
						nut+=change;
					change+=4;
					--ch;
				}
				while(ch);
				change = SelectedItem->change_atr[0];
				ch = 3;
				do
				{
					if((change-12) == 2)
						cha += change;
					change += 4;
					--ch;
				}
				while(ch);
			}
		}
	}
	screen->InsertItem(hpBar);
	hpBar->SetMaxRange(0, oCNpc::GetHero()->GetAttribute(NPC_ATR_HITPOINTS_MAX));
	hpBar->SetRange(0, oCNpc::GetHero()->GetAttribute(NPC_ATR_HITPOINTS_MAX));
	hpBar->SetPreview(oCNpc::GetHero()->GetAttribute(NPC_ATR_HITPOINTS));
	hpBar->SetValue(oCNpc::GetHero()->GetAttribute(NPC_ATR_HITPOINTS));
	if(oCNpc::GetHero()->GetAnictrl()){
		if(oCNpc::GetHero()->GetAnictrl()->wmode_last == 5){
			float timea, timeb, timec;
			oCNpc::GetHero()->GetSwimDiveTime(timea, timeb, timec);
			screen->InsertItem(swimBar);
			float da = timeb;
			if(timeb == -1000000.0f)
				timec = da;
			swimBar->SetMaxRange(0, da);
			swimBar->SetRange(0, da);
			swimBar->SetValue(timec);
		}
	}
	if(npc->GetInventory()->IsOpen() || npc->GetWeaponMode() == NPC_WEAPON_MAG){
		if(npc->GetAttribute(NPC_ATR_MANA_MAX) > 0){
			screen->InsertItem(manaBar);
			manaBar->SetMaxRange(0, npc->GetAttribute(NPC_ATR_MANA_MAX));
			manaBar->SetRange(0, npc->GetAttribute(NPC_ATR_MANA_MAX));
			manaBar->SetPreview(npc->GetAttribute(NPC_ATR_MANA));
			manaBar->SetValue(npc->GetAttribute(NPC_ATR_MANA));
		}
	}
	if(gStatusReadGameOptions){
		gStatusReadGameOptions = 0;
		zCOption* options = zCOption::GetOption();
		if(options){
			show_Focus = options->ReadBool(zOPT_SEC_OPTIONS, "show_Focus", show_Focus);
			show_FocusItm = options->ReadBool(zOPT_SEC_OPTIONS, "show_FocusItm", show_FocusItm);
			show_FocusMob = options->ReadBool(zOPT_SEC_OPTIONS, "show_FocusMob", show_FocusMob);
			show_FocusNpc = options->ReadBool(zOPT_SEC_OPTIONS, "show_FocusNpc", show_FocusNpc);
			show_FocusBar = options->ReadBool(zOPT_SEC_OPTIONS, "show_FocusBar", show_FocusBar);
		}
	}
	if(show_Focus){
		int itemType = -1;
		zSTRING PrintName;
		zCVob* focusVob = npc->GetFocusVob();
		if(!focusVob) return;
		PrintName.Clear();
		switch(GetPointerType((DWORD)focusVob))
		{
			case VOB_TYPE_VT_OCMOB:
			case VOB_TYPE_VT_OCMOBBED:
			case VOB_TYPE_VT_OCMOBCONTAINER:
			case VOB_TYPE_VT_OCMOBINTER:
			case VOB_TYPE_VT_OCMOBSWITCH:
			case VOB_TYPE_VT_OCMOBWHEEL:
			case VOB_TYPE_VT_OCMOBLOCKABLE:
			case VOB_TYPE_VT_OCMOBLADDER:
			{
				itemType = 0;
				oCMob* mob = reinterpret_cast<oCMob*>(focusVob);
				PrintName =  mob->GetMobName();
				if(PrintName.IsEmpty()) return;
				if(!show_FocusMob) return;
			}
			break;
			case VOB_TYPE_VT_OCNPC:
			{
				itemType = 1;
				oCNpc* npc_focus = reinterpret_cast<oCNpc*>(focusVob);
				PrintName = npc_focus->GetName();
				if(PrintName.IsEmpty()) return;
				if(!show_FocusNpc && !show_FocusBar) return;
				int npc_health = npc_focus->GetAttribute(NPC_ATR_HITPOINTS);
				if(npc_health > 0){
					if(show_FocusBar){
						int npc_maxhealth = npc_focus->GetAttribute(NPC_ATR_HITPOINTS_MAX);
						screen->InsertItem(focusBar);
						focusBar->SetMaxRange(0, npc_maxhealth);
						focusBar->SetRange(0, npc_maxhealth);
						focusBar->SetValue(npc_health);
					}
				}
				if(!show_FocusNpc) return;
			}
			break;
			case VOB_TYPE_VT_OCITEM:
			{
				itemType = 2;
				oCItem* item_focus = reinterpret_cast<oCItem*>(focusVob);
				PrintName = item_focus->GetName(1);
				if(PrintName.IsEmpty()) return;
				if(!show_FocusItm) return;
			}
			break;
			default:
				return;
			break;
		};
		zVEC3 Pos;
		zVEC3 Posa;
		DWORD VisualPtr = (DWORD)focusVob->visual;
		if(focusVob->visual && *(int*)focusVob->visual == 0x0083234C){
			Pos = zVEC3(focusVob->trafoObjToWorld.m[0][3], focusVob->trafoObjToWorld.m[1][3], focusVob->trafoObjToWorld.m[2][3]);
			Pos.y = *(float*)(VisualPtr + 0x100) * 1.600000023841858f + Pos.y;
			float multi = focusVob->bbox3D.bbox3D_maxs.y + 100.0f;
			if(Pos.y > multi)
				Pos.y = multi;
		}
		else{
			float a = focusVob->bbox3D.bbox3D_mins.z;
			float b = focusVob->bbox3D.bbox3D_maxs.z;
			float c = focusVob->bbox3D.bbox3D_mins.y;
			float d = focusVob->bbox3D.bbox3D_maxs.y;
			Pos.x = (focusVob->bbox3D.bbox3D_mins.x + focusVob->bbox3D.bbox3D_maxs.x) * 0.5f;
			Pos.y = (d + c) * 0.5f;
			Pos.z = (b + a) * 0.5f;
			Pos.y = (d - c) * 0.8199999928474426f + Pos.y;
		}
		zCCamera::GetCamera()->Activate();
		Posa = VectorMulti(zCCamera::GetCamera()->cameraMatrix, Pos);
		if(Posa.z <= 0.0f){
			return;
		}
		float unknown_a = 1.0 / Posa.z;
		DWORD CameraPtr = (DWORD)zCCamera::GetCamera();
		Pos.y = Posa.x * *(float*)(CameraPtr + 0x904) * unknown_a + *(float*)(CameraPtr + 0x98);
		Pos.z =  *(float*)(CameraPtr + 0x9C) - (Posa.y * *(float*)(CameraPtr + 0x908) * unknown_a);
		PrintName += "\n";
		int fontsize = zCView::GetScreen()->FontSize(PrintName);
		int naxval = zCView::GetScreen()->nax(fontsize / 2);
		Pos.y = Pos.y - naxval;
		int finalx = zCView::GetScreen()->anx(Pos.y);
		int finaly = zCView::GetScreen()->any(Pos.z);
		if(finalx < zCView::GetScreen()->FontY())
			finalx = zCView::GetScreen()->FontY();
		if(finalx > 8192 - zCView::GetScreen()->FontY())
			finalx = 8192 - zCView::GetScreen()->FontY();
		fontsize = zCView::GetScreen()->FontSize(PrintName);
		if(finaly >= 0){
			if(finaly > 8191 - fontsize)
				finaly = 8191 - fontsize;
		}
		else finaly = 0;
		zCView::GetScreen()->Print(finalx, finaly, PrintName);
	}
}