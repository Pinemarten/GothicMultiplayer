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
#ifndef __API_G2_CONSTANTS_H__
#define __API_G2_CONSTANTS_H__

// engine constants from constants.d

#include "ztypes.hpp"

// NPC FLAGS
const zINT ATR_HITPOINTS			=  0;
const zINT ATR_HITPOINTS_MAX		=  1;
const zINT ATR_MANA					=  2;
const zINT ATR_MANA_MAX				=  3;
const zINT ATR_STRENGTH				=  4;
const zINT ATR_DEXTERITY			=  5;
const zINT ATR_REGENERATEHP			=  6;
const zINT ATR_REGENERATEMANA		=  7; 

const zINT ATR_INDEX_MAX			=  8;

// FIGHT MODES
const zINT FMODE_NONE				=  0;
const zINT FMODE_FIST				=  1;
const zINT FMODE_MELEE				=  2;
const zINT FMODE_FAR				=  5;
const zINT FMODE_MAGIC				=  7;

// WALK MODES
const zINT NPC_RUN					=  0;
const zINT NPC_WALK					=  1;
const zINT NPC_SNEAK				=  2;
const zINT NPC_RUN_WEAPON			=  0 + 128;
const zINT NPC_WALK_WEAPON			=  1 + 128;
const zINT NPC_SNEAK_WEAPON			=  2 + 128;

// ARMOR FLAGS
const zINT WEAR_TORSO				=  1;		//	Torso	( Chest armor )
const zINT WEAR_HEAD				=  2;		//	Head	( Helmet )
const zINT WEAR_EFFECT				=  16;		


// INVENTORY CATEGORIES
const zINT INV_WEAPON				=  1;
const zINT INV_ARMOR				=  2;
const zINT INV_RUNE					=  3;
const zINT INV_MAGIC				=  4;
const zINT INV_FOOD					=  5;
const zINT INV_POTION				=  6;
const zINT INV_DOC					=  7;
const zINT INV_MISC					=  8;
const zINT INV_CAT_MAX				=  9;

// ITEM FLAGS
const zINT ITEM_KAT_NONE			= 1 << 0;  // Misc.
const zINT ITEM_KAT_NF				= 1 << 1;  // Melee Weapons
const zINT ITEM_KAT_FF				= 1 << 2;  // Bows, Crossbows, etc.
const zINT ITEM_KAT_MUN				= 1 << 3;  // Ammunition (MULTI)
const zINT ITEM_KAT_ARMOR			= 1 << 4;  // Armors
const zINT ITEM_KAT_FOOD			= 1 << 5;  // Food (MULTI)
const zINT ITEM_KAT_DOCS			= 1 << 6;  // Documents
const zINT ITEM_KAT_POTIONS			= 1 << 7;  // Potions
const zINT ITEM_KAT_LIGHT			= 1 << 8;  // Lights
const zINT ITEM_KAT_RUNE			= 1 << 9;  // Runes/Scrolls
const zINT ITEM_KAT_MAGIC			= 1 << 31; // Rings/Amulets/Belts
const zINT ITEM_KAT_KEYS			= ITEM_KAT_NONE;
// weapon type (flags)
const zINT ITEM_DAG					= 1 << 13;  // Daggers (OBSOLETE!)
const zINT ITEM_SWD					= 1 << 14;  // Swords
const zINT ITEM_AXE					= 1 << 15;  // Axes
const zINT ITEM_2HD_SWD				= 1 << 16;  // 2 hand weapons
const zINT ITEM_2HD_AXE				= 1 << 17;  // 2 hand axes
const zINT ITEM_SHIELD				= 1 << 18;  // Shields (OBSOLETE!)
const zINT ITEM_BOW					= 1 << 19;  // Bows
const zINT ITEM_CROSSBOW			= 1 << 20;  // Crossbows
// magic type (flags)
const zINT ITEM_RING				= 1 << 11;  // Ring
const zINT ITEM_AMULET				= 1 << 22;  // Amulet
const zINT ITEM_BELT				= 1 << 24;  // Belt
// attributes (flags)
const zINT ITEM_DROPPED 			= 1 << 10;  // (INTERNAL!)
const zINT ITEM_MISSION 			= 1 << 12;  // Quest item
const zINT ITEM_MULTI				= 1 << 21;  // Stackable
const zINT ITEM_NFOCUS				= 1 << 23;  // (INTERNAL!)
const zINT ITEM_CREATEAMMO			= 1 << 25;  // Creates ammo itself (magic)
const zINT ITEM_NSPLIT				= 1 << 26;  // Non-split-item (Weapon as interact-item!)
const zINT ITEM_DRINK				= 1 << 27;  // (OBSOLETE!)
const zINT ITEM_TORCH				= 1 << 28;  // Torch
const zINT ITEM_THROW				= 1 << 29;  // (OBSOLETE!)
const zINT ITEM_ACTIVE				= 1 << 30;  // (INTERNAL!)

