
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

#include "game_server.h"

#include <httplib.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <dylib.hpp>
#include <fstream>
#include <future>
#include <stack>
#include <string>

#include "HTTPServer.h"
#include "git.h"
#include "gothic_clock.h"
#include "net_enums.h"
#include "platform_depend.h"
#include "server_events.h"
#include "shared/event.h"
#include "shared/math.h"
#include "znet_server.h"

Net::NetServer* g_net_server = nullptr;

const char* lobbyAddress = "http://lobby.your-site.com";
const char* lobbyFile = "add.php";

extern const char* SAY;
extern const char* KICK;
extern const char* BAN;
extern const char* BAN_IP;
extern const char* UNBAN;
extern const char* SAVEBANS;
extern const char* MUTE;
extern const char* SET_TIME;
extern const char* KILL;
extern const char* LOOP_MSG;

// admin only
const char* MOD_ADD = "modadd ";
const char* MOD_SET = "modset ";  // ustawia hasło dla określonego moderatora
const char* MOD_DEL = "moddel ";

const char* WTF = "Dude, I dont understand you.";
const char* OK = "OK!";
const char* AG = "Access granted!";
const char* PDE = "Player doesn't exist!";
const char* IPNOTBANNED = "Following IP isn't banned!";
const char* IPISBANNED = "Following IP is already banned!";
const char* INVALIDPARAMETER = "Invalid command parameter!";
#define MAX_KILL_TXT 3
const char* KILLED[MAX_KILL_TXT] = {"K.O.", "R.I.P.", "FATALITY"};
std::atomic<bool> g_is_server_running = true;

using namespace Net;

namespace {

void LoadNetworkLibrary() {
  try {
    static dylib lib("znet_server");
    auto create_net_server_func = lib.get_function<Net::NetServer*()>("CreateNetServer");
    g_net_server = create_net_server_func();
  } catch (std::exception& ex) {
    SPDLOG_ERROR("LoadNetworkLibrary error: {}", ex.what());
    std::abort();
  }
}

void InitializeLogger(const Config& config) {
  if (!config.Get<bool>("log_to_stdout")) {
    spdlog::default_logger()->sinks().clear();
  }

  auto log_file = config.Get<std::string>("log_file");
  spdlog::default_logger()->sinks().push_back(
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::move(log_file), false));

  auto log_level = config.Get<std::string>("log_level");
  spdlog::set_level(spdlog::level::from_str(log_level));
  spdlog::flush_on(spdlog::level::debug);
}
}  // namespace

GameServer::GameServer() {
  InitializeLogger(config_);
  SPDLOG_INFO("|-----------------------------------|");
  if (git_IsPopulated()) {
    SPDLOG_INFO("Gothic Multiplayer Classic {}", git_Describe());
    SPDLOG_INFO("Commit date: {}", git_CommitDate());
  } else {
    SPDLOG_INFO("Gothic Multiplayer Classic");
  }
  SPDLOG_INFO("GMP Team (2011) | GMPC Team (2022)");
  SPDLOG_INFO("|-----------------------------------|");
  config_.LogConfigValues();
  SPDLOG_INFO("|-----------------------------------|");
  g_server = this;

  // Register server-side events.
  EventManager::Instance().RegisterEvent(kEventOnPlayerConnectName);
  EventManager::Instance().RegisterEvent(kEventOnPlayerMessageName);
}

GameServer::~GameServer() {
  g_is_server_running = false;
  delete script;
  g_net_server->RemovePacketHandler(*this);
  // server->Shutdown(300);
  g_server = nullptr;
}

bool GameServer::Init() {
  LoadNetworkLibrary();
  g_net_server->AddPacketHandler(*this);
#ifndef WIN32
  if (config_.Get<bool>("daemon")) {
    System::MakeMeDaemon(false);
  }
#endif
  this->spam_time = time(NULL) + 10;
  character_definition_manager_ = std::make_unique<CharacterDefinitionManager>();
  auto character_definitions_file = config_.Get<std::string>("character_definitions_file");
  if (!character_definitions_file.empty()) {
    character_definition_manager_->Load(character_definitions_file);
  } else {
    SPDLOG_WARN("Character definitions file not specified. No character definitions will be loaded.");
  }

  auto slots = config_.Get<std::int32_t>("slots");
  allow_modification = config_.Get<bool>("allow_modification");
  loop_msg = config_.Get<std::string>("message_of_the_day");

  auto port = config_.Get<std::int32_t>("port");

  if (!g_net_server->Start(port, slots)) {
    return false;
  }

  LoadBanList();
  g_is_server_running = true;

  clock_ = std::make_unique<GothicClock>(config_.Get<GothicClock::Time>("game_time"));
  if (IsPublic()) {
    public_list_http_thread_future_ = std::async(&GameServer::AddToPublicListHTTP, this);
  }
  http_server_ = std::make_unique<HTTPServer>();
  http_server_->Start(port);
  this->last_stand_timer = 0;

  SPDLOG_INFO("");
  script = new Script(config_.Get<std::vector<std::string>>("scripts"));
  SPDLOG_INFO("|-----------------------------------|");
  SPDLOG_INFO("GMP Classic Server initialized successfully!");
  return true;
}

