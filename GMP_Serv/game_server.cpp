
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

#include <bitsery/ext/value_range.h>
#include <bitsery/traits/vector.h>
#include <httplib.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <dylib.hpp>
#include <fstream>
#include <future>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <stack>
#include <string>

#include "HTTPServer.h"
#include "git.h"
#include "gothic_clock.h"
#include "net_enums.h"
#include "packets.h"
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
void (*g_destroy_net_server_func)(Net::NetServer*) = nullptr;

using namespace Net;

namespace {

template <typename Packet, typename TContainer = std::vector<std::uint8_t>>
void SerializeAndSend(const Packet& packet, Net::PacketPriority priority, Net::PacketReliability reliable, Net::PlayerId id,
                      std::uint32_t channel = 0) {
  TContainer buffer;
  auto written_size = bitsery::quickSerialization<bitsery::OutputBufferAdapter<TContainer>>(buffer, packet);
  g_net_server->Send(buffer.data(), written_size, priority, reliable, channel, id);
}

void LoadNetworkLibrary() {
  try {
    static dylib lib("znet_server");
    auto create_net_server_func = lib.get_function<Net::NetServer*()>("CreateNetServer");
    g_destroy_net_server_func = lib.get_function<void(Net::NetServer*)>("DestroyNetServer");
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
  spdlog::default_logger()->sinks().push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::move(log_file), false));

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
  g_destroy_net_server_func(g_net_server);
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
  last_update_time_ = std::chrono::steady_clock::now();
  return true;
}

void GameServer::Run() {
  constexpr double kRadius = 5000.0;

  g_net_server->Pulse();
  clock_->RunClock();

  // Send updates to all players.
  auto now = std::chrono::steady_clock::now();
  if (now - last_update_time_ > std::chrono::milliseconds(config_.Get<std::int32_t>("tick_rate_ms"))) {
    last_update_time_ = now;

    // Consists of two player IDs.
    using PlayersKey = std::pair<uint64_t, uint64_t>;
    struct PlayersKeyHash {
      std::size_t operator()(const PlayersKey& key) const {
        std::hash<uint64_t> hasher;
        return hasher(key.first) ^ (hasher(key.second) << 1);
      }
    };

    struct PlayersKeyEqual {
      bool operator()(const PlayersKey& lhs, const PlayersKey& rhs) const {
        return lhs.first == rhs.first && lhs.second == rhs.second;
      }
    };

    std::unordered_map<PlayersKey, float, PlayersKeyHash, PlayersKeyEqual> distances;
    for (const auto& [player_id_a, player_a] : players_) {
      if (player_a.is_ingame) {
        for (const auto& [player_id_b, player_b] : players_) {
          if (player_b.is_ingame) {
            // Make sure that ids in the key are always in the same order.
            PlayersKey key{std::min(player_id_a, player_id_b), std::max(player_id_a, player_id_b)};
            auto it = distances.find(key);
            if (it == distances.end()) {
              distances[key] = glm::distance(player_a.state.position, player_b.state.position);
            }
          }
        }
      }
    }

    for (const auto& [players, distance] : distances) {
      const auto& player_a = players_[players.first];
      const auto& player_b = players_[players.second];

      if (distance < kRadius) {
        PlayerStateUpdatePacket player_a_update_packet;
        player_a_update_packet.packet_type = PT_ACTUAL_STATISTICS;
        player_a_update_packet.player_id = player_a.id.guid;
        player_a_update_packet.state = player_a.state;

        PlayerStateUpdatePacket player_b_update_packet;
        player_b_update_packet.packet_type = PT_ACTUAL_STATISTICS;
        player_b_update_packet.player_id = player_b.id.guid;
        player_b_update_packet.state = player_b.state;

        SerializeAndSend(player_a_update_packet, IMMEDIATE_PRIORITY, UNRELIABLE, player_b.id);
        SerializeAndSend(player_b_update_packet, IMMEDIATE_PRIORITY, UNRELIABLE, player_a.id);
      } else {
        PlayerPositionUpdatePacket player_a_update_packet;
        player_a_update_packet.packet_type = PT_MAP_ONLY;
        player_a_update_packet.player_id = player_a.id.guid;
        player_a_update_packet.position = player_a.state.position;

        PlayerPositionUpdatePacket player_b_update_packet;
        player_b_update_packet.packet_type = PT_MAP_ONLY;
        player_b_update_packet.player_id = player_b.id.guid;
        player_b_update_packet.position = player_b.state.position;

        SerializeAndSend(player_a_update_packet, IMMEDIATE_PRIORITY, UNRELIABLE, player_b.id);
        SerializeAndSend(player_b_update_packet, IMMEDIATE_PRIORITY, UNRELIABLE, player_a.id);
      }
    }
  }
}