// GUILDS
const zINT GIL_NONE						= 0;	// (none)
const zINT GIL_HUMAN					= 1;	// Special Guild -> To set Constants for ALL Human Guilds --> used in species.d
const zINT GIL_PAL						= 1;	// Paladin
const zINT GIL_MIL						= 2;	// Militia
const zINT GIL_VLK						= 3;	// Citizen
const zINT GIL_KDF						= 4;	// Circle of Fire - mage
const zINT GIL_NOV						= 5;	// Circle of Fire - novice
const zINT GIL_DJG						= 6;	// Dragonchaser
const zINT GIL_SLD						= 7;	// Mercenary
const zINT GIL_BAU						= 8;	// Peasant
const zINT GIL_BDT						= 9;	// Bandit
const zINT GIL_STRF						= 10; 	// Prisoner
const zINT GIL_DMT						= 11;	// Dementor
const zINT GIL_OUT						= 12; 	// Outlander (z.B. kleine Bauernhöfe)
const zINT GIL_PIR						= 13;	// Pirate
const zINT GIL_KDW						= 14;	// Circle of Water
const zINT GIL_EMPTY_D					= 15;	// DO NOT USE!
//-----------------------------------------------
const zINT GIL_PUBLIC					= 15; 	// For public portal rooms
//-----------------------------------------------
const zINT GIL_SEPERATOR_HUM			= 16;
//-----------------------------------------------
const zINT GIL_MEATBUG					= 17;
const zINT GIL_SHEEP					= 18;
const zINT GIL_GOBBO					= 19; 	// Green Goblin / Black Goblin
const zINT GIL_GOBBO_SKELETON			= 20;
const zINT GIL_SUMMONED_GOBBO_SKELETON 	= 21;
const zINT GIL_SCAVENGER				= 22; 	// (whenever needed) Scavenger / Evil Scavenger / OrcBiter
const zINT GIL_GIANT_RAT				= 23;
const zINT GIL_GIANT_BUG				= 24;
const zINT GIL_BLOODFLY					= 25;
const zINT GIL_WARAN					= 26; 	// Waren / Feuerwaran
const zINT GIL_WOLF						= 27; 	// Wolf / Warg
const zINT GIL_SUMMONED_WOLF			= 28;
const zINT GIL_MINECRAWLER				= 29; 	// Minecrawler / Minecrawler Warrior
const zINT GIL_LURKER					= 30;
const zINT GIL_SKELETON					= 31;
const zINT GIL_SUMMONED_SKELETON		= 32;
const zINT GIL_SKELETON_MAGE			= 33;
const zINT GIL_ZOMBIE					= 34;
const zINT GIL_SNAPPER					= 35; 	// Snapper / Dragon Snapper /Razor
const zINT GIL_SHADOWBEAST				= 36;	// Shadowbeast / Bloodhound
const zINT GIL_SHADOWBEAST_SKELETON		= 37;
const zINT GIL_HARPY					= 38;
const zINT GIL_STONEGOLEM				= 39;
const zINT GIL_FIREGOLEM				= 40;
const zINT GIL_ICEGOLEM					= 41;
const zINT GIL_SUMMONED_GOLEM			= 42;
const zINT GIL_DEMON					= 43;
const zINT GIL_SUMMONED_DEMON			= 44;
const zINT GIL_TROLL					= 45; 	// Troll / Black Troll
const zINT GIL_SWAMPSHARK				= 46; 	// (whenever needed)
const zINT GIL_DRAGON					= 47; 	// Firedragon / Icedragon / Rockdragon / Swampdragon / Undead dragon
const zINT GIL_MOLERAT					= 48; 	// Molerat
//-----------------------------------------------
const zINT GIL_ALLIGATOR				= 49;
const zINT GIL_SWAMPGOLEM				= 50;
const zINT GIL_Stoneguardian			= 51;
const zINT GIL_Gargoyle					= 52;
const zINT GIL_Empty_A					= 53;
const zINT GIL_SummonedGuardian			= 54;
const zINT GIL_SummonedZombie			= 55;
const zINT GIL_EMPTY_B					= 56;
const zINT GIL_EMPTY_C					= 57;
//-----------------------------------------------
const zINT GIL_SEPERATOR_ORC			= 58;	// (was. 37)
//-----------------------------------------------
const zINT GIL_ORC						= 59;	// Ork-Warrior / Ork-Shaman / Ork-Elite
const zINT GIL_FRIENDLY_ORC				= 60;	// Ork-Slave / Ur-Shak
const zINT GIL_UNDEADORC				= 61;
const zINT GIL_DRACONIAN				= 62;
//-----------------------------------------------
const zINT GIL_EMPTY_X					= 63;
const zINT GIL_EMPTY_Y					= 64;
const zINT GIL_EMPTY_Z					= 65;
//-----------------------------------------------
const zINT GIL_MAX						= 66;	// (was 42)

// TALENTS
const zINT NPC_TALENT_UNKNOWN			= 0;

// Skilled Talents
const zINT NPC_TALENT_1H				= 1;
const zINT NPC_TALENT_2H				= 2;
const zINT NPC_TALENT_BOW				= 3;
const zINT NPC_TALENT_CROSSBOW			= 4;

const zINT NPC_TALENT_PICKLOCK			= 5;	// Now regulated by dextricity
//const zINT NPC_TALENT_PICKPOCKET		= 6;	// Old pickpocket from Gothic -- DO NOT USE

// Circle of magic
const zINT NPC_TALENT_MAGE				= 7;

// Special-Talents
const zINT NPC_TALENT_SNEAK				= 8;
const zINT NPC_TALENT_REGENERATE		= 9;	// Unused
const zINT NPC_TALENT_FIREMASTER		= 10;	// Unused
const zINT NPC_TALENT_ACROBAT			= 11;

// New talents. Fully regulated by scripts.
const zINT NPC_TALENT_PICKPOCKET		= 12;
const zINT NPC_TALENT_SMITH				= 13;
const zINT NPC_TALENT_RUNES				= 14;
const zINT NPC_TALENT_ALCHEMY			= 15;
const zINT NPC_TALENT_TAKEANIMALTROPHY	= 16;

const zINT NPC_TALENT_FOREIGNLANGUAGE	= 17;
const zINT NPC_TALENT_WISPDETECTOR		= 18;
const zINT NPC_TALENT_C					= 19;
const zINT NPC_TALENT_D					= 20;
const zINT NPC_TALENT_E					= 21;

const zINT NPC_TALENT_MAX				= 22;	// (was 12)

#endif //__API_G2_CONSTANTS_H__