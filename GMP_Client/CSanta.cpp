
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

/*****************************************************************************
**																			**
**	File name:		CGmpClient/CSanta.cpp		   							**
**																			**
**	Created by:		04/12/11	-	skejt23									**
**																			**
**	Description:	Santa Claus	 											**
**																			**
*****************************************************************************/
#pragma warning( disable : 4244 )

#include "CSanta.h"
#include "CGmpClient.h"

extern CGmpClient* client;
extern zCView* PrintTimedScreen;
zSTRING Hoho = "HOHOHOHO.WAV";
zSTRING ChristmasDance = "T_DANCE_02";

enum CURRENT_MAP
{
	MAP_UNKNOWN,
	MAP_COLONY,
	MAP_OLDWORLD,
	MAP_KHORINIS,
	MAP_JARKENDAR
};

CSanta::CSanta(short Map)
{
	Mover = oCMobInter::_CreateNewInstance();
	Mover1 = oCMobInter::_CreateNewInstance();
	Throw = oCVisualFX::_CreateNewInstance();
	zCParticleFX* Par = zCParticleFX::Load("THROWDRUGS.PFX");
	Throw->SetVisual(Par);
	zCVisual* Vis = zCVisual::LoadVisual("OW_LOB_WOODPLANKS_V4.3DS");
	Mover->SetVisual(Vis);
	Vis = zCVisual::LoadVisual("OW_LOB_BUSH_WATER_V1.3DS");
	Mover1->SetVisual(Vis);
	oCGame::GetGame()->GetWorld()->AddVob(Mover);
	zVEC3 Pos;
	if(Map == MAP_COLONY || Map == MAP_OLDWORLD) Pos = zVEC3(441.0f, 1116.0f, 12601.0f);
	else Pos = zVEC3(7585.0f, 1554.0f, -4966.0f);
	Mover->SetPositionWorld(Pos);
	oCGame::GetGame()->GetWorld()->AddVobAsChild(Mover1, Mover);
	oCGame::GetGame()->GetWorld()->AddVobAsChild(Throw, Mover);
	Mover1->SetPositionWorld(Pos);
	Throw->SetPositionWorld(Pos);
	FlyStage Stage;
	if(Map == MAP_COLONY || Map == MAP_OLDWORLD){
		Stage.StagePos = Pos; // stage 0
		Stages.push_back(Stage);
		Stage.StagePos = zVEC3(-7181.0f, 792.0f, 1689.0f); // stage 1
		Stages.push_back(Stage);
		Stage.StagePos = zVEC3(-6747.0f, 792.0f, -5131.0f); // stage 2
		Stages.push_back(Stage);
		Stage.StagePos = zVEC3(5759.0f, 792.0f, -4102.0f); // stage 3
		Stages.push_back(Stage);
		Stage.StagePos = zVEC3(5745.0f, 792.0f, 3225.0f); // stage 4
		Stages.push_back(Stage);
	}
	else{ // KHORINIS
		Stage.StagePos = Pos; // stage 0
		Stages.push_back(Stage);
		Stage.StagePos = zVEC3(3594.0f, 1554.0f, -4301.0f); // stage 1
		Stages.push_back(Stage);
		Stage.StagePos = zVEC3(2387.0f, 1554.0f, 2044.0f); // stage 2
		Stages.push_back(Stage);
		Stage.StagePos = zVEC3(6250.0f, 1554.0f, 4249.0f); // stage 3
		Stages.push_back(Stage);
		Stage.StagePos = zVEC3(5975.0f, 1554.0f, -1352.0f); // stage 4
		Stages.push_back(Stage);
	}
	CurrentStage = 0;
	MaxStages = Stages.size()-1;
	throwtime = clock() + 5000;
	zVEC3 SpawnPos = zVEC3(0,0,0);
	SantaNPC = oCObjectFactory::GetFactory()->CreateNpc(oCNpc::GetHero()->GetInstance());
	SantaNPC->SetSleeping(1);
	SantaNPC->SetName("Santa Claus");
	SantaNPC->SetCollDet(0);
	SantaNPC->EquipArmor(oCObjectFactory::GetFactory()->CreateItem(zCParser::GetParser()->GetIndex("ITAR_KDF_L")));
	SantaNPC->Enable(SpawnPos);
};

CSanta::~CSanta()
{
	oCGame::GetGame()->GetSpawnManager()->DeleteNpc(SantaNPC);
	Mover1->RemoveVobFromWorld();
	Throw->RemoveVobFromWorld();
	Mover->RemoveVobFromWorld();
	SantaNPC = NULL;
	Mover1 = NULL;
	Throw = NULL;
	Mover = NULL;
};

void CSanta::MoveToStage(FlyStage Stage)
{
	int StageX = Stage.StagePos.x;
	int StageY = Stage.StagePos.z;
	zVEC3 Pos = Mover->GetPositionWorld();
	int PosX = Pos.x;
	int PosY = Pos.z;
	if(PosX != StageX)
	{
		if(StageX > PosX) PosX+=5;
		if(StageX < PosX) PosX-=5;
	}
	if(PosY != StageY)
	{
		if(StageY > PosY) PosY+=5;
		if(StageY < PosY) PosY-=5;
	}
	if(client->GetVectorSurfaceSphere(50.0f,PosX, PosY, 0, StageX, StageY, 0)){
		if(CurrentStage < MaxStages) CurrentStage++;
		else CurrentStage = 0;
	}
	Pos.x = PosX;
	Pos.z = PosY;
	Mover->SetPositionWorld(Pos);
};

void CSanta::RenderSanta()
{
	if(moveframe < clock()){
		moveframe = clock() + 1;
		MoveToStage(Stages[CurrentStage]);
		if(SantaNPC){
			if(!SantaNPC->GetModel()->IsAnimationActive(ChristmasDance)) SantaNPC->GetModel()->StartAnimation(ChristmasDance);
		}
		if(SantaNPC) SantaNPC->RotateWorldY(1.0f);
		if(SantaNPC)SantaNPC->SetPosition(Mover->GetPositionWorld().x, Mover->GetPositionWorld().y+100, Mover->GetPositionWorld().z);
	}
	if(throwtime < clock())
	{
		throwtime = clock() + 5000;
		int GiftInstance = rand() % 1957 + 5892;
		oCItem* Gift = oCObjectFactory::GetFactory()->CreateItem(GiftInstance);
		if(SantaNPC)SantaNPC->DoDropVob(Gift);
		if(!Gift->GetItemName().IsEmpty()) zCSoundSystem::GetSoundSystem()->PlaySound3D(Hoho, SantaNPC, 2);
	};
};