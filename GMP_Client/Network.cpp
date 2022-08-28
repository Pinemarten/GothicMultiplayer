#include "Network.h"
#include <exception>
#include "PacketHandlers/Connection.hpp"
#include "PacketHandlers/Game.hpp"

using namespace std;
using namespace RakNet;

#define RAKNET_PASSWORD "YOUR_PASS"

Network::Network(CGmpClient* client)
{
	this->client = client;
	IsConnected = false;
	playerID = -1;
	error = 0;
	peer = RakPeerInterface::GetInstance();
}

void Network::Init()
{
	peer->SetTimeoutTime(1500, UNASSIGNED_SYSTEM_ADDRESS);
	SocketDescriptor socketDescriptor(0, 0);
	socketDescriptor.socketFamily = AF_INET;
	peer->Startup(1, &socketDescriptor, 1);
	peer->SetOccasionalPing(true);
	AddPacketHandlers();
}

void Network::AddPacketHandlers()
{
	packetHandlers[PT_WHOAMI] = Connection::OnWhoami;
	packetHandlers[PT_MAP_NAME] = Game::OnMapName;
	packetHandlers[WL_INGAME] = Game::OnInGame;
	packetHandlers[PT_ACTUAL_STATISTICS] = Game::OnActualStatistics;
	packetHandlers[PT_MAP_ONLY] = Game::OnMapOnly;
	packetHandlers[PT_DODIE] = Game::OnDoDie;
	packetHandlers[PT_RESPAWN] = Game::OnRespawn;
	packetHandlers[PT_CASTSPELL] = Game::OnCastSpell;
	packetHandlers[PT_CASTSPELLONTARGET] = Game::OnCastSpellOnTarget;
	packetHandlers[PT_DROPITEM] = Game::OnDropItem;
	packetHandlers[PT_TAKEITEM] = Game::OnTakeItem;
	packetHandlers[PT_WHISPER] = Game::OnWhisper;
	packetHandlers[PT_MSG] = Game::OnMessage;
	packetHandlers[PT_SRVMSG] = Game::OnServerMessage;
	packetHandlers[PT_RCON] = Game::OnRcon;
	packetHandlers[PT_ALL_OTHERS] = Game::OnAllOthers;
	packetHandlers[PT_JOIN_GAME] = Game::OnJoinGame;
	packetHandlers[PT_YOUR_NAME] = Game::OnYourName;
	packetHandlers[PT_GAME_INFO] = Game::OnGameInfo;
	packetHandlers[PT_LEFT_GAME] = Game::OnLeftGame;
    packetHandlers[PT_VOICE] = Game::OnVoice;
	packetHandlers[ID_DISCONNECTION_NOTIFICATION] = Connection::OnDisconnectOrLostConnection;
	packetHandlers[ID_CONNECTION_LOST] = Connection::OnDisconnectOrLostConnection;
}

bool Network::Connect(string hostAddress, int hostPort)
{
	Init();
	string hostPassword = RAKNET_PASSWORD;
	ConnectionAttemptResult connectionAttemptResult = peer->Connect(hostAddress.c_str(), hostPort, hostPassword.c_str(), hostPassword.length());
	if (connectionAttemptResult != CONNECTION_ATTEMPT_STARTED) { return false; }
	RakNet::Packet* packet = nullptr;
	do { packet = peer->Receive(); } while (!packet);
	MessageID message = packet->data[0];
	serverAddress = packet->systemAddress;
	peer->DeallocatePacket(packet);
	if (message != ID_CONNECTION_REQUEST_ACCEPTED) { error = message; return false; }
	IsConnected = true;
	client->DownloadWBFile();
	client->DownloadClassFile();
	client->DownloadSpawnpointsFile();
	client->PrepareToJoin();
	return true;
}

void Network::Disconnect()
{
	IsConnected = false;
	peer->CloseConnection(serverAddress, true, 11, IMMEDIATE_PRIORITY);
	Sleep(1000);
	RakNet::RakPeerInterface::DestroyInstance(peer);
	peer = nullptr;
}

void Network::Receive()
{
	for (RakNet::Packet* packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive()) {
		HandlePacket(packet);
	}
}

void Network::HandlePacket(RakNet::Packet* packet)
{
	try {
		packetHandlers[(int)packet->data[0]](client, packet);
	}
	catch (exception& e) {
		error = packet->data[0];
	}
}

void Network::Send(const char* packet, int length, PacketPriority priority, PacketReliability reliable)
{
	peer->Send(packet, length, priority, reliable, 0, serverAddress, false);
}

int Network::GetPing()
{
	return peer->GetAveragePing(peer->GetSystemAddressFromIndex(0));
}

void Network::UpdateMyId(uint64_t id)
{
	playerID = id;
	if (LocalPlayer) {
		LocalPlayer->id = id;
	}
}