void GameServer::Run() {
  g_net_server->Pulse();
  clock_->RunClock();
}

bool GameServer::HandlePacket(Net::PlayerId playerId, unsigned char* data, std::uint32_t size) {
  Packet p{data, size, playerId};

  unsigned char packetIdentifier = GetPacketIdentifier(p);
  switch (packetIdentifier) {
    case ID_DISCONNECTION_NOTIFICATION:
      SendDisconnectionInfo(p.id.guid);
      DeleteFromPlayerList(p.id);
      SPDLOG_INFO("{} disconnected. Still connected {} users.", g_net_server->GetPlayerIp(p.id), players.size());
      break;
    case ID_NEW_INCOMING_CONNECTION:  // nadchodzące połączenie zaakceptowane | można dodać guid gracza do listy graczy;
    {
      sPlayer pl;
      pl.has_admin = 0;
      pl.moderator_passwd = 0;
      pl.admin_passwd = 0;
      pl.is_ingame = 0;
      pl.passed_crc_test = 0;
      pl.id = p.id;
      pl.mute = 0;
      players[p.id.guid] = std::move(pl);
    }
      SPDLOG_INFO("ID_NEW_INCOMING_CONNECTION from {} with GUID {}. Now we have {} connected users.",
                  g_net_server->GetPlayerIp(p.id), p.id.guid, players.size());
      break;
    case ID_INCOMPATIBLE_PROTOCOL_VERSION:
      SPDLOG_WARN("ID_INCOMPATIBLE_PROTOCOL_VERSION");
      break;
    case ID_CONNECTION_LOST:
      SendDisconnectionInfo(p.id.guid);
      DeleteFromPlayerList(p.id);
      SPDLOG_WARN("Connection lost from {}. Still connected {} users.", g_net_server->GetPlayerIp(p.id),
                  players.size());
      break;
    case PT_REQUEST_FILE_LENGTH:
    case PT_REQUEST_FILE_PART:
      break;
    case PT_WHOAMI:  // zwraca id gracza(czyli jego guid)
    {
      unsigned char val[9];
      val[0] = PT_WHOAMI;
      memcpy(val + 1, &p.id.guid, sizeof(uint64_t));
      g_net_server->Send((const char*)val, 9, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p.id);
    } break;
    case PT_JOIN_GAME:
      SomeoneJoinGame(p);
      break;
    case PT_ACTUAL_STATISTICS:  // dostarcza nam informacji o sobie
      HandlePlayerUpdate(p);
      break;
    case PT_CHECKSUM:
      if (!this->allow_modification) {
        auto map_md5 = config_.Get<std::string>("map_md5");
        if ((!memcmp(p.data + 2, map_md5.data(), 16)) && (!p.data[1])) {
          auto player = GetPlayerById(p.id.guid);
          if (player) {
            player->get().passed_crc_test = 1;
            unsigned char val[2] = {PT_CHECKSUM, 0xFF};
            g_net_server->Send((const char*)val, 2, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, p.id);
          }
        } else {
          players.erase(p.id.guid);
          g_net_server->AddToBanList(p.id, 3600000);
        }
      }
      break;
    case PT_HP_DIFF:
      MakeHPDiff(p);
      break;
    case PT_MSG:
      HandleNormalMsg(p);
      break;
    case PT_CASTSPELL:
      HandleCastSpell(p, false);
      break;
    case PT_CASTSPELLONTARGET:
      HandleCastSpell(p, true);
      break;
    case PT_DROPITEM:
      HandleDropItem(p);
      break;
    case PT_TAKEITEM:
      HandleTakeItem(p);
      break;
    case PT_WHISPER:
      HandleWhisp(p);
      break;
    case PT_RCON:
      HandleRMConsole(p);
      break;
    case PT_GAME_INFO:  // na razie tylko czas
      HandleGameInfo(p);
      break;
    case PT_MAP_NAME:
      HandleMapNameReq(p);
      break;
    case PT_VOICE:
      HandleVoice(p);
      break;
    default:
      SPDLOG_WARN("(S)He or it try to do something strange. It's packet ID: {}", packetIdentifier);
      break;
  }
  return true;
}

bool GameServer::Receive() {
  g_net_server->Pulse();
  return true;
}

unsigned char GameServer::GetPacketIdentifier(const Packet& p) {
  if ((unsigned char)p.data[0] == ID_TIMESTAMP) {
    return (unsigned char)p.data[1 + sizeof(std::uint32_t)];
  } else
    return (unsigned char)p.data[0];
}

