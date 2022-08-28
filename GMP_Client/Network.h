#pragma once

#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#include "MessageIdentifiers.h"
#include <string>
#include <map>
#include <functional>

class CGmpClient;

class Network
{
private: //Wszystko prywatne, network to ma być black box
    CGmpClient* client; //swaghetti
	RakNet::RakPeerInterface* peer;
	RakNet::SystemAddress serverAddress;
    uint64_t playerID;
    std::map<int, std::function<void(CGmpClient*, RakNet::Packet* packet)> >  packetHandlers;
public:
    bool IsConnected;
    BYTE error;

    enum PacketTypes
    {
        WL_PREPARE_TO_JOIN,
        WL_JOIN_TO_GAME,
        WL_INGAME,
        PT_MSG = ID_USER_PACKET_ENUM + 1,
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
        PT_VOICE,
    };

    struct Packet
    {
        PacketTypes type;
        std::string data;
    };

    Network(CGmpClient*);
    ~Network() = default;

    bool Connect(std::string hostAddress, int hostPort);
    void Disconnect();
    int GetPing();
    void Send(const char*, int, PacketPriority, PacketReliability);
    void Receive();

    inline uint64_t GetMyId() { return playerID; }
    void UpdateMyId(uint64_t);
    inline const RakNet::SystemAddress& GetServerAddress() const { return serverAddress; }
private:
    void Init();
    void AddPacketHandlers();
    void HandlePacket(RakNet::Packet*);
};

