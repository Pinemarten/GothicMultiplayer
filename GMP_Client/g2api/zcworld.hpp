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
#include "macros.hpp"
#include "ocitem.hpp"
#include "ocnpc.hpp"
#include "ocspawnmanager.hpp"
#include "zcskycontroller.hpp"
#include "zcbsptree.hpp"

class zCVob;
class zCVertex;
class zCCSPlayer;
class zCWayNet;
class zCWorldPerFrameCallback;
class zCZone;
class zCSession;
class zCViewProgressBar;
class oCSpawnManager;

#define zCWorld_DIMENSION 3
#define VOB_HASHTABLE_SIZE 2048

enum zTWorldLoadMode		 
{   
	zWLD_LOAD_GAME_STARTUP,		 
	zWLD_LOAD_GAME_SAVED_DYN,	   
	zWLD_LOAD_GAME_SAVED_STAT,	  
	zWLD_LOAD_EDITOR_COMPILED,	  
	zWLD_LOAD_EDITOR_UNCOMPILED,	
	zWLD_LOAD_MERGE 
};

enum zTWorldSaveMode		 
{   
	zWLD_SAVE_GAME_SAVED_DYN,	   
	zWLD_SAVE_EDITOR_COMPILED,	  
	zWLD_SAVE_EDITOR_UNCOMPILED,	
	zWLD_SAVE_COMPILED_ONLY		 
};

enum zTWorldLoadMergeMode	
{   
	zWLD_LOAD_MERGE_ADD,					
	zWLD_LOAD_MERGE_REPLACE_ROOT_VISUAL 
};

enum zTStaticWorldLightMode 
{   
	zWLD_LIGHT_VERTLIGHT_ONLY,
	zWLD_LIGHT_VERTLIGHT_LIGHTMAPS_LOW_QUAL,
	zWLD_LIGHT_VERTLIGHT_LIGHTMAPS_MID_QUAL,
	zWLD_LIGHT_VERTLIGHT_LIGHTMAPS_HI_QUAL
};

/** Insert description. */
class zCWorld : public zCObject
{
private:
	zCTree<zCVob>			globalVobTree;

	int						foundHit;
	zCVob*					foundVob;  
	zCPolygon*				foundPoly;
	zVEC3					foundIntersection;
	zVEC3					foundPolyNormal;   
	zCVertex*				foundVertex;
	zCSession*				ownerSession;
	zCCSPlayer*				csPlayer;

	zSTRING					m_strlevelName;			  
	int						compiled;
	int						compiledEditorMode;
	int						traceRayIgnoreVobFlag;			  
	int						m_bIsInventoryWorld;		
	zTWld_RenderMode		worldRenderMode;
	zCWayNet*				wayNet;
	zTFrameCtr				masterFrameCtr;		
	float					vobFarClipZ;			  
	float					vobFarClipZScalability;
	
	zCArray<zCVob*>			traceRayVobList;
	zCArray<zCVob*>			traceRayTempIgnoreVobList;

	int						renderingFirstTime;
	int						showWaynet;	   
	int						showTraceRayLines;		
	zCViewProgressBar*  	progressBar;
	DWORD					unarchiveFileLen;
	DWORD					unarchiveStartPosVobtree;
	int						numVobsInWorld;

	zCArray<zCWorldPerFrameCallback*> perFrameCallbackList;

	zCSkyControlerIndoor*	skyControlerIndoor;
	zCSkyControlerOutdoor*	skyControlerOutdoor;
	zCSkyControler*			activeSkyControler;

	zCArray<zCZone*>		zoneGlobalList;
	zCArraySort<zCZone*>	zoneActiveList;
	zCArraySort<zCZone*>	zoneLastClassList;

	//zCVobBBox3DSorter<zCZone> zoneBoxSorter
	/*0x0118*/		  int zoneBoxSorter_vtbl;	 // Method table
	//zCArray<zTBoxSortHandle *>		handles;
	/*0x011C*/		  int zoneBoxSorter_handles_array;		 //zTBoxSortHandle**
	/*0x0120*/		  int zoneBoxSorter_handles_numAlloc;	  //int
	/*0x0124*/		  int zoneBoxSorter_handles_numInArray;	//int
	//zCArraySort<zTNode*> nodeList[zCWorld_DIMENSION];
	//0
	/*0x0128*/		  int zoneBoxSorter_nodeList0_array;		 //zTNode**
	/*0x012C*/		  int zoneBoxSorter_nodeList0_numAlloc;	  //int			 
	/*0x0130*/		  int zoneBoxSorter_nodeList0_numInArray;	//int			 
	/*0x0134*/		  int zoneBoxSorter_nodeList0_compare;	   //int	(*Compare)(const void* ele1,const void* ele2)
	//1												
	/*0x0138*/		  int zoneBoxSorter_nodeList1_array;		 //zTNode**
	/*0x013C*/		  int zoneBoxSorter_nodeList1_numAlloc;	  //int			 
	/*0x0140*/		  int zoneBoxSorter_nodeList1_numInArray;	//int			 
	/*0x0144*/		  int zoneBoxSorter_nodeList1_compare;	   //int	(*Compare)(const void* ele1,const void* ele2)
	//2												
	/*0x0148*/		  int zoneBoxSorter_nodeList2_array;		 //zTNode**
	/*0x014C*/		  int zoneBoxSorter_nodeList2_numAlloc;	  //int			 
	/*0x0150*/		  int zoneBoxSorter_nodeList2_numInArray;	//int			 
	/*0x0154*/		  int zoneBoxSorter_nodeList2_compare;	   //int	(*Compare)(const void* ele1,const void* ele2)
	/*0x0158*/		  int zoneBoxSorter_sorted;				  //zBOOL