void GameServer::LoadBanList() {
  std::string szTmp;
  szTmp.reserve(32);
  std::ifstream ifs("bans.txt");
  if (ifs.is_open()) {
    while (1) {
      ifs.getline((char*)szTmp.data(), 32);
      if (ifs.eof())
        break;
      if (szTmp.length() >= 7) {
        this->ban_list.push_back(szTmp.c_str());
      }
    }
    szTmp.clear();
    ifs.close();
  } else {
    SPDLOG_WARN("bans.txt which contains active IP bans not exist");
    return;
  }
  if (!this->ban_list.empty()) {
    for (size_t i = 0; i < this->ban_list.size(); i++) {
      g_net_server->AddToBanList(this->ban_list[i].c_str(), 0);
    }
  }
  SPDLOG_INFO("Active bans loaded.");
}

void GameServer::SaveBanList() {
  if (!this->ban_list.empty()) {
    std::ofstream ofs("bans.txt");
    if (!ofs.is_open()) {
      SPDLOG_ERROR("Could not save active bans to file bans.txt!");
      return;
    }
    for (size_t i = 0; i < this->ban_list.size(); i++) {
      ofs << this->ban_list[i].c_str() << "\n";
    }
    ofs.clear();
  }
  SPDLOG_INFO("Bans written to bans.txt.");
}

void GameServer::DeleteFromPlayerList(Net::PlayerId id) {
  players.erase(id.guid);
}

std::optional<std::reference_wrapper<GameServer::sPlayer>> GameServer::GetPlayerById(std::uint64_t id) {
  auto it = players.find(id);
  if (it != players.end())
    return std::ref(it->second);
  return std::nullopt;
}

void GameServer::SomeoneJoinGame(Packet p) {
  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt) {
    SPDLOG_WARN("Someone tried to join game, but he is not on the player list, ID {}!", p.id.guid);
    return;
  }
  auto& player = player_opt.value().get();

  if (!allow_modification) {
    if (!player.passed_crc_test) {
      players.erase(p.id.guid);
      g_net_server->AddToBanList(p.id, 3600000);  // i dorzucamy banana na 1h
      return;
    }
  }
  player.tod = 0;
  player.char_class = p.data[1];
  player.health = character_definition_manager_->GetCharacterDefinition(p.data[1]).abilities[HP];
  memcpy(player.pos, p.data + 2, 12);
  memcpy(player.nrot, p.data + 14, 12);
  memcpy(&player.left_hand, p.data + 26, 2);
  memcpy(&player.right_hand, p.data + 28, 2);
  memcpy(&player.armor, p.data + 30, 2);
  memcpy(&player.animation, p.data + 32, 2);
  player.head = p.data[34];
  player.skin = p.data[35];
  player.body = p.data[36];
  player.walkstyle = p.data[37];
  player.name = (const char*)(p.data + 38);

  unsigned char nick_packet[128];
  memcpy(nick_packet + 1, player.name.c_str(), player.name.length() + 1);
  nick_packet[0] = PT_YOUR_NAME;
  g_net_server->Send(nick_packet, 2 + (int)player.name.length(), IMMEDIATE_PRIORITY, RELIABLE, 0, p.id);
  std::string inform_packet;
  inform_packet.reserve(1024);
  char* szTmpPtr = (char*)inform_packet.c_str();
  szTmpPtr[0] = p.data[0];
  memcpy((char*)inform_packet.data() + (1 + sizeof(uint64_t)), p.data + 1, p.length - 1);
  memcpy((char*)inform_packet.data() + 1, &p.id.guid, sizeof(uint64_t));

  // Informing other players about new player
  memcpy((char*)inform_packet.data() + (38 + sizeof(uint64_t)), player.name.c_str(), player.name.length());
  for (size_t i = 0; i < players.size(); i++) {
    if (players[i].is_ingame) {
      g_net_server->Send((unsigned char*)inform_packet.data(), 40 + (int)player.name.length() + sizeof(uint64_t),
                         IMMEDIATE_PRIORITY, RELIABLE, 3, players[i].id);
    }
  }
  // generowanie i słanie pakietów o graczach(max 10 graczy per pakiet)
  unsigned char it = 0;
  size_t szIt = 1;
  for (const auto& [id, existing_player] : players) {
    if (existing_player.is_ingame) {
      it++;
      memcpy((char*)inform_packet.data() + szIt, &existing_player.id.guid, (sizeof(uint64_t)));
      szIt += sizeof(uint64_t);
      *(char*)(inform_packet.data() + szIt) = existing_player.char_class;
      szIt++;
      memcpy((char*)inform_packet.data() + szIt, existing_player.pos, 12);
      szIt += 12;
      memcpy((char*)inform_packet.data() + szIt, &existing_player.left_hand, 2);
      szIt += 2;
      memcpy((char*)inform_packet.data() + szIt, &existing_player.right_hand, 2);
      szIt += 2;
      memcpy((char*)inform_packet.data() + szIt, &existing_player.armor, 2);
      szIt += 2;
      *(char*)(inform_packet.data() + szIt) = existing_player.head;
      szIt++;
      *(char*)(inform_packet.data() + szIt) = existing_player.skin;
      szIt++;
      *(char*)(inform_packet.data() + szIt) = existing_player.body;
      szIt++;
      *(char*)(inform_packet.data() + szIt) = existing_player.walkstyle;
      szIt++;
      memcpy((char*)inform_packet.data() + szIt, existing_player.name.c_str(), existing_player.name.length() + 1);
      szIt += existing_player.name.length() + 1;
    }
    if (it == 10) {
      *(unsigned char*)inform_packet.data() = PT_ALL_OTHERS;
      g_net_server->Send((unsigned char*)inform_packet.data(), (int)szIt, IMMEDIATE_PRIORITY, RELIABLE, 2, p.id);
      it = 0;
      szIt = 1;
    }
  }
  if (it > 0) {
    *(unsigned char*)(inform_packet.data()) = PT_ALL_OTHERS;
    g_net_server->Send((unsigned char*)inform_packet.data(), (int)szIt, IMMEDIATE_PRIORITY, RELIABLE, 2, p.id);
    it = 0;
    szIt = 1;
  }
  player.is_ingame = 1;

  // join
  EventManager::Instance().TriggerEvent(kEventOnPlayerConnectName, p.id.guid);
}

