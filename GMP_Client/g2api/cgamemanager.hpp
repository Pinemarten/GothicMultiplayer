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
#include "ocgame.hpp"

/** Insert description. */
class CGameManager
{
public:
	void ApplySomeSettings(void){XCALL(0x004276B0);};
	// close g2
	void Done(void){XCALL(0x004254E0);};
	// Shows Real play time wtf?
	void ShowRealPlayTime(void){XCALL(0x0042BDE0);};
	// Do nothing
	void Run(void){XCALL(0x00425830);};
	int ExitGame(void){XCALL(0x00425780);};
	int ExitSession(void){XCALL(0x00425790);};
	oCGame* GetGame(void){XCALL(0x0042B1B0);};
	int GetPlaytimeSeconds(void){XCALL(0x00429280);};

	//.text:0042B7E0 ; public: int __thiscall CGameManager::IntroduceChapter(class zSTRING, class zSTRING, class zSTRING, class zSTRING, int)
	/** This method shows a "chapter introduction" screen.
	* @param Title Determines the chapter title
	* @param Subtitle Determines the subtitle
	* @param Texture Determines the background texture
	* @param Sound Determines which sound will be played when the introduction screen is being executed
	* @param WaitTime Determines how long the introduction screen will be shown
	* @usable Ingame only
	*/
	int IntroduceChapter(zSTRING Title, zSTRING Subtitle, zSTRING Texture, zSTRING Sound, int WaitTime)
	{
		XCALL(0x0042B7E0);
	};
	int IsGameRunning(void){XCALL(0x0042B200);};
	int IsIntroActive(void){XCALL(0x0042B1C0);};

	//.text:0042B940 ; public: int __thiscall CGameManager::PlayVideo(class zSTRING)
	/** This method plays a Bink video.
	* @param Filename Filename of the video (relative to data\videos\)
	* @usable Ingame only
	*/
	int PlayVideo(zSTRING Filename)
	{
		XCALL(0x0042B940);
	};

	//.text:0042BB10 ; public: int __thiscall CGameManager::PlayVideoEx(class zSTRING, short, int)
	/** This method plays a Bink video.
	* @param Filename Filename of the video (relative to data\videos\)
	* @param ScreenBlend Determines whether the "BLACK_SCREEN" should be played after playing the video (does not execute with ExitSession is true)
	* @param ExitSession Determines whether the game should be ended after playing the video.
	* @usable Ingame only
	*/
	int PlayVideoEx(zSTRING Filename, zBOOL ScreenBlend, zBOOL ExitSession)
	{
		XCALL(0x0042BB10);
	};
	inline static CGameManager*		GetGameManager(){ return *(CGameManager**)0x008C2958; };
};
