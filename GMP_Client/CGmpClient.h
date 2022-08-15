
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

#include <slikenet/MessageIdentifiers.h>
#include <slikenet/peerinterface.h>
#include <slikenet/statistics.h>
#include <slikenet/types.h>
#include <slikenet/BitStream.h>
#include <slikenet/sleep.h>
#include <slikenet/PacketLogger.h>

#include "CLanguage.h"
#include "CHeroClass.h"
#include "CSpawnPoint.h"
#include "HooksManager.h"
#include <string.h>
#include <string>
#include "WorldBuilder\load.h"
#include "CSyncFuncs.h"
#include "CPlayer.h"
#include "CObservation.h"

#define WARN(x) MessageBoxA(NULL, x, x, 0)

enum P_TYPE
{
        PT_MSG = ID_USER_PACKET_ENUM+1,
        PT_NEWCONN,      // <- u�ywa kto� tego?
        PT_REQUEST_FILE_LENGTH,
        PT_REQUEST_FILE_PART,
        PT_WHOAMI,
        PT_JOIN_GAME,
        PT_ACTUAL_STATISTICS,   // <- chyba tutaj będe musiał dodac optymalizacje gdyż nie potrzeba nam wszystkich informacji o graczu który jest od nas dalej niż 5000.0f
        PT_ALL_OTHERS,  //pakiety wysy�any tylko z serwera do klienta, informacje o wszystkich graj�cych wysy�ane jednorazowo
        PT_HP_DIFF,
        PT_MAP_ONLY,    //wysy�a tylko informacje o x,y graczy �eby mapka dzia�a�a
        PT_CHECKSUM,    //pakiet sprawdzaj�cy czy crc32 serwera i klienta s� zgodne
        PT_RCON,                //administrowanie
        PT_WHISPER,
        PT_EXTENDED_4_SCRIPTS,  //jak ju� kiedy� wdro�ymy skrypty
        PT_SRVMSG,
        PT_YOUR_NAME,
        PT_LEFT_GAME,
        PT_GAME_INFO,
        PT_MAP_NAME,
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

struct SPlayer{
	uint64_t id;
	oCNpc *npc;
	short hp, enable, update_hp_packet;
	unsigned char char_class;
};
struct MD5Sum{
	BYTE data[16];
};

union STime{
	int time;
	struct{
		unsigned short day;
		unsigned char hour ,min;
	};
};

class CGmpClient : public CSyncFuncs
{
public:
	CGmpClient(const char *ip, CLanguage *ptr);
	~CGmpClient(void);
	void HandleNetwork(void);
	BOOL IsConnected(void);
	BOOL IsReadyToJoin(void);
	bool Connect(void);
	void JoinGame(BYTE selected_class);
	zSTRING& GetLastError(void);
	void SendDropItem(short Instance, short amount);
	void SendTakeItem(short Instance);
	void SendCastSpell(oCNpc* Target, short SpellId);
	void SendMessage(const char* msg);
	void SendWhisper(const char* player_name, const char* msg);
	void SendRemoteMessage(const char* msg);
	void UpdatePlayerStats(short anim);
	void SendHPDiff(size_t who, short diff);
	void SyncGameTime(void);
	bool IsDisconnected(void);
	void Disconnect(void);
	bool IsInterrupted(void);
	/* według mnie to jest zbędny shit
	bool Send(std::string message); //Zmienic pozniej by wysyłało sPacket
	bool Receive(sPacket & packet);*/
	int GetPing();
	//*/
	CHeroClass *classmgr;
	CSpawnPoint *spawnpoint;
	vector<CPlayer*> player;
	SLNet::SystemAddress srvAddr;
	std::vector<Info> VobsWorldBuilderMap;
	int HeroLastHp;
	zSTRING map;
	bool IsAdminOrModerator;
	bool IgnoreFirstTimeMessage;
	unsigned char game_mode;
	int DropItemsAllowed;
	int ForceHideMap;
	CObservation::ObseravtionMode ObserveMode;
private:
	enum{
		WL_PREPARE_TO_JOIN,
		WL_JOIN_TO_GAME,
		WL_INGAME,
	};

	BOOL connected_with_srv, disconnected, interrupted;
	int clientPort;
	std::string clientHost;
	SLNet::RakPeerInterface * client;
	SLNet::RakNetStatistics *rss;
	CLanguage *lang;
	BYTE work_layer, error, tmp8[2];
	uint64_t playerID;
	time_t last_mp_regen;
	short mp_restore;

	bool Connect(std::string hostAddress, std::string hostPassword, int hostPort);
	BYTE AreDefaultItems(void);	//0 - yes| 1 - no
	MD5Sum *GetMD5(LPBYTE data, DWORD size);
	unsigned char GetPacketIdentifier(SLNet::Packet *p);
};

/*
Streszczenie działania:
Całą robotę z połączeniem i innymi bajerami sieciowymi będzie zajmowała się metoda HandleNetwork;
HandleNetwork będzie wywoływał odpowiednie metody zależnie od warstwy na której będzie pracował.
Warstwy:
0 - ładujemy podstawe dane z serwera i wybieramy klasę postaci
0a - wysyłamy crc mapy(może za to dostaniemy bana :D)
0b - pobieramy liste klas
0c - pobieramy liste spawnpointow
1 - po wstępnych ustawieniach dołączamy do gry
2 - zarzynamy się nawzajem/robimy sesje rpg/whatever
*/