void GameServer::HandlePlayerUpdate(Packet p) {
  constexpr double kRadiusSquared = 5000.0 * 5000.0;

  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value()) {
    return;
  }
  auto& updated_player = player_opt.value().get();

  memcpy(updated_player.pos, p.data + 1, 12);
  memcpy(updated_player.nrot, p.data + 13, 12);
  memcpy(&updated_player.left_hand, p.data + 25, 2);
  memcpy(&updated_player.right_hand, p.data + 27, 2);
  memcpy(&updated_player.armor, p.data + 29, 2);
  memcpy(&updated_player.animation, p.data + 31, 2);
  memcpy(&updated_player.mana, p.data + 33, 2);
  updated_player.figth_pos = p.data[35];
  updated_player.spellhand = p.data[36];
  updated_player.headstate = p.data[37];
  memcpy(&updated_player.rangedeq, p.data + 38, 2);
  memcpy(&updated_player.meleeeq, p.data + 40, 2);

  // generowanie wiadomości do słania
  size_t szIt[2] = {1, 1025};
  unsigned char it[2] = {0, 0};
  std::string msg;
  msg.reserve(2048);
  for (const auto& [id, existing_player] : players) {
    if (existing_player.is_ingame) {
      if (p.id != existing_player.id) {
        if (IsWithinRadius(updated_player.pos, existing_player.pos, kRadiusSquared)) {
          it[0]++;
          memcpy((char*)msg.data() + szIt[0], &existing_player.id.guid, sizeof(uint64_t));
          szIt[0] += sizeof(uint64_t);
          *(char*)(msg.data() + szIt[0]) = existing_player.flags;
          szIt[0]++;
          memcpy((char*)msg.data() + szIt[0], existing_player.pos, 12);
          szIt[0] += 12;
          memcpy((char*)msg.data() + szIt[0], existing_player.nrot, 12);
          szIt[0] += 12;
          memcpy((char*)msg.data() + szIt[0], &existing_player.left_hand, 2);
          szIt[0] += 2;
          memcpy((char*)msg.data() + szIt[0], &existing_player.right_hand, 2);
          szIt[0] += 2;
          memcpy((char*)msg.data() + szIt[0], &existing_player.armor, 2);
          szIt[0] += 2;
          memcpy((char*)msg.data() + szIt[0], &existing_player.animation, 2);
          szIt[0] += 2;
          memcpy((char*)msg.data() + szIt[0], &existing_player.health, 2);
          szIt[0] += 2;
          memcpy((char*)msg.data() + szIt[0], &existing_player.mana, 2);
          szIt[0] += 2;
          *(char*)(msg.data() + szIt[0]) = existing_player.spellhand;
          szIt[0]++;
          *(char*)(msg.data() + szIt[0]) = existing_player.figth_pos;
          szIt[0]++;
          *(char*)(msg.data() + szIt[0]) = existing_player.headstate;
          szIt[0]++;
          memcpy((char*)msg.data() + szIt[0], &existing_player.rangedeq, 2);
          szIt[0] += 2;
          memcpy((char*)msg.data() + szIt[0], &existing_player.meleeeq, 2);
          szIt[0] += 2;
        } else {  // ktoś nieistotny
          it[1]++;
          memcpy((char*)msg.data() + szIt[1], &existing_player.id.guid, sizeof(uint64_t));
          szIt[1] += sizeof(uint64_t);
          memcpy((char*)msg.data() + szIt[1], &existing_player.pos[0], 4);
          szIt[1] += 4;  // float x,y
          memcpy((char*)msg.data() + szIt[1], &existing_player.pos[2], 4);
          szIt[1] += 4;
        }
      } else {  // to jesteśmy my, starczy tylko info o hp
        it[0]++;
        memcpy((char*)msg.data() + szIt[0], &p.id.guid, sizeof(uint64_t));
        szIt[0] += sizeof(uint64_t);
        memcpy((char*)msg.data() + szIt[0], &updated_player.health, 2);
        szIt[0] += 2;
      }
      if (it[0] == 10) {
        *(unsigned char*)(msg.data()) = PT_ACTUAL_STATISTICS;
        g_net_server->Send((unsigned char*)msg.data(), (int)szIt[0], IMMEDIATE_PRIORITY, UNRELIABLE, 0, p.id);
        it[0] = 0;
        szIt[0] = 1;
      }
      if (it[1] == 60) {
        *(unsigned char*)(msg.data() + 1024) = PT_MAP_ONLY;
        g_net_server->Send((unsigned char*)msg.data() + 1024, (int)szIt[1] - 1024, IMMEDIATE_PRIORITY, UNRELIABLE, 0,
                           p.id);
        it[1] = 0;
        szIt[1] = 1025;
      }
    }
  }
  if (it[0] > 0) {
    *(unsigned char*)(msg.data()) = PT_ACTUAL_STATISTICS;
    g_net_server->Send((unsigned char*)msg.data(), (int)szIt[0], IMMEDIATE_PRIORITY, UNRELIABLE, 0, p.id);
    it[0] = 0;
  }
  if (it[1] > 0) {
    *(unsigned char*)(msg.data() + 1024) = PT_MAP_ONLY;
    g_net_server->Send((unsigned char*)msg.data() + 1024, (int)szIt[1] - 1024, IMMEDIATE_PRIORITY, UNRELIABLE, 32,
                       p.id);
    it[1] = 0;
  }
}

