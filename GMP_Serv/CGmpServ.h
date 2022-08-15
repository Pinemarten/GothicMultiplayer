

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

#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetStatistics.h>
#include <RakNetTypes.h>
#include <BitStream.h>
#include <RakThread.h>
#include <RakSleep.h>
#include <PacketLogger.h>

#include <string.h>
#include <string>
#include <vector>
#include <ctime>
#include "CConfig.h"
#include "CSpawnList.h"
#include "CWBFile.h"
#include "CClasses.h"   //CClassManager
#include "CPermissions.h"

#define DEFAULT_PORT 0xDEAD
#define DEFAULT_ADMIN_PORT 0x404

class CLog;

enum CONFIG_FLAGS{
	QUICK_POTS		= 0x01,
	DROP_ITEMS		= 0x02,
	HIDE_MAP		= 0x04,
	MANA_REGENERATION	= 0x08,
};

class CGmpServ:CConfig
{
public:
	enum PL_FLAGS{
		PL_UNCONCIOUS = 0x01,   //00000001
		PL_BURN = 0x02, //00000010
	};
	enum P_TYPE
	{
		PT_MSG = ID_USER_PACKET_ENUM+1,
		PT_NEWCONN,	 // <- używa ktoś tego?
			PT_REQUEST_FILE_LENGTH,
			PT_REQUEST_FILE_PART,
			PT_WHOAMI,
			PT_JOIN_GAME,
			PT_ACTUAL_STATISTICS,   // <- chyba tutaj będe musiał dodac optymalizacje gdyż nie potrzeba nam wszystkich informacji o graczu który jest od nas dalej niż 5000.0f
			PT_ALL_OTHERS,  //pakiety wysyłany tylko z serwera do klienta, informacje o wszystkich grających wysyłane jednorazowo
			PT_HP_DIFF,
			PT_MAP_ONLY,	//wysyła tylko informacje o x,y graczy żeby mapka działała
			PT_CHECKSUM,	//pakiet sprawdzający czy crc32 serwera i klienta są zgodne
			PT_RCON,		//remote console, czyli zdalny admin
			PT_WHISPER,
			PT_EXTENDED_4_SCRIPTS,  //jak już kiedyś wdrożymy skrypty
			PT_SRVMSG,
			PT_YOUR_NICKNAME,
			PT_LEFT_GAME,	//+
			PT_GAME_INFO,	//+
			PT_MAP_NAME,	//+
			PT_DODIE,
			PT_RESPAWN,
			PT_DROPITEM,
			PT_TAKEITEM,
			PT_CASTSPELL,
			PT_CASTSPELLONTARGET,
	};
	enum FILE_REQ{
		CLASS_FILE	= 1,
		SPAWN_FILE	= 2,
		WB_FILE		= 3,
		NULL_SIZE	= 255
	};
	struct sPacket
	{
		P_TYPE type;
		std::string data;
	};
	struct sPlayer{
		RakNet::RakNetGUID id;
		std::string name;
		unsigned char char_class, flags, head, skin, body, walkstyle, figth_pos, spellhand, headstate, has_admin, admin_passwd, moderator_passwd, is_ingame, passed_crc_test, mute;
		SModerator *moderator;
		short health, mana;
		float pos[3];
		float nrot[3];
		//miejce na obrót głowy
		time_t tod; //time of death
		unsigned short left_hand, right_hand, armor, rangedeq, meleeeq, animation;
	};
public:
	CGmpServ(const char *password, int argc, char **argv);
	CGmpServ(int port, const char * password, unsigned short maxConnections);
	~CGmpServ(void);
	static RAK_THREAD_DECLARATION(AddToPublicListHTTP);
	bool Send(std::string message);
	bool Receive(sPacket & packet);
	bool Init();
	void SaveBanList(void);
	bool IsPublic(void);
	void DoRespawns(void);
	void SendSpamMessage(void);
	void RegenerationHPMP(void);
private:
	void DeleteFromPlayerList(RakNet::RakNetGUID guid);
	sPlayer* FindPlayer(const char* nickname);
	void LoadBanList(void);
	void HandleCastSpell(RakNet::Packet* p, bool Target);
	void HandleDropItem(RakNet::Packet* p);
	void HandleTakeItem(RakNet::Packet* p);
	void HandleFileReq(RakNet::Packet *p);
	void SomeoneJoinGame(RakNet::Packet *p);
	void HandlePlayerUpdate(RakNet::Packet *p);
	void MakeHPDiff(RakNet::Packet *p);
	void HandleNormalMsg(RakNet::Packet *p);
	void HandleWhisp(RakNet::Packet *p);
	void HandleRMConsole(RakNet::Packet *p);
	void HandleGameInfo(RakNet::Packet *p);
	void HandleMapNameReq(RakNet::Packet *p);
	void SendDisconnectionInfo(uint64_t disconnected_id);
	void SendDeathInfo(uint64_t deadman);
	void SendRespawnInfo(uint64_t luckyguy);
	void SendGameInfo(RakNet::RakNetGUID who);
	size_t FindIDOnList(uint64_t guid);

	std::vector<std::string> ban_list;
	CLog *log;
	CSpawnList *spawnlist;
	CClassManager *classmgr;
	CWBFile *wb_mgr;
	time_t last_stand_timer;
	time_t regen_time;
	
	unsigned char GetPacketIdentifier(RakNet::Packet *p);
	int serverPort;
	int arg_count;
	char **arg_vec;
	unsigned short maxConnections;
	time_t spam_time;
	std::string serverPassword;
	RakNet::RakPeerInterface * server;
	RakNet::RakNetStatistics *rss;
	std::vector<sPlayer> players;
	DataStructures::List<RakNet::RakNetSocket2* > sockets;
};
