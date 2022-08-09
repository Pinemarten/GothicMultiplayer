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

#ifndef __API_G2_OCGAME_H__
#define __API_G2_OCGAME_H__

#pragma once

#include "ztypes.hpp"
#include "macros.hpp"
#include "zcsession.hpp"
#include "zcvisual.hpp"


class zCView;
class oCNpc;
class zCWorld;
class oCWorldTimer;
class zCViewProgressBar;
class oCSavegameManager;
class oCViewStatusBar;
class oCGameInfo;
class zCVobLight;	
class oCGuilds;						
class oCInfoManager;			   
class oCNewsManager;				 
class oCSVMManager;		   
class oCTradeManager;			   
class oCPortalRoomManager;
class oCSpawnManager;
class oCWorld;

#define UsePotionKeys *(int*)(0x008B21CC)

enum oHEROSTATUS
{
	HERO_NORMAL,
	HERO_THREATENED,
	HERO_FIGHTING
};
	
enum oTGameDialogView
{
	GAME_VIEW_SCREEN,
	GAME_VIEW_CONVERSATION,	   
	GAME_VIEW_AMBIENT,	   
	GAME_VIEW_CINEMA,	   
	GAME_VIEW_CHOICE,
	GAME_VIEW_NOISE,
	GAME_VIEW_MAX
};

/** Insert description */
typedef struct 
{
	zSTRING	 objName;
	int		 stateNum;
	int		 hour1,min1;
	int		 type;
} TObjectRoutine;

/** Insert description. */
class oCGame : public zCSession
{
public:
	float 				cliprange;
	float 				fogrange;
	int 				inScriptStartup;
	int 				inLoadSaveGame;
	int 				inLevelChange;
	
	zCView* 			array_view[GAME_VIEW_MAX];
	int 				array_view_visible[GAME_VIEW_MAX];
	int 				array_view_enabled[GAME_VIEW_MAX];
	
	oCSavegameManager* 	savegameManager;	  
	zCView*				game_text;						   
	zCView*				load_screen;						   
	zCView*				save_screen;					   
	zCView*				pause_screen;						
	oCViewStatusBar* 	hpBar;
	oCViewStatusBar* 	swimBar;  
	oCViewStatusBar* 	manaBar;
	oCViewStatusBar* 	focusBar;
	int 				showPlayerStatus;

	int 				game_drawall;
	int 				game_frameinfo;
	int 				game_showaniinfo;
	int 				game_showwaynet;
	int 				game_testmode;
	int 				game_editwaynet;
	int 				game_showtime;
	int 				game_showranges;
	int 				drawWayBoxes;
	int 				scriptStartup;
	int 				showFreePoints; 
	oCNpc*  			showRoutineNpc;   

	int 				loadNextLevel;							 
	zSTRING 			loadNextLevelName;							
	zSTRING 			loadNextLevelStart;						 
	
	zVEC3 				startpos;				   
	oCGameInfo* 		gameInfo;
	zCVobLight*  		pl_light;				
	float 				pl_lightval;						  

	oCWorldTimer*   	wldTimer;		   
	float 				timeStep;
	int 				singleStep;							 

	oCGuilds* 			guilds;						
	oCInfoManager* 		infoman;			   
	oCNewsManager*		newsman;				 
	oCSVMManager*  		svmman;		   
	oCTradeManager*  	trademan;			   
	oCPortalRoomManager*portalman;
	oCSpawnManager* 	spawnman;		 
	
	float 				music_delay;						   
	oCNpc* 				watchnpc;							   

	int 				m_bWorldEntered;							 
	float 				m_fEnterWorldTimer;						   
	
	int 				initial_hour;									
	int 				initial_minute;								 

	zCArray<zCVob*>		   debugInstances;

	int 				debugChannels;	  
	int 				debugAllInstances;

	int 				oldRoutineDay;
	
	zCListSort<TObjectRoutine>	objRoutineList;
		
	zCListSort<TObjectRoutine>* currentObjectRoutine;
	
	zCViewProgressBar* 	progressBar;

	zCArray<zCVisual*>  visualList;
public:
	void CloseLoadscreen(void){XCALL(0x006C2BD0);};
	void CloseSavescreen(void){XCALL(0x006C2630);};
	void ClearGameState(void){XCALL(0x006C5ED0);};
	void DeleteTorches(void){XCALL(0x006CB640);};
	oHEROSTATUS GetHeroStatus(void){XCALL(0x006C2D10);};
	oCViewStatusBar* GetHealthBar(){ return this->hpBar;};
	zVEC3 GetStartPos(void){XCALL(0x006BFE50);};
	void LoadWorld(int, const zSTRING & pA){XCALL(0x006C90B0);};
	void LoadGame(int, const zSTRING & pA){XCALL(0x006C65A0);};

	//.text:006C4E70 ; public: virtual void __thiscall oCGame::GetTime(int &, int &, int &)
	/** This method returns the current ingame time.
	* @param day Reference to integer variable for the day value.
	* @param hwr Reference to integer variable for the hour value. 
	* @param min Reference to integer variable for the minute value.
	* @usable Ingame only
	*/
	void GetTime(int & day, int & hwr, int & min)
	{
		XCALL(0x006C4E70);
	};
	oCWorldTimer* GetWorldTimer(void){XCALL(0x006C2C80);};
	void SwitchCamToNextNpc(void){XCALL(0x006FBF40);};
	zCWorld* GetWorld(void){XCALL(0x005DECF0);};
	void OpenLoadscreen(int p1, zSTRING p2){XCALL(0x006C2690);};
	void OpenSavescreen(int p1){XCALL(0x006C2250);};
	void Pause(int p1 = 0){XCALL(0x006C8AD0);};		//raczej nie skorzystam
	oCSpawnManager* GetSpawnManager(void){XCALL(0x006C2D00);};
	//.text:006C4DE0 ; public: virtual void __thiscall oCGame::SetTime(int, int, int)
	/** This method sets the ingame time.
	* @param day Day
	* @param hwr Hour
	* @param min Minute
	* @usable Ingame only
	*/
	void SetTime(int day, int hwr, int min){XCALL(0x006C4DE0);};
	void Unpause(void){XCALL(0x006C8D50);};
	zCViewProgressBar* GetProgressBar(void){return *(zCViewProgressBar**)(this+0x172);};
	inline static oCGame* GetGame() { return *(oCGame**)0x00AB0884; };
	oCWorld *GetGameWorld(void) { XCALL(0x006C2C30); };
	void CallScriptStartup(void){XCALL(0x006C1C70);};
	void LoadWorldStartup(const zSTRING & pA){XCALL(0x006C9C10);};
	void ChangeLevel(const zSTRING & pA, const zSTRING & pB){XCALL(0x006C7290);};
	void CallScriptInit(void){XCALL(0x006C1F60);};
	void EnterWorld(class oCNpc *, int, zSTRING const& pA) { XCALL(0x006C96F0); };
};

#endif // __API_G2_OCGAME_H__