void GameServer::MakeHPDiff(Packet p) {
  uint64_t player_id;
  short diffed_hp;

  auto attacker_opt = GetPlayerById(p.id.guid);
  if (!attacker_opt.has_value())
    return;

  auto& attacker = attacker_opt.value().get();

  if (attacker.is_ingame) {
    memcpy(&player_id, p.data + 1, sizeof(uint64_t));
    memcpy(&diffed_hp, p.data + (1 + sizeof(uint64_t)), 2);

    auto victim_opt = GetPlayerById(player_id);
    if (!victim_opt.has_value())
      return;
    auto& victim = victim_opt.value().get();

    if ((victim.health <= 0) || (victim.tod))
      return;  // just ignore
    if (player_id == p.id.guid) {
      if (victim.health)
        victim.health += diffed_hp;
      if (victim.health <= 0) {
        victim.health = 0;
        victim.tod = time(NULL);
        SendDeathInfo(victim.id.guid);
      }
      if (victim.health > character_definition_manager_->GetCharacterDefinition(victim.char_class).abilities[HP]) {
        victim.health = character_definition_manager_->GetCharacterDefinition(victim.char_class).abilities[HP];
      }
    } else {
      if (config_.Get<bool>("be_unconcious_before_dead")) {
        switch (attacker.figth_pos) {
          case 1:
          case 3:
          case 4:  // od tych broni nasz zawodnik na pewno nie padnie
            if (victim.flags & PL_UNCONCIOUS) {
              victim.flags &= 0xFE;
              victim.tod = time(NULL);
              victim.health = 0;
              SendDeathInfo(victim.id.guid);
            } else {
              victim.health += diffed_hp;
              if (victim.health <= 1) {
                victim.health = 1;
                victim.flags |= PL_UNCONCIOUS;
              }
            }
            break;
          default:
            if (victim.health > 0) {
              victim.health += diffed_hp;
              if (victim.health < 0)
                victim.health = 0;
              if (victim.health <= 0) {
                victim.tod = time(NULL);
                SendDeathInfo(victim.id.guid);
              }
            }
            break;
        }
      } else {
        victim.health += diffed_hp;
        if (victim.health == 1) {
          victim.health = 0;
          victim.tod = time(NULL);
          SendDeathInfo(victim.id.guid);
        }
        if ((victim.health == 0) && (!victim.tod)) {
          victim.tod = time(NULL);
          SendDeathInfo(victim.id.guid);
        }
        if (victim.health < 0) {
          victim.health = 0;
          victim.tod = time(NULL);
          SendDeathInfo(victim.id.guid);
        }
        if (victim.health > character_definition_manager_->GetCharacterDefinition(victim.char_class).abilities[HP]) {
          victim.health = character_definition_manager_->GetCharacterDefinition(victim.char_class).abilities[HP];
        }
      }
    }
    if ((victim.health <= 0) && (!victim.tod)) {
      victim.tod = time(NULL);
      SendDeathInfo(victim.id.guid);
    }
  }
}

void GameServer::HandleVoice(Packet p) {
  // TODO: no need to resend player id right now, it won't be needed until we add 3d chat
  std::string data;
  data.reserve(p.length);
  memcpy(data.data(), p.data, p.length);
  for (const auto& [id, existing_player] : players) {
    if (existing_player.is_ingame && existing_player.id.guid != p.id.guid) {
      g_net_server->Send((unsigned char*)data.data(), p.length, IMMEDIATE_PRIORITY, UNRELIABLE, 5, existing_player.id);
    }
  }
}