	//zCVobBBox3DSorter<zCZone>::zTBoxSortHandle	zoneActiveHandle;
	/*0x015C*/			int zoneActiveHandle_vtbl;				  //Pointer to method table
	/*0x0160*/			int zoneActiveHandle_mySorter;			  //zCBBox3DSorterBase*
						zTBBox3D bbox3D;

	/*0x017C*/			int zoneActiveHandle_indexBegin [zCWorld_DIMENSION]; //int
	/*0x0188*/			int zoneActiveHandle_indexEnd   [zCWorld_DIMENSION]; //int

	//zCArray<VOB*> activeList;
	/*0x0194*/				int zoneActiveHandle_activeList_array;		 //VOB **
	/*0x0198*/				int zoneActiveHandle_activeList_numAlloc;	  //int			 
	/*0x019C*/				int zoneActiveHandle_activeList_numInArray;	//int			 

	int						addZonesToWorld;
	int						showZonesDebugInfo;

	zCCBspTree*				cbspTree;
	zCBspTree				bspTree;

	zCArray<zCVob*>			activeVobList;
	zCArray<zCVob*>			walkList;
	zCArray<zCVob*>			vobHashTable[VOB_HASHTABLE_SIZE];

	zSTRING					worldFilename;
	zSTRING					worldName;

	zCListSort<zCVob>*		voblist;
	zCListSort<oCNpc>*		voblist_npcs;
	zCListSort<oCItem>*		voblist_items;

public:
	zCTree<zCVob>* AddVob(zCVob* pA){XCALL(0x00624810)};
	zCTree<zCVob>* AddVobAsChild(zCVob* pA, zCVob* pB){XCALL(0x006247F0)};
	void RemoveVob(zCVob* pA){XCALL(0x007800C0)};
	void SearchVobListByName(const zSTRING& name, zCArray<zCVob*>& resultList){XCALL(0x007806C0);};
	//zCVob* SearchVobByName(const zSTRING& name){XCALL(0x00623FB0);};
	void SetSkyControlerIndoor(zCSkyControler*){XCALL(0x006203D0)};
	void SetSkyControlerOutdoor(zCSkyControler*){XCALL(0x00620410)};
	void SetVobFarClipZ(float value){this->vobFarClipZ = value;};
	float GetVobFarClipZ(void){return this->vobFarClipZ;};
	void StartSnow(){GetOutdoorSkyController()->SetWeatherType(ZTWEATHER_SNOW);};
	zCListSort<zCVob>* GetVobList(void){return this->voblist;};
	zCListSort<oCNpc>* GetNpcList(void){return this->voblist_npcs;};
	zSTRING& GetWorldName(void){return this->worldFilename;};
	zCWayNet* GetWayNet(void){return this->wayNet;};
	zCListSort<oCItem>* GetItemList(void){return this->voblist_items;};
	zCArray<zCVob*> GetArrayOfVobs(void){return this->walkList;};
	zCSkyControlerIndoor* GetIndoorSkyController(void){return this->skyControlerIndoor;};
	zCSkyControlerOutdoor* GetOutdoorSkyController(void){return this->skyControlerOutdoor;};
	zCSkyControler* GetActiveSkyController(void){return this->activeSkyControler;};
	zCSkyControler* GetSkyControler(void){XCALL(0x006203A0);};
	zCBspTree* GetBspTree(void){XCALL(0x006D0020)};
	static void DeleteAllNpcs(){
		zCListSort<oCNpc>* NpcList= oCGame::GetGame()->GetWorld()->GetNpcList();
		int size=NpcList->GetSize();
		for(int i=0; i<size; i++){
			NpcList=NpcList->GetNext();
			oCNpc *NpcOnList=NpcList->GetData();
			if(NpcOnList!=oCNpc::GetHero()) oCGame::GetGame()->GetSpawnManager()->DeleteNpc(NpcOnList);
		}
		oCGame::GetGame()->GetSpawnManager()->SetSpawningEnabled(0);
	}
	static void DeleteAllNpcsAlt(){
		zCListSort<oCNpc>* NpcList= oCGame::GetGame()->GetWorld()->GetNpcList();
		int size=NpcList->GetSize();
		for(int i=0; i<size; i++){
			NpcList=NpcList->GetNext();
			oCNpc *NpcOnList=NpcList->GetData();
			if(NpcOnList->GetInstance()!=11471) NpcOnList->Disable();
		}
		oCGame::GetGame()->GetSpawnManager()->SetSpawningEnabled(0);
	}
};
