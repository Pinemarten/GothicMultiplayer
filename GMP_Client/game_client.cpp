
/*
MIT License

Copyright (c) 2022 Gothic Multiplayer Team.

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

#include "game_client.h"

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/ext/std_optional.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
#include <spdlog/spdlog.h>
#include <wincrypt.h>

#include <array>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <glm/glm.hpp>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "CChat.h"
#include "CConfig.h"
#include "CIngame.h"
#include "CLocalPlayer.h"
#include "CSelectClass.h"
#include "HTTPDownloader.h"
#include "Interface.h"
#include "g2Api.h"
#include "net_enums.h"
#include "packets.h"
#include "patch.h"

CConfig *user_config = NULL;
zSTRING CYL = "Choose your language:";
const char *LANG_DIR = ".\\Multiplayer\\Localization\\";
std::vector<zSTRING> vec_choose_lang;
std::vector<std::string> vec_lang_files;
const BYTE GDI_MD5[16] = {0x34, 0x16, 0x98, 0x62, 0x43, 0xB0, 0x3D, 0x36, 0xD6, 0xB0, 0x1A, 0x4D, 0x2B, 0xCB, 0xDB, 0x8B};

float fWRatio, fHRatio;
extern CConfig *user_config;
extern CIngame *global_ingame;
extern CLocalPlayer *LocalPlayer;
extern zCOLOR AQUA, RED;

using namespace Net;

template <typename TContainer = std::vector<std::uint8_t>, typename Packet>
void SerializeAndSend(Network *network, const Packet &packet, Net::PacketPriority priority, Net::PacketReliability reliable) {
  TContainer buffer;
  auto written_size = bitsery::quickSerialization<bitsery::OutputBufferAdapter<TContainer>>(buffer, packet);
  network->Send(buffer.data(), written_size, priority, reliable);
}

GameClient::GameClient(const char *ip, CLanguage *langPtr)
    : network(new Network(this)),
      voiceCapture(nullptr),
      voicePlayback(nullptr),
      IsReadyToJoin(false),
      lang(langPtr),
      classmgr(nullptr),
      spawnpoint(nullptr),
      IsAdminOrModerator(false),
      IgnoreFirstTimeMessage(true),
      DropItemsAllowed(false),
      IsInGame(false),
      ObserveMode(CObservation::NO_OBSERVATION),
      clientPort(0xDEAD),
      clientHost(ip) {
  srand(static_cast<unsigned int>(time(NULL)));

  // Extract port number from IP address if present
  size_t pos = clientHost.find_last_of(':');
  if (pos != std::string::npos) {
    std::string portStr = clientHost.substr(pos + 1);
    std::istringstream iss(portStr);
    iss >> clientPort;
    clientHost.erase(pos);
  }
}

GameClient::~GameClient() {
  delete classmgr;
  classmgr = nullptr;

  delete spawnpoint;
  spawnpoint = nullptr;

  delete voiceCapture;
  voiceCapture = nullptr;

  delete voicePlayback;
  voicePlayback = nullptr;

  delete network;
  network = nullptr;
  IsInGame = false;
}

bool GameClient::Connect() {
  if (network->Connect(clientHost, clientPort)) {
    this->voiceCapture = new VoiceCapture();
    this->voiceCapture->StartCapture();
    this->voicePlayback = new VoicePlayback();
    this->voicePlayback->StartPlayback();
    return true;
  }
  return false;
}

void GameClient::PrepareToJoin() {
  std::uint8_t val[2] = {};
  val[0] = PT_WHOAMI;
  network->Send(val, 1, IMMEDIATE_PRIORITY, RELIABLE);
  val[0] = PT_MAP_NAME;
  network->Send(val, 1, IMMEDIATE_PRIORITY, RELIABLE);
  IsReadyToJoin = true;
}

string GameClient::GetServerAddresForHTTPDownloader() {
  auto address = network->GetServerIp() + ":" + std::to_string(network->GetServerPort() + 1);
  return address;
}

void GameClient::DownloadWBFile() {
  auto content = HTTPDownloader::GetWBFile(GetServerAddresForHTTPDownloader());
  static const std::filesystem::path path = ".\\Multiplayer\\Data\\";

  auto serverWbFile = path / (network->GetServerIp() + "_" + std::to_string(network->GetServerPort()));

  if (content == "EMPTY") {
    std::filesystem::remove(serverWbFile);
    return;
  }

  std::ofstream wbFile(serverWbFile, std::ios::binary);
  if (wbFile) {
    wbFile.write(content.data(), content.length());
  }
}

void GameClient::DownloadClassFile() {
  string content = HTTPDownloader::GetClassFile(GetServerAddresForHTTPDownloader());
  if (content.compare("EMPTY") == 0) {
    return;
  }
  this->classmgr = new CHeroClass(content.c_str(), content.length());
}

void GameClient::DownloadSpawnpointsFile() {
  string content = HTTPDownloader::GetSpawnpointsFile(GetServerAddresForHTTPDownloader());
  if (content.compare("EMPTY") == 0) {
    return;
  }
  this->spawnpoint = new CSpawnPoint(content.c_str());
  size_t randomSpawnpoint = rand() % spawnpoint->GetSize();
  oCNpc::GetHero()->SetPosition((*spawnpoint)[randomSpawnpoint]->x, (*spawnpoint)[randomSpawnpoint]->y, (*spawnpoint)[randomSpawnpoint]->z);
}

void GameClient::RestoreHealth() {
  if (!mp_restore || !IsInGame) {
    return;
  }
  if (last_mp_regen != time(NULL)) {
    last_mp_regen = time(NULL);
    if (oCNpc::GetHero()->GetMaxMana() != oCNpc::GetHero()->GetMana()) {
      oCNpc::GetHero()->SetMana(oCNpc::GetHero()->GetMana() + mp_restore);
      if (oCNpc::GetHero()->GetMana() > oCNpc::GetHero()->GetMaxMana())
        oCNpc::GetHero()->SetMana(oCNpc::GetHero()->GetMaxMana());
    }
  }
}

void GameClient::HandleNetwork() {
  if (IsConnected()) {
    network->Receive();
  }
}

bool GameClient::IsConnected() {
  return network->IsConnected();
}

zSTRING &GameClient::GetLastError() {
  switch (network->error) {
    case ID_CONNECTION_ATTEMPT_FAILED:
      return (*this->lang)[CLanguage::ERR_CONN_FAIL];
    case ID_ALREADY_CONNECTED:
      return (*this->lang)[CLanguage::ERR_CONN_ALREADY_CONNECTED];
    case ID_NO_FREE_INCOMING_CONNECTIONS:
      return (*this->lang)[CLanguage::ERR_CONN_SRV_FULL];
    case ID_CONNECTION_BANNED:
      return (*this->lang)[CLanguage::ERR_CONN_BANNED];
    case ID_INVALID_PASSWORD:
    case ID_INCOMPATIBLE_PROTOCOL_VERSION:
      return (*this->lang)[CLanguage::ERR_CONN_INCOMP_TECHNIC];
    default:
      return (*this->lang)[CLanguage::ERR_CONN_NO_ERROR];
  }
}

void GameClient::JoinGame(BYTE selected_class) {
  if (IsReadyToJoin) {
    HooksManager::GetInstance()->AddHook(HT_RENDER, (DWORD)InterfaceLoop, false);
    HooksManager::GetInstance()->RemoveHook(HT_RENDER, (DWORD)CSelectClass::Loop);

    JoinGamePacket packet;
    packet.packet_type = PT_JOIN_GAME;
    packet.selected_class = selected_class;

    auto pos = oCNpc::GetHero()->GetPosition();
    packet.position.x = pos.x;
    packet.position.y = pos.y;
    packet.position.z = pos.z;

    packet.normal.x = oCNpc::GetHero()->GetAngleNX();
    packet.normal.y = oCNpc::GetHero()->GetAngleNY();
    packet.normal.z = oCNpc::GetHero()->GetAngleNZ();

    oCItem *itPtr = oCNpc::GetHero()->GetLeftHand();
    if (itPtr) {
      packet.left_hand_item_instance = itPtr->GetInstance();
    }
    itPtr = oCNpc::GetHero()->GetRightHand();
    if (itPtr) {
      packet.right_hand_item_instance = itPtr->GetInstance();
    }
    itPtr = oCNpc::GetHero()->GetEquippedArmor();
    if (itPtr) {
      packet.equipped_armor_instance = itPtr->GetInstance();
    }
    packet.animation = 265;  // TODO: get current animation (or remove it?)
    packet.head_model = user_config->headmodel;
    packet.skin_texture = user_config->skintexture;
    packet.face_texture = user_config->facetexture;
    packet.walk_style = user_config->walkstyle;
    packet.player_name = oCNpc::GetHero()->GetName().ToChar();

    SerializeAndSend(network, packet, IMMEDIATE_PRIORITY, RELIABLE_ORDERED);

    CIngame *g = new CIngame(lang);
    if (!LocalPlayer)
      new CLocalPlayer();
    LocalPlayer->id = network->GetMyId();
    LocalPlayer->enable = TRUE;
    LocalPlayer->SetNpc(oCNpc::GetHero());
    LocalPlayer->hp = static_cast<short>(LocalPlayer->GetHealth());
    LocalPlayer->update_hp_packet = 0;
    LocalPlayer->npc->SetMovLock(0);
    LocalPlayer->char_class = selected_class;
    this->player.push_back(LocalPlayer);
    this->HeroLastHp = oCNpc::GetHero()->GetHealth();
  }
}

void GameClient::SendMessage(const char *msg) {
  MessagePacket packet;
  packet.packet_type = PT_MSG;
  packet.message = msg;
  SerializeAndSend(network, packet, MEDIUM_PRIORITY, RELIABLE);
}

void GameClient::SendWhisper(const char *player_name, const char *msg) {
  bool found = false;
  size_t i;
  size_t length = strlen(player_name);
  for (i = 0; i < this->player.size(); i++) {
    if (this->player[i]->GetNameLength() == length) {
      if (!memcmp(this->player[i]->GetName(), player_name, length)) {
        found = true;
        break;
      }
    }
  }

  if (found) {
    MessagePacket packet;
    packet.packet_type = PT_WHISPER;
    packet.message = msg;
    packet.recipient = player[i]->id;

    SerializeAndSend(network, packet, HIGH_PRIORITY, RELIABLE_ORDERED);
  }
}

void GameClient::SendCommand(const char *msg) {
  MessagePacket packet;
  packet.packet_type = PT_COMMAND;
  packet.message = msg;
  SerializeAndSend(network, packet, HIGH_PRIORITY, RELIABLE_ORDERED);
}

void GameClient::SendCastSpell(oCNpc *Target, short SpellId) {
  CastSpellPacket packet;
  packet.spell_id = static_cast<uint16_t>(SpellId);
  packet.packet_type = PT_CASTSPELL;

  if (Target) {
    for (int i = 0; i < (int)player.size(); i++) {
      if (player[i]->npc == Target) {
        packet.target_id = player[i]->id;
        packet.packet_type = PT_CASTSPELLONTARGET;
        break;
      }
    }
  }

  SerializeAndSend(network, packet, HIGH_PRIORITY, RELIABLE);
}

void GameClient::SendDropItem(short instance, short amount) {
  DropItemPacket packet;
  packet.packet_type = PT_DROPITEM;
  packet.item_instance = instance;
  packet.item_amount = amount;
  SerializeAndSend(network, packet, HIGH_PRIORITY, RELIABLE);
}

void GameClient::SendTakeItem(short instance) {
  TakeItemPacket packet;
  packet.packet_type = PT_TAKEITEM;
  packet.item_instance = instance;
  SerializeAndSend(network, packet, HIGH_PRIORITY, RELIABLE);
}

glm::vec3 Vec3ToGlmVec3(const zVEC3 &vec) {
  return glm::vec3(vec.x, vec.y, vec.z);
}

std::uint8_t GetHeadDirectionByte(oCNpc *Hero) {
  zVEC2 HeadVar = Hero->GetAnictrl()->GetLookAtPos();
  if (HeadVar.x == 0)
    return static_cast<uint8_t>(CPlayer::HEAD_LEFT);
  else if (HeadVar.x == 1)
    return static_cast<uint8_t>(CPlayer::HEAD_RIGHT);
  else if (HeadVar.y == 0)
    return static_cast<uint8_t>(CPlayer::HEAD_UP);
  else if (HeadVar.y == 1)
    return static_cast<uint8_t>(CPlayer::HEAD_DOWN);

  return 0;
}

void GameClient::UpdatePlayerStats(short anim) {
  oCNpc *Hero = oCNpc::GetHero();

  PlayerStateUpdatePacket packet;
  packet.packet_type = PT_ACTUAL_STATISTICS;
  packet.state.position = Vec3ToGlmVec3(Hero->GetPosition());
  packet.state.nrot = glm::vec3(Hero->GetAngleNX(), Hero->GetAngleNY(), Hero->GetAngleNZ());
  packet.state.left_hand_item_instance = Hero->GetLeftHand() ? static_cast<short>(Hero->GetLeftHand()->GetInstance()) : 0;
  packet.state.right_hand_item_instance = Hero->GetRightHand() ? static_cast<short>(Hero->GetRightHand()->GetInstance()) : 0;
  packet.state.equipped_armor_instance = Hero->GetEquippedArmor() ? static_cast<short>(Hero->GetEquippedArmor()->GetInstance()) : 0;
  packet.state.animation = anim;
  packet.state.mana_points = static_cast<short>(Hero->GetMana());
  packet.state.weapon_mode = static_cast<uint8_t>(Hero->GetWeaponMode());
  packet.state.active_spell_nr = Hero->GetActiveSpellNr() > 0 ? static_cast<uint8_t>(Hero->GetActiveSpellNr()) : 0;
  packet.state.head_direction = GetHeadDirectionByte(Hero);
  packet.state.melee_weapon_instance = Hero->GetEquippedMeleeWeapon() ? static_cast<short>(Hero->GetEquippedMeleeWeapon()->GetInstance()) : 0;
  packet.state.ranged_weapon_instance = Hero->GetEquippedRangedWeapon() ? static_cast<short>(Hero->GetEquippedRangedWeapon()->GetInstance()) : 0;

  SerializeAndSend(network, packet, IMMEDIATE_PRIORITY, RELIABLE_ORDERED);
}

void GameClient::SendHPDiff(size_t who, short diff) {
  if (who < this->player.size()) {
    HPDiffPacket packet;
    packet.packet_type = PT_HP_DIFF;
    packet.player_id = this->player[who]->id;
    packet.hp_difference = diff;

    SerializeAndSend(network, packet, IMMEDIATE_PRIORITY, RELIABLE);
  }
}

void GameClient::SendVoice() {
  int audioChannels = voiceCapture->GetNumberOfChannels();
  char *voiceBuffer = new char[480 * sizeof(float) * audioChannels * 4096];  // TODO: correct size
  ZeroMemory(voiceBuffer, 480 * sizeof(float) * audioChannels * 4096);
  int size;
  if (voiceCapture->GetAndFlushVoiceBuffer(voiceBuffer, size) && zCInput::GetInput()->KeyPressed(KEY_K)) {
    VoicePacket packet;
    packet.packet_type = PT_VOICE;
    packet.voice_data_size = static_cast<std::uint32_t>(size);
    packet.voice_data.assign(voiceBuffer, voiceBuffer + size);

    SerializeAndSend(network, packet, IMMEDIATE_PRIORITY, UNRELIABLE);
  }
  delete[] voiceBuffer;
}

void GameClient::SyncGameTime() {
  BYTE data[2] = {PT_GAME_INFO, 0};
  network->Send((char *)data, 1, IMMEDIATE_PRIORITY, RELIABLE);
}

void GameClient::Disconnect() {
  if (network->IsConnected()) {
    IsInGame = false;
    IgnoreFirstTimeMessage = true;
    global_ingame->IgnoreFirstSync = true;
    LocalPlayer->SetNpcType(CPlayer::NPC_HUMAN);
    network->Disconnect();
    delete LocalPlayer;
    CPlayer::DeleteAllPlayers();
    if (VobsWorldBuilderMap.size() > 0) {
      for (int i = 0; i < (int)VobsWorldBuilderMap.size(); i++) {
        VobsWorldBuilderMap[i].Vob->RemoveVobFromWorld();
      }
      VobsWorldBuilderMap.clear();
    }
    CChat::GetInstance()->ClearChat();
    global_ingame->WhisperingTo.clear();
    oCNpc::GetHero()->SetWeaponMode(NPC_WEAPON_NONE);
  }
  if (this->classmgr) {
    delete this->classmgr;
    this->classmgr = NULL;
  }
  if (this->spawnpoint) {
    delete this->spawnpoint;
    this->spawnpoint = NULL;
  }
  if (this->voiceCapture) {
    delete this->voiceCapture;
    this->voiceCapture = nullptr;
  }
  if (this->voicePlayback) {
    delete this->voicePlayback;
    this->voicePlayback = nullptr;
  }
}