void GameServer::HandleNormalMsg(Packet p) {
  std::string szMsg;

  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame || player_opt.value().get().mute)
    return;

  szMsg.reserve(p.length + sizeof(uint64_t) + 1);

  *((unsigned char*)szMsg.data()) = p.data[0];
  memcpy((char*)szMsg.data() + 1, (const char*)(&p.id.guid), sizeof(uint64_t));  // id
  memcpy((char*)szMsg.data() + sizeof(uint64_t) + 1, p.data + 1, p.length - 1);  // text

  EventManager::Instance().TriggerEvent(kEventOnPlayerMessageName,
                                        OnPlayerMessageEvent{p.id.guid, (const char*)(p.data + 1)});

  for (const auto& [id, existing_player] : players) {
    if (existing_player.is_ingame)
      g_net_server->Send((unsigned char*)szMsg.data(), p.length + sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 5,
                         existing_player.id);
  }

  SPDLOG_INFO("{}:{}", player_opt->get().name, (const char*)(p.data + 1));
}

void GameServer::HandleWhisp(Packet p) {
  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame)
    return;

  uint64_t recipient_id;
  memcpy(&recipient_id, p.data + 1, sizeof(uint64_t));

  auto recipient_opt = GetPlayerById(recipient_id);
  if (!recipient_opt.has_value())
    return;
  auto& recipient = recipient_opt.value().get();

  std::string szMsg;
  szMsg.reserve(p.length + 1 + sizeof(uint64_t));
  *((char*)szMsg.data()) = p.data[0];
  memcpy((char*)(szMsg.data() + 1), &p.id.guid, sizeof(uint64_t));               // id
  memcpy((char*)szMsg.data() + 1 + sizeof(uint64_t), p.data + 1, p.length - 1);  // text

  g_net_server->Send((unsigned char*)szMsg.data(), p.length + 1 + sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 6,
                     p.id);
  g_net_server->Send((unsigned char*)szMsg.data(), p.length + 1 + sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 6,
                     recipient.id);

  SPDLOG_INFO("({} WHISPERS TO {}) {}", player_opt->get().name, recipient.name,
              (const char*)(p.data + 1 + sizeof(uint64_t)));
}

void GameServer::HandleCastSpell(Packet p, bool target) {
  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame)
    return;

  std::string szMsg;
  szMsg.reserve(p.length + sizeof(uint64_t));

  if (!target) {
    *((unsigned char*)szMsg.data()) = p.data[0];
    *((unsigned char*)(szMsg.data() + sizeof(uint64_t) + 1)) = p.data[1];
    memcpy((char*)szMsg.data() + 1, (const char*)(&p.id.guid), sizeof(uint64_t));
  } else {
    uint64_t target_id;
    memcpy(&target_id, p.data + 1, 8);

    auto target_opt = GetPlayerById(target_id);
    if (!target_opt.has_value() || !target_opt.value().get().is_ingame)
      return;

    *((unsigned char*)szMsg.data()) = p.data[0];
    memcpy((char*)szMsg.data() + sizeof(uint64_t) + 1, p.data + 1, 8);
    *((unsigned char*)(szMsg.data() + sizeof(uint64_t) + 9)) = p.data[9];
    memcpy((char*)szMsg.data() + 1, (const char*)(&p.id.guid), sizeof(uint64_t));
  }
  for (const auto& [id, existing_player] : players) {
    if (existing_player.is_ingame && existing_player.id.guid != p.id.guid) {
      g_net_server->Send((unsigned char*)szMsg.data(), p.length + sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0,
                         existing_player.id);
    }
  }
}

void GameServer::HandleDropItem(Packet p) {
  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame)
    return;

  std::string szMsg;
  szMsg.reserve(p.length + sizeof(uint64_t));

  *((unsigned char*)szMsg.data()) = p.data[0];
  memcpy((char*)szMsg.data() + sizeof(uint64_t) + 1, p.data + 1, 2);
  memcpy((char*)szMsg.data() + sizeof(uint64_t) + 3, p.data + 3, 2);
  memcpy((char*)szMsg.data() + 1, (const char*)(&p.id.guid), sizeof(uint64_t));

  for (const auto& [id, existing_player] : players) {
    if (existing_player.is_ingame && existing_player.id.guid != p.id.guid) {
      g_net_server->Send((unsigned char*)szMsg.data(), p.length + sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0,
                         existing_player.id);
    }
  }
  short amount;
  memcpy(&amount, p.data + 3, 2);
  SPDLOG_INFO("{} DROPPED ITEM. AMOUNT: {}", player_opt->get().name, amount);
}