bool GameServer::HandlePacket(Net::PlayerId playerId, unsigned char* data, std::uint32_t size) {
  Packet p{data, size, playerId};

  unsigned char packetIdentifier = GetPacketIdentifier(p);
  SPDLOG_INFO("Packet identifier: {}", packetIdentifier);
  switch (packetIdentifier) {
    case ID_DISCONNECTION_NOTIFICATION:
      SendDisconnectionInfo(p.id.guid);
      DeleteFromPlayerList(p.id);
      SPDLOG_INFO("{} disconnected. Still connected {} users.", g_net_server->GetPlayerIp(p.id), players_.size());
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
      players_[p.id.guid] = std::move(pl);
    }
      SPDLOG_INFO("ID_NEW_INCOMING_CONNECTION from {} with GUID {}. Now we have {} connected users.", g_net_server->GetPlayerIp(p.id), p.id.guid,
                  players_.size());
      break;
    case ID_INCOMPATIBLE_PROTOCOL_VERSION:
      SPDLOG_WARN("ID_INCOMPATIBLE_PROTOCOL_VERSION");
      break;
    case ID_CONNECTION_LOST:
      SendDisconnectionInfo(p.id.guid);
      DeleteFromPlayerList(p.id);
      SPDLOG_WARN("Connection lost from {}. Still connected {} users.", g_net_server->GetPlayerIp(p.id), players_.size());
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
    case PT_COMMAND:
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
  players_.erase(id.guid);
}

std::optional<std::reference_wrapper<GameServer::sPlayer>> GameServer::GetPlayerById(std::uint64_t id) {
  auto it = players_.find(id);
  if (it != players_.end())
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
      players_.erase(p.id.guid);
      g_net_server->AddToBanList(p.id, 3600000);  // i dorzucamy banana na 1h
      return;
    }
  }

  JoinGamePacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);
  SPDLOG_TRACE("{} from {}", packet, p.id.guid);

  player.tod = 0;
  player.char_class = packet.selected_class;
  player.health = character_definition_manager_->GetCharacterDefinition(packet.selected_class).abilities[HP];

  player.state.position = packet.position;
  player.state.nrot = packet.normal;
  player.state.left_hand_item_instance = packet.left_hand_item_instance;
  player.state.right_hand_item_instance = packet.right_hand_item_instance;
  player.state.equipped_armor_instance = packet.equipped_armor_instance;
  player.state.animation = packet.animation;
  player.head = packet.head_model;
  player.skin = packet.skin_texture;
  player.body = packet.face_texture;
  player.walkstyle = packet.walk_style;
  player.name = packet.player_name;

  // Update the packet we received with his ID, so we can send it to others.
  packet.player_id = p.id.guid;

  std::vector<ExistingPlayerPacket> existing_players;
  existing_players.reserve(players_.size());

  // Informing other players about new player
  for (const auto& [id, existing_player] : players_) {
    if (existing_player.id.guid != p.id.guid) {
      if (existing_player.is_ingame) {
        SerializeAndSend(packet, IMMEDIATE_PRIORITY, RELIABLE, existing_player.id);
      }

      ExistingPlayerPacket player_packet;
      player_packet.packet_type = PT_ALL_OTHERS;
      player_packet.player_id = existing_player.id.guid;
      player_packet.selected_class = existing_player.char_class;
      player_packet.position = existing_player.state.position;
      player_packet.left_hand_item_instance = existing_player.state.left_hand_item_instance;
      player_packet.right_hand_item_instance = existing_player.state.right_hand_item_instance;
      player_packet.equipped_armor_instance = existing_player.state.equipped_armor_instance;
      player_packet.head_model = existing_player.head;
      player_packet.skin_texture = existing_player.skin;
      player_packet.face_texture = existing_player.body;
      player_packet.walk_style = existing_player.walkstyle;
      player_packet.player_name = existing_player.name;
      existing_players.push_back(std::move(player_packet));
    }
  }

  // Previously, we were splitting this into mulitple packets. Find out if this is still necessary.
  std::vector<std::uint8_t> buffer;
  bitsery::OutputBufferAdapter<std::vector<std::uint8_t>> adapter(buffer);
  auto serializer = bitsery::Serializer<bitsery::OutputBufferAdapter<std::vector<std::uint8_t>>>(std::move(adapter));
  serializer.container(existing_players, 400);
  serializer.adapter().flush();
  auto written_size = serializer.adapter().writtenBytesCount();
  g_net_server->Send(buffer.data(), written_size, IMMEDIATE_PRIORITY, RELIABLE, 0, p.id);

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

  PlayerStateUpdatePacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  updated_player.state = packet.state;

  // Lua event on player update?
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
  for (const auto& [id, existing_player] : players_) {
    if (existing_player.is_ingame && existing_player.id.guid != p.id.guid) {
      g_net_server->Send((unsigned char*)data.data(), p.length, IMMEDIATE_PRIORITY, UNRELIABLE, 5, existing_player.id);
    }
  }
}

