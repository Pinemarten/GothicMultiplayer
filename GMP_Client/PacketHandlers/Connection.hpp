#pragma once
#include "RakNetTypes.h"
#include "../CGmpClient.h"
#include "../CChat.h"
#include "CLocalPlayer.h"

extern CLocalPlayer* LocalPlayer;
namespace Connection {
	void OnWhoami(CGmpClient* client, RakNet::Packet* packet) {
		uint64_t playerID;
		memcpy(&playerID, packet->data + 1, sizeof(uint64_t));
		client->network->UpdateMyId(playerID);
	}

	void OnDisconnectOrLostConnection(CGmpClient* client, RakNet::Packet* packet)
	{
		client->network->error = packet->data[0];
        client->Disconnect();
		oCNpc::GetHero()->ResetPos(oCNpc::GetHero()->GetPosition());
		client->network->IsConnected = false;
		client->IsInGame = false;
		client->IsReadyToJoin = false;
		CChat::GetInstance()->WriteMessage(NORMAL, false, zCOLOR(255, 0, 0, 255), "%s", (*client->lang)[CLanguage::DISCONNECTED].ToChar());
	}
}