void GameServer::HandleTakeItem(Packet p) {
  if (!config_.Get<bool>("allow_dropitems"))
    return;

  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame)
    return;

  std::string szMsg;
  szMsg.reserve(p.length + sizeof(uint64_t));
  *((unsigned char*)szMsg.data()) = p.data[0];
  memcpy((char*)szMsg.data() + sizeof(uint64_t) + 1, p.data + 1, 2);
  memcpy((char*)szMsg.data() + 1, (const char*)(&p.id.guid), sizeof(uint64_t));

  for (const auto& [id, existing_player] : players) {
    if (existing_player.is_ingame && existing_player.id.guid != p.id.guid) {
      g_net_server->Send((unsigned char*)szMsg.data(), p.length + sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0,
                         existing_player.id);
    }
  }
  SPDLOG_INFO("{} TOOK ITEM.", player_opt->get().name);
}

void GameServer::HandleRMConsole(Packet p) {
  // Intentionally left blank. This can be implemented in the scripts.
}

void MakeHTTPReq(char* file) {
  httplib::Client cli(lobbyAddress);
  cli.Get(file);
}

void GameServer::AddToPublicListHTTP() {
  using namespace std::chrono_literals;

  char* szBuff = new char[256];
  auto last_update = std::chrono::system_clock::now();
  while (g_is_server_running) {
    if (g_server->IsPublic()) {
      if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_update).count() >=
          5) {
        last_update = std::chrono::system_clock::now();
        auto server_name = g_server->config_.Get<std::string>("name");
        for (size_t i = 0; i <= strlen(server_name.c_str()); i++)
          if ((*((unsigned char*)server_name.c_str() + i) < 0x20) &&
              (*((unsigned char*)server_name.c_str() + i) != 0x07))
            *((unsigned char*)server_name.data() + i) = 0;
        memset(szBuff, 0, 256);
        sprintf(szBuff, "%s?sn=%s&port=%d&crt=%u&mx=%d&map=%s", lobbyFile, server_name.c_str(),
                g_server->config_.Get<std::int32_t>("port"), static_cast<unsigned int>(g_server->players.size()),
                g_server->config_.Get<std::int32_t>("slots"), g_server->config_.Get<std::string>("map").c_str());
        MakeHTTPReq(szBuff);
      }
    }
    std::this_thread::sleep_for(100ms);
  }
  delete[] szBuff;
}

void GameServer::HandleGameInfo(Packet p) {
  SendGameInfo(p.id);
}

// void GameServer::HandleGameInfo(Packet p){
void GameServer::SendGameInfo(Net::PlayerId who) {
  std::string szData;
  int len = 7;
  szData.reserve(64);
  *((unsigned char*)szData.data()) = PT_GAME_INFO;
  GothicClock::Time game_time = clock_->GetTime();
  memcpy((char*)szData.data() + 1, &game_time, 4);
  *((unsigned char*)szData.data() + 5) = (unsigned char)config_.Get<std::int32_t>("game_mode");
  ;
  *((unsigned char*)szData.data() + 6) = 0;
  if (config_.Get<bool>("quick_pots"))
    *((unsigned char*)szData.data() + 6) |= QUICK_POTS;
  if (config_.Get<bool>("allow_dropitems"))
    *((unsigned char*)szData.data() + 6) |= DROP_ITEMS;
  if (config_.Get<bool>("hide_map"))
    *((unsigned char*)szData.data() + 6) |= HIDE_MAP;
  g_net_server->Send((unsigned char*)szData.data(), len, MEDIUM_PRIORITY, RELIABLE, 9, who);
}

void GameServer::HandleMapNameReq(Packet p) {
  std::string szData;
  szData.reserve(256);
  *((unsigned char*)szData.data()) = PT_MAP_NAME;
  auto map = config_.Get<std::string>("map");
  memcpy((char*)szData.data() + 1, map.c_str(), strlen(map.data()) + 1);
  g_net_server->Send((char*)szData.data(), static_cast<int>(2 + strlen(map.data())), MEDIUM_PRIORITY, RELIABLE, 9,
                     p.id);
  szData.clear();
}
void GameServer::SendDisconnectionInfo(uint64_t disconnected_id) {
  std::string szData;
  szData.reserve(32);
  *((unsigned char*)szData.data()) = PT_LEFT_GAME;
  memcpy((char*)szData.data() + 1, &disconnected_id, sizeof(uint64_t));
  for (size_t i = 0; i < players.size(); i++) {
    if (players[i].is_ingame)
      if (players[i].id.guid != disconnected_id) {
        g_net_server->Send(szData.data(), 1 + sizeof(uint64_t), IMMEDIATE_PRIORITY, RELIABLE, 8, players[i].id);
      }
  }
  szData.clear();
}

bool GameServer::IsPublic() {
  return (config_.Get<bool>("public")) ? true : false;
}