void GameServer::HandleNormalMsg(Packet p) {
  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame || player_opt.value().get().mute)
    return;

  MessagePacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  EventManager::Instance().TriggerEvent(kEventOnPlayerMessageName, OnPlayerMessageEvent{p.id.guid, packet.message});

  packet.sender = p.id.guid;
  for (const auto& [id, existing_player] : players_) {
    if (existing_player.is_ingame) {
      SerializeAndSend(packet, LOW_PRIORITY, RELIABLE_ORDERED, existing_player.id);
    }
  }

  SPDLOG_INFO("{}", packet);
}

void GameServer::HandleWhisp(Packet p) {
  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame)
    return;

  MessagePacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  if (!packet.recipient.has_value()) {
    SPDLOG_ERROR("No recipient in whisper packet!");
    return;
  }

  auto recipient_opt = GetPlayerById(*packet.recipient);
  if (!recipient_opt.has_value())
    return;
  auto& recipient = recipient_opt.value().get();
  packet.sender = p.id.guid;

  SerializeAndSend(packet, LOW_PRIORITY, RELIABLE_ORDERED, p.id);
  SerializeAndSend(packet, LOW_PRIORITY, RELIABLE_ORDERED, recipient.id);

  SPDLOG_INFO("({} WHISPERS TO {}) {}", player_opt->get().name, recipient.name, (const char*)(p.data + 1 + sizeof(uint64_t)));
}

void GameServer::HandleCastSpell(Packet p, bool target) {
  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame)
    return;

  CastSpellPacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);
  packet.caster_id = p.id.guid;

  if (target) {
    if (!packet.target_id.has_value()) {
      SPDLOG_ERROR("No target in cast spell packet!");
      return;
    }

    auto target_opt = GetPlayerById(*packet.target_id);
    if (!target_opt.has_value() || !target_opt.value().get().is_ingame) {
      return;
    }
  }
  for (const auto& [id, existing_player] : players_) {
    if (existing_player.is_ingame && existing_player.id.guid != p.id.guid) {
      SerializeAndSend(packet, HIGH_PRIORITY, RELIABLE, existing_player.id);
    }
  }
}

void GameServer::HandleDropItem(Packet p) {
  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame)
    return;

  DropItemPacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);
  packet.player_id = p.id.guid;

  for (const auto& [id, existing_player] : players_) {
    if (existing_player.is_ingame && existing_player.id.guid != p.id.guid) {
      SerializeAndSend(packet, HIGH_PRIORITY, RELIABLE, existing_player.id);
    }
  }
  SPDLOG_INFO("{} DROPPED ITEM. AMOUNT: {}", player_opt->get().name, packet.item_amount);
}

void GameServer::HandleTakeItem(Packet p) {
  if (!config_.Get<bool>("allow_dropitems"))
    return;

  auto player_opt = GetPlayerById(p.id.guid);
  if (!player_opt.has_value() || !player_opt.value().get().is_ingame)
    return;

  TakeItemPacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);
  packet.player_id = p.id.guid;

  for (const auto& [id, existing_player] : players_) {
    if (existing_player.is_ingame && existing_player.id.guid != p.id.guid) {
      SerializeAndSend(packet, HIGH_PRIORITY, RELIABLE, existing_player.id);
    }
  }
  SPDLOG_INFO("{} TOOK ITEM.", player_opt->get().name);
}

void GameServer::HandleRMConsole(Packet p) {
  // Intentionally left blank. This can be implemented in the scripts.
}

void MakeHTTPReq(const std::string& file) {
  httplib::Client cli(lobbyAddress);
  cli.Get(file);
}

