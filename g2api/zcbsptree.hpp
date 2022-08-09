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

class zCBspNode;
class zCBspLeaf;
class zCBspSector;
class zCCBspTree;
class zCVob;
class zCVobLight;
class zCBspSector;
class zCBspBase;
class zCMesh;
class zCPolygon;

enum zTBspTreeMode	
{   
	zBSP_MODE_INDOOR, 
	zBSP_MODE_OUTDOOR, 
	zBSP_MODE_FORCEINT	= 0x7FFFFFFF
};

enum zTWld_RenderMode
{ 
	zWLD_RENDER_MODE_VERT_LIGHT,		
	zWLD_RENDER_MODE_LIGHTMAPS,
	zWLD_RENDER_MODE_FORCEINT	= 0x7FFFFFFF
};

/** Insert description */
class zCBspTree
{
	zCBspNode*				actNodePtr;
	zCBspLeaf*				actLeafPtr;

	zCBspBase*				bspRoot;
	zCMesh*					mesh;
	zCPolygon**				treePolyList;
	zCBspNode*				nodeList;
	zCBspLeaf*				leafList;
	int						numNodes;   
	int						numLeafs;	 
	int						numPolys;	   

	zCArray<zCVob*>			renderVobList;
	zCArray<zCVobLight*>	renderLightList;
	zCArray<zCBspSector*>	sectorList;
	zCArray<zCPolygon*>	 portalList;												

	zTBspTreeMode			bspTreeMode; 
	zTWld_RenderMode		worldRenderMode;
	float					vobFarClipZ;			 

	struct zTPlane
	{
		float distance;
		zPOINT3 normal;
	};

	zTPlane					vobFarPlane;
	int						vobFarPlaneSignbits;					   
	int 					drawVobBBox3D;			
	int 					leafsRendered;				  
	int 					vobsRendered;				
	int 					m_bRenderedFirstTime;		
	zTFrameCtr 				masterFrameCtr;
	zCPolygon** 			actPolyPtr;
	int 					compiled;
public:
	zCArray<zCVob*> GetVobsInBsp(void){return this->renderVobList;};
	void AddVob(zCVob*){XCALL(0x00531040)};
};