void GameServer::DoRespawns() {
  auto respawn_time = config_.Get<std::int32_t>("respawn_time_seconds");
  if (respawn_time > 0) {
    if (respawn_time & 0x80000000) {
      // last stand
      switch (config_.Get<std::int32_t>("game_mode")) {
        case 1:  // TDM
          if ((!last_stand_timer) && (players.size())) {
            std::string living_team;
            size_t living_players = 0, total_ingame = 0;
            for (size_t i = 0; i < players.size(); i++) {
              if (players[i].is_ingame)
                total_ingame++;
            }
            for (size_t i = 0; i < players.size(); i++) {
              if ((players[i].is_ingame) && (!players[i].tod)) {
                living_players++;
                if (living_team.empty()) {
                  living_team = character_definition_manager_->GetCharacterDefinition(players[i].char_class).team_name;
                } else {
                  if (living_team !=
                      character_definition_manager_->GetCharacterDefinition(players[i].char_class).team_name)
                    return;
                }
              }
            }
            if ((!total_ingame) || (living_players == total_ingame))
              return;
            last_stand_timer = time(NULL) + 8;
          } else {
            if ((last_stand_timer <= time(NULL)) && (players.size())) {
              for (size_t i = 0; i < players.size(); i++) {
                if ((players[i].is_ingame) && (players[i].tod)) {
                  players[i].tod = 0;
                  players[i].health =
                      character_definition_manager_->GetCharacterDefinition(players[i].char_class).abilities[HP];
                  SendRespawnInfo(players[i].id.guid);
                }
              }
              last_stand_timer = 0;
            }
          }
          break;
        default: {
          size_t living_players = 0;
          for (size_t i = 0; i < players.size(); i++) {
            if ((players[i].is_ingame) && (!players[i].tod))
              living_players++;
          }
          if (living_players > 1)
            last_stand_timer = 0;
          if ((living_players <= 1) && (!last_stand_timer)) {
            last_stand_timer = time(NULL) + 4;
          }
          if ((living_players <= 1) &&
              (last_stand_timer <= time(NULL))) {  // do resurrection; http://www.youtube.com/watch?v=J5t0iLqqgiI&t=55s
            for (size_t i = 0; i < players.size(); i++) {
              if ((players[i].is_ingame) && (players[i].tod)) {
                players[i].flags = 0;
                players[i].tod = 0;
                players[i].health =
                    character_definition_manager_->GetCharacterDefinition(players[i].char_class).abilities[HP];
                SendRespawnInfo(players[i].id.guid);
              }
            }
            last_stand_timer = 0;
          }
        } break;
      }
    } else {
      // timed respawn
      for (size_t i = 0; i < players.size(); i++) {
        if ((players[i].is_ingame) && (players[i].tod)) {
          if (players[i].tod + respawn_time < time(NULL)) {
            players[i].flags = 0;
            players[i].tod = 0;
            players[i].health =
                character_definition_manager_->GetCharacterDefinition(players[i].char_class).abilities[HP];
            SendRespawnInfo(players[i].id.guid);
          }
        }
      }
    }
  }
}

void GameServer::SendSpamMessage() {
  auto loop_time = config_.Get<std::int32_t>("message_of_the_day_interval_seconds");
  if (loop_time > 0) {
    if (spam_time < time(NULL)) {
      spam_time = time(NULL) + loop_time;
      if (players.size()) {
        std::string szData;
        szData.reserve(512);
        *((unsigned char*)szData.data()) = PT_SRVMSG;
        memcpy((char*)szData.data() + 1, loop_msg.c_str(), loop_msg.length() + 1);
        for (size_t i = 0; i < players.size(); i++) {
          if (players[i].is_ingame)
            g_net_server->Send(szData.data(), static_cast<int>(loop_msg.length() + 2), MEDIUM_PRIORITY, UNRELIABLE, 11,
                               players[i].id);
        }
        szData.clear();
      }
    }
  }
}

void GameServer::SendDeathInfo(uint64_t deadman) {
  unsigned char buffer[9];
  buffer[0] = PT_DODIE;
  memcpy(buffer + 1, &deadman, 8);
  for (size_t i = 0; i < players.size(); i++) {
    if (players[i].is_ingame)
      g_net_server->Send((unsigned char*)buffer, 9, IMMEDIATE_PRIORITY, RELIABLE, 13, players[i].id);
  }
}

void GameServer::SendRespawnInfo(uint64_t luckyguy) {
  unsigned char buffer[9];
  buffer[0] = PT_RESPAWN;
  memcpy(buffer + 1, &luckyguy, 8);
  for (size_t i = 0; i < players.size(); i++) {
    if (players[i].is_ingame)
      g_net_server->Send((unsigned char*)buffer, 9, IMMEDIATE_PRIORITY, RELIABLE, 13, players[i].id);
  }
}

GameServer::sPlayer* GameServer::FindPlayer(const char* nickname) {
  sPlayer* ret = NULL;
  for (size_t i = 0; i < players.size(); i++) {
    if (players[i].is_ingame) {
      if (!memcmp(nickname, players[i].name.c_str(), players[i].name.length() + 1)) {
        ret = &players[i];
        break;
      }
    }
  }
  return ret;
}