void GameServer::AddToPublicListHTTP() {
  using namespace std::chrono_literals;

  auto last_update = std::chrono::system_clock::now();
  while (g_is_server_running) {
    if (g_server->IsPublic()) {
      if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_update).count() >= 5) {
        last_update = std::chrono::system_clock::now();
        auto server_name = g_server->config_.Get<std::string>("name");
        for (size_t i = 0; i <= strlen(server_name.c_str()); i++)
          if ((*((unsigned char*)server_name.c_str() + i) < 0x20) && (*((unsigned char*)server_name.c_str() + i) != 0x07))
            *((unsigned char*)server_name.data() + i) = 0;
        std::string buffer = fmt::format("{}?sn={}&port={}&crt={}&mx={}&map={}", lobbyFile, server_name, g_server->config_.Get<std::int32_t>("port"),
                                         static_cast<unsigned int>(g_server->players_.size()), g_server->config_.Get<std::int32_t>("slots"),
                                         g_server->config_.Get<std::string>("map"));
        MakeHTTPReq(buffer);
      }
    }
    std::this_thread::sleep_for(100ms);
  }
}

void GameServer::HandleGameInfo(Packet p) {
  SendGameInfo(p.id);
}

// void GameServer::HandleGameInfo(Packet p){
void GameServer::SendGameInfo(Net::PlayerId who) {
  GameInfoPacket packet;
  packet.packet_type = PT_GAME_INFO;
  GothicClock::TimeUnion game_time = clock_->GetTime();
  packet.raw_game_time = game_time.raw;

  packet.game_mode = config_.Get<std::int32_t>("game_mode");
  if (config_.Get<bool>("quick_pots")) {
    packet.flags |= QUICK_POTS;
  }
  if (config_.Get<bool>("allow_dropitems")) {
    packet.flags |= DROP_ITEMS;
  }
  if (config_.Get<bool>("hide_map")) {
    packet.flags |= HIDE_MAP;
  }

  SerializeAndSend(packet, MEDIUM_PRIORITY, RELIABLE, who, 9);
}

void GameServer::HandleMapNameReq(Packet p) {
  MapNamePacket packet;
  packet.packet_type = PT_MAP_NAME;
  packet.map_name = config_.Get<std::string>("map");
  SerializeAndSend(packet, MEDIUM_PRIORITY, RELIABLE, p.id, 9);
}

void GameServer::SendDisconnectionInfo(uint64_t disconnected_id) {
  DisconnectionInfoPacket packet;
  packet.disconnected_id = disconnected_id;
  packet.packet_type = PT_LEFT_GAME;

  for (const auto& [id, existing_player] : players_) {
    if (existing_player.is_ingame && existing_player.id.guid != disconnected_id) {
      SerializeAndSend(packet, IMMEDIATE_PRIORITY, RELIABLE, existing_player.id);
    }
  }
}

bool GameServer::IsPublic() {
  return (config_.Get<bool>("public")) ? true : false;
}

void GameServer::SendSpamMessage() {
  auto loop_time = config_.Get<std::int32_t>("message_of_the_day_interval_seconds");
  if (loop_time > 0) {
    if (spam_time < time(NULL)) {
      spam_time = time(NULL) + loop_time;
      if (!players_.empty()) {
        MessagePacket packet;
        packet.packet_type = PT_SRVMSG;
        packet.message = loop_msg;

        for (const auto& [id, existing_player] : players_) {
          if (existing_player.is_ingame) {
            SerializeAndSend(packet, MEDIUM_PRIORITY, UNRELIABLE, existing_player.id, 11);
          }
        }
      }
    }
  }
}

void GameServer::SendDeathInfo(uint64_t deadman) {
  PlayerDeathInfoPacket packet;
  packet.packet_type = PT_DODIE;
  packet.player_id = deadman;

  for (const auto& [id, existing_player] : players_) {
    if (existing_player.is_ingame) {
      SerializeAndSend(packet, IMMEDIATE_PRIORITY, RELIABLE, existing_player.id, 13);
    }
  }
}

void GameServer::SendRespawnInfo(uint64_t luckyguy) {
  PlayerRespawnInfoPacket packet;
  packet.packet_type = PT_RESPAWN;
  packet.player_id = luckyguy;

  for (const auto& [id, existing_player] : players_) {
    if (existing_player.is_ingame) {
      SerializeAndSend(packet, IMMEDIATE_PRIORITY, RELIABLE, existing_player.id, 13);
    }
  }
}