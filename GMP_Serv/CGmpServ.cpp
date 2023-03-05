
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

#include "CGmpServ.h"
#include "platform_depend.h"
#include "CPermissions.h"
#include "CCompression.h"
#include "gothic_clock.h"
#include "znet_server.h"
#include "server_events.h"
#include "event.h"
#include <dylib.hpp>
#include <fstream>
#include <string>
#include <future>
#include "net_enums.h"

#include <stack>
#include <httplib.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "HTTPServer.h"

CGmpServ *pSrv = nullptr;
Net::NetServer* g_net_server = nullptr;

const char *lobbyAddress="lobby.your-site.com";
const char *lobbyFile="add.php";

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
extern const char* HP_REGEN;
extern const char* MP_REGEN;
//admin only
const char *MOD_ADD="modadd ";
const char *MOD_SET="modset ";	//ustawia hasło dla określonego moderatora
const char *MOD_DEL="moddel ";

const char* WTF="Dude, I dont understand you.";
const char* OK="OK!";
const char* AG="Access granted!";
const char* PDE="Player doesn't exist!";
const char* IPNOTBANNED="Following IP isn't banned!";
const char* IPISBANNED="Following IP is already banned!";
const char* INVALIDPARAMETER="Invalid command parameter!";
#define MAX_KILL_TXT 3
const char* KILLED[MAX_KILL_TXT]={"K.O.","R.I.P.","FATALITY"};

using namespace Net;

namespace
{

void LoadNetworkLibrary()
{
  try {
    static dylib lib("znet_server");
    auto create_net_server_func = lib.get_function<Net::NetServer*()>("CreateNetServer");
    g_net_server = create_net_server_func();
  } catch (std::exception& ex) {
    SPDLOG_ERROR("LoadNetworkLibrary error: {}", ex.what());
    std::abort();
  }
}

void InitializeLogger(const Config& config)
{
  if (!config.Get<bool>("log_to_stdout"))
  {
    spdlog::default_logger()->sinks().clear();
  }

  auto log_file = config.Get<std::string>("log_file");
  spdlog::default_logger()->sinks().push_back(
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::move(log_file), false));

  auto log_level = config.Get<std::string>("log_level");
  spdlog::set_level(spdlog::level::from_str(log_level));
	spdlog::flush_on(spdlog::level::debug);
}
} // namespace

CGmpServ::CGmpServ(int argc, char** argv)
{
	InitializeLogger(config_);
	config_.LogConfigValues();
  pSrv = this;
  arg_count = argc;
  arg_vec = argv;

	// Register server-side events.
	EventManager::Instance().RegisterEvent("OnPlayerConnect");
}

CGmpServ::~CGmpServ() {
  g_net_server->RemovePacketHandler(*this);
  delete classmgr;
  delete script;
  // server->Shutdown(300);
  pSrv = nullptr;
}

bool CGmpServ::Init() {
  LoadNetworkLibrary();
  g_net_server->AddPacketHandler(*this);
#ifndef WIN32
  if (config_.Get<bool>("daemon")) {
    System::MakeMeDaemon(false);
  }
#endif
  CPermissions* perms = new CPermissions();
  perms = NULL;
  this->spam_time = time(NULL) + 10;
  this->classmgr = new CClassManager;

  auto slots = config_.Get<std::int32_t>("slots");
  allow_modification = config_.Get<bool>("allow_modification");
  loop_msg = config_.Get<std::string>("message_of_the_day");

  auto port = config_.Get<std::int32_t>("port");

  if (!g_net_server->Start(port, slots)) {
    return false;
  }

  SPDLOG_INFO("GMP server started!");
  LoadBanList();

  clock_ = std::make_unique<GothicClock>(config_.Get<GothicClock::Time>("game_time"));
  public_list_http_thread_future_ = std::async(&CGmpServ::AddToPublicListHTTP, this);
  http_thread_future_ = std::async(&CGmpServ::HTTPServerThread, this, port + 1);
  this->last_stand_timer = 0;
  script = new Script(config_.Get<std::vector<std::string>>("scripts"));
  return true;
}

void CGmpServ::HTTPServerThread(std::int32_t port)
{
  auto httpServer = std::make_unique<HTTPServer>();
  httpServer->Start(port);
}

void CGmpServ::Run()
{
	g_net_server->Pulse();
	clock_->RunClock();
}

bool CGmpServ::HandlePacket(Net::PlayerId playerId, unsigned char* data, std::uint32_t size)
{
	Packet p{data, size, playerId};

	unsigned char packetIdentifier = GetPacketIdentifier(p);
	switch (packetIdentifier)
	{
	case ID_DISCONNECTION_NOTIFICATION:
		SendDisconnectionInfo(p.id.guid);
		DeleteFromPlayerList(p.id);
		SPDLOG_INFO("{} disconnected. Still connected {} users.", g_net_server->GetPlayerIp(p.id), players.size());
		break;
	case ID_NEW_INCOMING_CONNECTION:	//nadchodzące połączenie zaakceptowane | można dodać guid gracza do listy graczy;
		{
			sPlayer pl;
			pl.has_admin=0;
			pl.moderator=NULL;
			pl.moderator_passwd=0;
			pl.admin_passwd=0;
			pl.is_ingame=0;
			pl.passed_crc_test=0;
			pl.id=p.id;
			pl.mute=0;
			players.push_back(pl);
		}
		SPDLOG_INFO("ID_NEW_INCOMING_CONNECTION from {} with GUID {}. Now we have {} connected users.", g_net_server->GetPlayerIp(p.id), p.id.guid, players.size());
		break;
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		SPDLOG_WARN("ID_INCOMPATIBLE_PROTOCOL_VERSION");
		break;
	case ID_CONNECTION_LOST:
		SendDisconnectionInfo(p.id.guid);
		DeleteFromPlayerList(p.id);
		SPDLOG_WARN("Connection lost from {}. Still connected {} users.", g_net_server->GetPlayerIp(p.id), players.size());
		break;
	case PT_REQUEST_FILE_LENGTH:
	case PT_REQUEST_FILE_PART:
		break;
	case PT_WHOAMI: //zwraca id gracza(czyli jego guid)
		{
			unsigned char val[9];
			val[0]=PT_WHOAMI;
			memcpy(val+1, &p.id.guid, sizeof(uint64_t));
			g_net_server->Send((const char*)val, 9, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p.id);
		}
		break;
	case PT_JOIN_GAME:
		SomeoneJoinGame(p);
		break;
	case PT_ACTUAL_STATISTICS:  //dostarcza nam informacji o sobie
		HandlePlayerUpdate(p);
		break;
	case PT_CHECKSUM:
		if(!this->allow_modification){
			auto map_md5 = config_.Get<std::string>("map_md5");
			if((!memcmp(p.data+2, map_md5.data(), 16)) && (!p.data[1])){
				players[FindIDOnList(p.id.guid)].passed_crc_test=1;
				unsigned char val[2]={PT_CHECKSUM, 0xFF};
				g_net_server->Send((const char*)val, 2, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, p.id);
			}
			else{
				players.erase(players.begin()+FindIDOnList(p.id.guid));
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
	case PT_GAME_INFO:// na razie tylko czas
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

bool CGmpServ::Receive()
{
	g_net_server->Pulse();
	return true;
}

unsigned char CGmpServ::GetPacketIdentifier(const Packet& p) {
  if ((unsigned char)p.data[0] == ID_TIMESTAMP) {
    return (unsigned char)p.data[1 + sizeof(std::uint32_t)];
  } else
    return (unsigned char)p.data[0];
}

void CGmpServ::LoadBanList() {
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

void CGmpServ::SaveBanList(){
	if(!this->ban_list.empty()){
		std::ofstream ofs("bans.txt");
		if(!ofs.is_open()){
			SPDLOG_ERROR("Could not save active bans to file bans.txt!");
			return;
		}
		for(size_t i=0; i<this->ban_list.size(); i++){
			ofs << this->ban_list[i].c_str() << "\n";
		}
		ofs.clear();
	}
	SPDLOG_INFO("Bans written to bans.txt.");
}

void CGmpServ::DeleteFromPlayerList(Net::PlayerId id) {
  for (size_t i = 0; i < players.size(); i++) {
    if (id.guid == players[i].id.guid) {
      players.erase(players.begin() + i);
      break;
    }
  }
}

void CGmpServ::SomeoneJoinGame(Packet p){
	size_t pos_in_list=FindIDOnList(p.id.guid);
	if(!allow_modification){
		if(!players[pos_in_list].passed_crc_test){
			players.erase(players.begin()+pos_in_list); //nie ma szans, usuwamy z listy
			g_net_server->AddToBanList(p.id, 3600000);//i dorzucamy banana na 1h
			return;
		}
	}
	players[pos_in_list].tod=0;
	players[pos_in_list].char_class=p.data[1];
	players[pos_in_list].health=classmgr->class_array[p.data[1]].abilities[HP];
	memcpy(players[pos_in_list].pos, p.data+2, 12);
	memcpy(players[pos_in_list].nrot, p.data+14, 12);
	memcpy(&players[pos_in_list].left_hand, p.data+26, 2);
	memcpy(&players[pos_in_list].right_hand, p.data+28, 2);
	memcpy(&players[pos_in_list].armor, p.data+30, 2);
	memcpy(&players[pos_in_list].animation, p.data+32, 2);
	players[pos_in_list].head=p.data[34];
	players[pos_in_list].skin=p.data[35];
	players[pos_in_list].body=p.data[36];
	players[pos_in_list].walkstyle=p.data[37];
	players[pos_in_list].name=(const char*)(p.data+38);
	//TO DO sprawdzenie czy ktoś już nie ma takiej ksywki
	short matches;
	do{
		matches=0;
		for(size_t i=0; i<players.size(); i++){
			if(i!=pos_in_list){
				if(players[i].is_ingame){
					if(!memcmp(players[pos_in_list].name.c_str(), players[i].name.c_str(), players[pos_in_list].name.length()+1)){
						matches++;
						char letter=*players[pos_in_list].name.rbegin();
						if((letter>='0') && (letter<='8')){
							letter++;
							players[pos_in_list].name.erase(players[pos_in_list].name.size()-1);
							players[pos_in_list].name+=letter;
						} else players[pos_in_list].name+='0';
						break;
					}
				}
			}
		}
	}while(matches);
	unsigned char nick_packet[128];
	memcpy(nick_packet+1, players[pos_in_list].name.c_str(), players[pos_in_list].name.length()+1);
	nick_packet[0]=PT_YOUR_NAME;
	g_net_server->Send(nick_packet, 2+(int)players[pos_in_list].name.length(), IMMEDIATE_PRIORITY, RELIABLE, 0, p.id);
	std::string inform_packet;
	inform_packet.reserve(1024);
	char *szTmpPtr=(char*)inform_packet.c_str();
	szTmpPtr[0]=p.data[0];
	memcpy((char*)inform_packet.data()+(1+sizeof(uint64_t)), p.data+1, p.length-1);
	memcpy((char*)inform_packet.data()+1, &p.id.guid, sizeof(uint64_t));
	//informowanie innych o tym że ktoś dołączył się do gej party
	memcpy((char*)inform_packet.data()+(38+sizeof(uint64_t)), players[pos_in_list].name.c_str(), players[pos_in_list].name.length());
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame){
			g_net_server->Send((unsigned char*)inform_packet.data(), 40+(int)players[pos_in_list].name.length()+sizeof(uint64_t), IMMEDIATE_PRIORITY, RELIABLE, 3, players[i].id);
		}
	}
	//generowanie i słanie pakietów o graczach(max 10 graczy per pakiet)
	memset((char*)inform_packet.data(), 0, 1024);
	unsigned char it=0;
	size_t szIt=1;
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame){
			it++;
			memcpy((char*)inform_packet.data()+szIt, &players[i].id.guid, (sizeof(uint64_t))); szIt+=sizeof(uint64_t);
			*(char*)(inform_packet.data()+szIt)=players[i].char_class; szIt++;
			memcpy((char*)inform_packet.data()+szIt, players[i].pos, 12); szIt+=12;
			memcpy((char*)inform_packet.data()+szIt, &players[i].left_hand, 2); szIt+=2;
			memcpy((char*)inform_packet.data()+szIt, &players[i].right_hand, 2); szIt+=2;
			memcpy((char*)inform_packet.data()+szIt, &players[i].armor, 2); szIt+=2;
			*(char*)(inform_packet.data()+szIt)=players[i].head; szIt++;
			*(char*)(inform_packet.data()+szIt)=players[i].skin; szIt++;
			*(char*)(inform_packet.data()+szIt)=players[i].body; szIt++;
			*(char*)(inform_packet.data()+szIt)=players[i].walkstyle; szIt++;
			memcpy((char*)inform_packet.data()+szIt, players[i].name.c_str(), players[i].name.length()+1); szIt+=players[i].name.length()+1;
		}
		if(it==10){
			*(unsigned char*)inform_packet.data()=PT_ALL_OTHERS;
			g_net_server->Send((unsigned char*)inform_packet.data(), (int)szIt, IMMEDIATE_PRIORITY, RELIABLE, 2, p.id);
			it=0;
			szIt=1;
		}
	}
	if(it>0){
		*(unsigned char*)(inform_packet.data())=PT_ALL_OTHERS;
		g_net_server->Send((unsigned char*)inform_packet.data(), (int)szIt, IMMEDIATE_PRIORITY, RELIABLE, 2, p.id);
		it=0;
		szIt=1;
	}
	players[pos_in_list].is_ingame=1;

	// join
	EventManager::Instance().TriggerEvent("OnPlayerConnect", p.id.guid);
}

size_t CGmpServ::FindIDOnList(uint64_t guid) {
  for (size_t i = 0; i < players.size(); i++) {
    if (guid == players[i].id.guid)
      return i;
  }
  size_t ret = 0;
  ret = ~ret;
  return ret;
}

void CGmpServ::HandlePlayerUpdate(Packet p){
	double rp, x1, y1;  //<- zmienne do sprawdzenia czy dana osoba mieści się w kole o r=5000.0f
	size_t pos_in_list=FindIDOnList(p.id.guid);
	size_t forbidenid=0;
	forbidenid= ~forbidenid;
	if(pos_in_list==forbidenid) return;
	memcpy(players[pos_in_list].pos, p.data+1, 12);
	memcpy(players[pos_in_list].nrot, p.data+13, 12);
	memcpy(&players[pos_in_list].left_hand, p.data+25, 2);
	memcpy(&players[pos_in_list].right_hand, p.data+27, 2);
	memcpy(&players[pos_in_list].armor, p.data+29, 2);
	memcpy(&players[pos_in_list].animation, p.data+31, 2);
	memcpy(&players[pos_in_list].mana, p.data+33, 2);
	players[pos_in_list].figth_pos=p.data[35];
	players[pos_in_list].spellhand=p.data[36];
	players[pos_in_list].headstate=p.data[37];
	memcpy(&players[pos_in_list].rangedeq, p.data+38, 2);
	memcpy(&players[pos_in_list].meleeeq, p.data+40, 2);
	
	//generowanie wiadomości do słania
	size_t szIt[2]={1,1025};
	unsigned char it[2]={0,0};
	std::string msg;
	msg.reserve(2048);
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame){
			if(i!=pos_in_list){
				x1=static_cast<double>(players[pos_in_list].pos[0]-players[i].pos[0]);
				y1=static_cast<double>(players[pos_in_list].pos[1]-players[i].pos[1]);
				rp=x1*x1+y1*y1;
				if(rp<25000000.0){  //ktoś czający się w pobliżu gracza
					it[0]++;
					memcpy((char*)msg.data()+szIt[0], &players[i].id.guid, sizeof(uint64_t)); szIt[0]+=sizeof(uint64_t);
					*(char*)(msg.data()+szIt[0])=players[i].flags; szIt[0]++;
					memcpy((char*)msg.data()+szIt[0], players[i].pos, 12); szIt[0]+=12;
					memcpy((char*)msg.data()+szIt[0], players[i].nrot, 12); szIt[0]+=12;
					memcpy((char*)msg.data()+szIt[0], &players[i].left_hand, 2); szIt[0]+=2;
					memcpy((char*)msg.data()+szIt[0], &players[i].right_hand, 2); szIt[0]+=2;
					memcpy((char*)msg.data()+szIt[0], &players[i].armor, 2); szIt[0]+=2;
					memcpy((char*)msg.data()+szIt[0], &players[i].animation, 2); szIt[0]+=2;
					memcpy((char*)msg.data()+szIt[0], &players[i].health, 2); szIt[0]+=2;
					memcpy((char*)msg.data()+szIt[0], &players[i].mana, 2); szIt[0]+=2;
					*(char*)(msg.data()+szIt[0])=players[i].spellhand; szIt[0]++;
					*(char*)(msg.data()+szIt[0])=players[i].figth_pos; szIt[0]++;
					*(char*)(msg.data()+szIt[0])=players[i].headstate; szIt[0]++;
					memcpy((char*)msg.data()+szIt[0], &players[i].rangedeq, 2); szIt[0]+=2;
					memcpy((char*)msg.data()+szIt[0], &players[i].meleeeq, 2); szIt[0]+=2;
				}
				else{   //ktoś nieistotny
					it[1]++;
					memcpy((char*)msg.data()+szIt[1], &players[i].id.guid, sizeof(uint64_t)); szIt[1]+=sizeof(uint64_t);
					memcpy((char*)msg.data()+szIt[1], &players[i].pos[0], 4); szIt[1]+=4; //float x,y
					memcpy((char*)msg.data()+szIt[1], &players[i].pos[2], 4); szIt[1]+=4;
				}
			}
			else{//to jesteśmy my, starczy tylko info o hp
				it[0]++;
				memcpy((char*)msg.data()+szIt[0], &p.id.guid, sizeof(uint64_t)); szIt[0]+=sizeof(uint64_t);
				memcpy((char*)msg.data()+szIt[0], &players[pos_in_list].health, 2); szIt[0]+=2;
			}
			if(it[0]==10){
				*(unsigned char*)(msg.data())=PT_ACTUAL_STATISTICS;
				g_net_server->Send((unsigned char*)msg.data(), (int)szIt[0], IMMEDIATE_PRIORITY, UNRELIABLE, 0, p.id);
				it[0]=0;
				szIt[0]=1;
			}
			if(it[1]==60){
				*(unsigned char*)(msg.data()+1024)=PT_MAP_ONLY;
				g_net_server->Send((unsigned char*)msg.data()+1024, (int)szIt[1]-1024, IMMEDIATE_PRIORITY, UNRELIABLE, 0, p.id);
				it[1]=0;
				szIt[1]=1025;
			}
		}
	}
	if(it[0]>0){
		*(unsigned char*)(msg.data())=PT_ACTUAL_STATISTICS;
		g_net_server->Send((unsigned char*)msg.data(), (int)szIt[0], IMMEDIATE_PRIORITY, UNRELIABLE, 0, p.id);
		it[0]=0;
	}
	if(it[1]>0){
		*(unsigned char*)(msg.data()+1024)=PT_MAP_ONLY;
		g_net_server->Send((unsigned char*)msg.data()+1024, (int)szIt[1]-1024, IMMEDIATE_PRIORITY, UNRELIABLE, 32, p.id);
		it[1]=0;
	}
}

void CGmpServ::MakeHPDiff(Packet p){
	size_t forbidenid=0, pwgd;
	uint64_t player_id;
	short diffed_hp;
	forbidenid= ~forbidenid;
	if(forbidenid==FindIDOnList(p.id.guid)) return;
	if(players[FindIDOnList(p.id.guid)].is_ingame){
		memcpy(&player_id, p.data+1, sizeof(uint64_t));
		memcpy(&diffed_hp, p.data+(1+sizeof(uint64_t)), 2);
		pwgd=FindIDOnList(player_id);
		if((pwgd==forbidenid) || (pwgd>=players.size())) return;
		if((players[pwgd].health<=0) || (players[pwgd].tod)) return;	//just ignore
		if(player_id==p.id.guid){
			if(players[pwgd].health) players[pwgd].health+=diffed_hp;
			if(players[pwgd].health<=0){
				players[pwgd].health=0;
				players[pwgd].tod=time(NULL);
				SendDeathInfo(players[pwgd].id.guid);
			}
			if(players[pwgd].health>classmgr->class_array[players[pwgd].char_class].abilities[HP]) players[pwgd].health=classmgr->class_array[players[pwgd].char_class].abilities[HP];
		}
		else{
			if(config_.Get<bool>("be_unconcious_before_dead")){
				size_t attacker=FindIDOnList(p.id.guid);
				switch(players[attacker].figth_pos){
					case 1:
					case 3:
					case 4: //od tych broni nasz zawodnik na pewno nie padnie
						if(players[pwgd].flags&PL_UNCONCIOUS){
							players[pwgd].flags&=0xFE;
							players[pwgd].tod=time(NULL);
							players[pwgd].health=0;
							SendDeathInfo(players[pwgd].id.guid);
						}
						else{
							players[pwgd].health+=diffed_hp;
							if(players[pwgd].health<=1){
								players[pwgd].health=1;
								players[pwgd].flags|=PL_UNCONCIOUS;
							}
						}
						break;
					default:
						if(players[pwgd].health>0){
							players[pwgd].health+=diffed_hp;
							if(players[pwgd].health<0) players[pwgd].health=0;
							if(players[pwgd].health<=0){
								players[pwgd].tod=time(NULL);
								SendDeathInfo(players[pwgd].id.guid);
							}
						}
						break;
				}
			} else{
				players[pwgd].health+=diffed_hp;
				if(players[pwgd].health==1){
					players[pwgd].health=0;
					players[pwgd].tod=time(NULL);
					SendDeathInfo(players[pwgd].id.guid);
				}
				if((players[pwgd].health==0) && (!players[pwgd].tod)){
					players[pwgd].tod=time(NULL);
					SendDeathInfo(players[pwgd].id.guid);
				}
				if(players[pwgd].health<0){
					players[pwgd].health=0;
					players[pwgd].tod=time(NULL);
					SendDeathInfo(players[pwgd].id.guid);
				}
				if(players[pwgd].health>classmgr->class_array[players[pwgd].char_class].abilities[HP]) players[pwgd].health=classmgr->class_array[players[pwgd].char_class].abilities[HP];
			}
		}
	}
	if((players[pwgd].health<=0) && (!players[pwgd].tod)){
		players[pwgd].tod=time(NULL);
		SendDeathInfo(players[pwgd].id.guid);
	}
}

void CGmpServ::HandleVoice(Packet p) {
  // TODO: no need to resend player id right now, it won't be needed until we add 3d chat
  std::string data;
  data.reserve(p.length);
  memcpy(data.data(), p.data, p.length);
  for (size_t i = 0; i < players.size(); i++) {
    if (players[i].is_ingame && players[i].id.guid != p.id.guid) {
      g_net_server->Send((unsigned char*)data.data(), p.length, IMMEDIATE_PRIORITY, UNRELIABLE, 5, players[i].id);
    }
  }
}

void CGmpServ::HandleNormalMsg(Packet p){
	size_t forbiden_id=0;
	std::string szMsg;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p.id.guid)==forbiden_id) return;
	else if(!players[FindIDOnList(p.id.guid)].is_ingame) return;
	if(players[FindIDOnList(p.id.guid)].mute) return;
	size_t stranger=FindIDOnList(p.id.guid);
	szMsg.reserve(1024);
	*((unsigned char*)szMsg.data())=p.data[0];
	memcpy((char*)szMsg.data()+sizeof(uint64_t)+1, p.data+1, p.length-1);
	memcpy((char*)szMsg.data()+1,(const char*)(&p.id.guid), sizeof(uint64_t));
	if(!config_.Get<bool>("real_chat")) for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame) g_net_server->Send((unsigned char*)szMsg.data(), p.length+sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 5, players[i].id);
	}
#define EQ(x) (players[i].pos[x]-players[stranger].pos[x])
	else for(size_t i=0;i<players.size(); i++){
		if(players[i].is_ingame){
                  if ((EQ(0) * EQ(0) + EQ(1) * EQ(1) <= 2000000.0f) && (EQ(0) * EQ(0) + EQ(2) * EQ(2) <= 2000000.0f) &&
                      (EQ(1) * EQ(1) + EQ(2) * EQ(2) <= 2000000.0f))
                  {
				g_net_server->Send((unsigned char*)szMsg.data(), p.length+sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 5, players[i].id);
			}
		}
	}
#undef EQ
	SPDLOG_INFO("{}:{}", players[FindIDOnList(p.id.guid)].name.c_str(), (const char*)(p.data+1));
	szMsg.clear();
}

void CGmpServ::HandleWhisp(Packet p)
{
  size_t forbiden_id = 0;
  forbiden_id = ~forbiden_id;
  if (FindIDOnList(p.id.guid) == forbiden_id)
    return;
  else if (!players[FindIDOnList(p.id.guid)].is_ingame)
    return;
  uint64_t say_to;
  memcpy(&say_to, p.data + 1, sizeof(uint64_t));
  if (FindIDOnList(say_to) == forbiden_id)
    return;
  std::string szMsg;
  szMsg.reserve(1024);
  *((char*)szMsg.data()) = p.data[0];
  memcpy((char*)szMsg.data() + 1 + sizeof(uint64_t), p.data + 1, p.length - 1);
  memcpy((char*)(szMsg.data() + 1), &p.id.guid, sizeof(uint64_t));
  g_net_server->Send((unsigned char*)szMsg.data(), p.length + 1 + sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 6, p.id);
  g_net_server->Send((unsigned char*)szMsg.data(), p.length + 1 + sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 6,
               players[FindIDOnList(say_to)].id);
  SPDLOG_INFO("({} WHISPER TO {}) {}", players[FindIDOnList(p.id.guid)].name.c_str(),
              players[FindIDOnList(say_to)].name.c_str(), (const char*)(p.data + 1 + sizeof(uint64_t)));
  szMsg.clear();
}

void CGmpServ::HandleCastSpell(Packet p, bool Target){
	size_t forbiden_id=0;
	std::string szMsg;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p.id.guid)==forbiden_id) return;
	else if(!players[FindIDOnList(p.id.guid)].is_ingame) return;
	szMsg.reserve(1024);
	if(!Target){
		*((unsigned char*)szMsg.data())=p.data[0];
		*((unsigned char*)(szMsg.data()+sizeof(uint64_t)+1))=p.data[1];
		memcpy((char*)szMsg.data()+1,(const char*)(&p.id.guid), sizeof(uint64_t));
		for(size_t i=0; i<players.size(); i++){
			if(!memcmp(&players[i].id, &p.id.guid, sizeof(uint64_t))) continue;
			if(players[i].is_ingame) g_net_server->Send((unsigned char*)szMsg.data(), p.length+sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0, players[i].id);
		}
	}
	else{
		uint64_t TargetId;
		memcpy(&TargetId, p.data+1, 8);
		if(FindIDOnList(TargetId)==forbiden_id) return;
		else if(!players[FindIDOnList(TargetId)].is_ingame) return;
		*((unsigned char*)szMsg.data())=p.data[0];
		memcpy((char*)szMsg.data()+sizeof(uint64_t)+1, p.data+1, 8);
		*((unsigned char*)(szMsg.data()+sizeof(uint64_t)+9))=p.data[9];
		memcpy((char*)szMsg.data()+1,(const char*)(&p.id.guid), sizeof(uint64_t));
		for(size_t i=0; i<players.size(); i++){
			if(!memcmp(&players[i].id, &p.id.guid, sizeof(uint64_t))) continue;
			if(players[i].is_ingame) g_net_server->Send((unsigned char*)szMsg.data(), p.length+sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0, players[i].id);
		}
	}
	memset((char*)szMsg.data(), 0, 1024);
	szMsg.clear();
}

void CGmpServ::HandleDropItem(Packet p){
	size_t forbiden_id=0;
	std::string szMsg;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p.id.guid)==forbiden_id) return;
	else if(!players[FindIDOnList(p.id.guid)].is_ingame) return;
	szMsg.reserve(1024);
	*((unsigned char*)szMsg.data())=p.data[0];
	memcpy((char*)szMsg.data()+sizeof(uint64_t)+1, p.data+1, 2);
	memcpy((char*)szMsg.data()+sizeof(uint64_t)+3, p.data+3, 2);
	memcpy((char*)szMsg.data()+1,(const char*)(&p.id.guid), sizeof(uint64_t));
	for(size_t i=0; i<players.size(); i++){
		if(!memcmp(&players[i].id, &p.id.guid, sizeof(uint64_t))) continue;
		if(players[i].is_ingame) g_net_server->Send((unsigned char*)szMsg.data(), p.length+sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0, players[i].id);
	}
	short amount;
	memcpy(&amount, p.data+3, 2);
	SPDLOG_INFO("{} DROPPED ITEM. AMOUNT: {}", players[FindIDOnList(p.id.guid)].name, amount);
	szMsg.clear();
}

void CGmpServ::HandleTakeItem(Packet p){
	if(!config_.Get<bool>("allow_dropitems")) return;
	size_t forbiden_id=0;
	std::string szMsg;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p.id.guid)==forbiden_id) return;
	else if(!players[FindIDOnList(p.id.guid)].is_ingame) return;
	szMsg.reserve(1024);
	*((unsigned char*)szMsg.data())=p.data[0];
	memcpy((char*)szMsg.data()+sizeof(uint64_t)+1, p.data+1, 2);
	memcpy((char*)szMsg.data()+1,(const char*)(&p.id.guid), sizeof(uint64_t));
	for(size_t i=0; i<players.size(); i++){
		if(!memcmp(&players[i].id, &p.id.guid, sizeof(uint64_t))) continue;
		if(players[i].is_ingame) g_net_server->Send((unsigned char*)szMsg.data(), p.length+sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0, players[i].id);
	}
	SPDLOG_INFO("{} TOOK ITEM.", players[FindIDOnList(p.id.guid)].name);
	szMsg.clear();
}

void CGmpServ::HandleRMConsole(Packet p){
	std::string szLog;
	size_t forbiden_id=0;
	szLog.reserve(512);
	forbiden_id= ~forbiden_id;
	if(forbiden_id==FindIDOnList(p.id.guid)) return;
	size_t rid=FindIDOnList(p.id.guid);
	if(!memcmp("passwd ", p.data+1, 7)){
		auto admin_passwd = config_.Get<std::string>("admin_passwd");
		if(memcmp(p.data+8, admin_passwd.c_str(), admin_passwd.length()+1)){
			players[rid].admin_passwd++;
			if(players[rid].admin_passwd>3){
				SendDisconnectionInfo(players[rid].id.guid);
				g_net_server->AddToBanList(players[rid].id, 1800000);
				SPDLOG_WARN("[#] Too much failed authorizations(admin permissions). Banned {}@{}.", players[rid].name, players[rid].id.guid);
				DeleteFromPlayerList(players[rid].id);
			}
			goto eoh;
		}
		players[rid].admin_passwd=0;
		players[rid].has_admin=1;
		*((unsigned char*)szLog.data())=PT_RCON;
		memcpy((char*)szLog.data()+1, AG, strlen(AG)+1);
		g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(AG)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
	}
	else if(!memcmp("login ", p.data+1, 6)){
		if(!players[rid].is_ingame){
			g_net_server->AddToBanList(p.id, 1800000);
			DeleteFromPlayerList(players[rid].id);
		} else{
			char cmd[16];
			char nickname[32];
			char password[64];
			if(sscanf((const char*)p.data+1, "%s%s%s", cmd, nickname, password)==3){
				switch(CPermissions::GetInstance()->IsModerator(nickname, password)){
					case 1:
						players[rid].moderator_passwd=0;
						players[rid].moderator=CPermissions::GetInstance()->GetModerator(nickname);
						*((unsigned char*)szLog.data())=PT_RCON;
						memcpy((char*)szLog.data()+1, AG, strlen(AG)+1);
						g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(AG)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
					case 0:	//lol
						break;
					case -1:
						players[rid].moderator_passwd++;
						if(players[rid].moderator_passwd>3){
							SendDisconnectionInfo(players[rid].id.guid);
							g_net_server->AddToBanList(players[rid].id, 1800000);
							SPDLOG_WARN("[#] Too much failed authoraztions(moderator permissions). Banned {}@{}.", players[rid].name, players[rid].id.guid);
							DeleteFromPlayerList(players[rid].id);
						}
						break;
				}
			}
		}
	}
	else if((players[rid].has_admin) || (players[rid].moderator)){
		//jeśli jest adminem/moderatorem to przetwarzaj
		//TO DO
		if(!memcmp(p.data+1, SAY, strlen(SAY))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(SAY)) goto eoh;
			SPDLOG_INFO("{}@{} say {}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid, (const char*)p.data+5);
			*((unsigned char*)szLog.data())=PT_SRVMSG;
			memcpy((char*)szLog.data()+1, p.data+5, p.length-5);
			for(size_t i=0; i<players.size();i++){
				if(players[i].is_ingame) g_net_server->Send((unsigned char*)szLog.data(), p.length-4, MEDIUM_PRIORITY, RELIABLE, 9, players[i].id);
			}
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, 4);
			g_net_server->Send((unsigned char*)szLog.data(), 5, MEDIUM_PRIORITY, RELIABLE, 9, p.id);
		} else if(!memcmp(p.data+1, KICK,strlen(KICK))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(KICK)) goto eoh;
			size_t kid=forbiden_id;
			for(size_t i=0; i<players.size(); i++){
				if(players[i].is_ingame){
					if(!memcmp(players[i].name.c_str(), p.data+6, p.length-6)){
						if(!players[i].has_admin) kid=i;
						break;
					}
				}
			}
			if(kid==forbiden_id){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
				goto eoh;
			}
			SendDisconnectionInfo(players[kid].id.guid);
			SPDLOG_INFO("{}@{} kick {}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid, players[kid].name.c_str());
			g_net_server->AddToBanList(players[kid].id, 5000);
			DeleteFromPlayerList(players[kid].id);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
		} else if(!memcmp(p.data+1, BAN,strlen(BAN))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(BAN)) goto eoh;
			size_t bid=forbiden_id;
			for(size_t i=0; i<players.size(); i++){
				if(players[i].is_ingame){
					if(!memcmp(players[i].name.c_str(), p.data+5, p.length-5)){
						if((!players[i].has_admin) && (!players[i].moderator)) bid=i;
						break;
					}
				}
			}
			if(bid==forbiden_id){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
				goto eoh;
			}
			SendDisconnectionInfo(players[bid].id.guid);
			SPDLOG_WARN("{}@{} ban {}@{}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), 
									p.id.guid, players[bid].name.c_str(), players[bid].id.guid);
			g_net_server->AddToBanList(players[bid].id, 0);
			ban_list.push_back(g_net_server->GetPlayerIp(players[bid].id));
			DeleteFromPlayerList(players[bid].id);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
		} else if(!memcmp(p.data+1, BAN_IP,strlen(BAN_IP))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(BAN_IP)) goto eoh;
			if(g_net_server->IsBanned((char*)p.data+7)){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, IPISBANNED, strlen(IPISBANNED)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(IPISBANNED)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
				goto eoh;
			}
			std::stack<sPlayer*> ip_players;
			for(size_t i=0; i<players.size(); i++){
				if(!memcmp(g_net_server->GetPlayerIp(players[i].id).c_str(), p.data+7, p.length-7)){
					ip_players.push(&players[i]);
				}
			}
			if(!ip_players.empty()){
				do{
					SendDisconnectionInfo(ip_players.top()->id.guid);
					SPDLOG_WARN("{}@{} banned because of IP by {}@{}", ip_players.top()->name, (char*)p.data+7, (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid);
					DeleteFromPlayerList(ip_players.top()->id);
					ip_players.pop();
				} while(!ip_players.empty());
			}
			g_net_server->AddToBanList((char*)p.data+7, 0);
			ban_list.push_back((char*)p.data+7);
			SPDLOG_WARN("{}@{} ban ip: {}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid, (char*)p.data+7);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
		} else if(!memcmp(p.data+1, UNBAN,strlen(UNBAN))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(UNBAN)) goto eoh;
			if(g_net_server->IsBanned((char*)p.data+7)){
				SPDLOG_WARN("{}@{} unbanned following IP: {}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid, (char*)p.data+7);
				g_net_server->RemoveFromBanList((char*)p.data+7);
			} else{
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, IPNOTBANNED, strlen(IPNOTBANNED)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(IPNOTBANNED)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
			}
		} else if(!memcmp(p.data+1, SAVEBANS,strlen(SAVEBANS)-1)){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(SAVEBANS)) goto eoh;
			SaveBanList();
			sprintf((char*)szLog.data(), "%s@%s saved ban list!", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
		} else if(!memcmp(p.data+1, MUTE,strlen(MUTE))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(MUTE)) goto eoh;
			size_t mid=forbiden_id;
			for(size_t i=0; i<players.size(); i++){
				if(players[i].is_ingame){
					if(!memcmp(players[i].name.c_str(), p.data+6, p.length-6)){
						mid=i;
						break;
					}
				}
			}
			if(mid==forbiden_id){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
			} else{
				players[mid].mute= ~players[mid].mute;
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
			}
		} else if(!memcmp(p.data+1, SET_TIME,strlen(SET_TIME))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(SET_TIME)) goto eoh;
			char buffer[16];
			unsigned short h,m;
			if(sscanf((char*)p.data+1, "%s%hu%hu", buffer, &h, &m)==3){
				GothicClock::Time new_time;
				new_time.hour_=static_cast<unsigned char>(h);
				new_time.min_=static_cast<unsigned char>(m);
				new_time.day_++;
				if(new_time.day_ > 30000) new_time.day_=1;
				for(size_t i=0; i<players.size(); i++){
					if(players[i].is_ingame){
						*((unsigned char*)szLog.data())=PT_GAME_INFO;
						memcpy((char*)szLog.data()+1, &new_time.day_, 4);
						*((unsigned char*)szLog.data()+5)=(unsigned char)config_.Get<std::int32_t>("game_mode");
						*((unsigned char*)szLog.data()+6)=0;
						if(config_.Get<bool>("quick_pots")) *((unsigned char*)szLog.data()+6)|=QUICK_POTS;
						if(config_.Get<bool>("allow_dropitems")) *((unsigned char*)szLog.data()+6)|=DROP_ITEMS;
						g_net_server->Send((unsigned char*)szLog.data(), 7, IMMEDIATE_PRIORITY, RELIABLE, 9, players[i].id);
					}
				}
				clock_->UpdateTime(new_time);
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
			} else{
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, INVALIDPARAMETER, strlen(INVALIDPARAMETER)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(INVALIDPARAMETER)), MEDIUM_PRIORITY, RELIABLE, 9, p.id);
			}
		} else if(!memcmp(p.data+1, KILL, strlen(KILL))){
			if(!players[rid].has_admin) if(!CPermissions::GetInstance()->IsPermitted(KILL)) goto eoh;
			sPlayer *n00b=FindPlayer((const char*)p.data+1+strlen(KILL));
			if(n00b){
				n00b->health=0;
				n00b->tod=time(NULL);
				int r=rand()%MAX_KILL_TXT;
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, KILLED[r], strlen(KILLED[r])+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(KILLED[r])+1), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
				SendDeathInfo(n00b->id.guid);
				SPDLOG_WARN("{}@{} killed {}@{}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid, n00b->name, n00b->id.guid);
			} else{
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
			}
		} else if(!memcmp(p.data+1, LOOP_MSG, strlen(LOOP_MSG))){
			if(!players[rid].has_admin) if(!CPermissions::GetInstance()->IsPermitted(LOOP_MSG)) goto eoh;
			if(!loop_msg.empty()) loop_msg.clear();
			loop_msg=((char*)p.data+strlen(LOOP_MSG)+1);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
			SPDLOG_WARN("{}@{} changed loop message to: {}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid,loop_msg);
		} else if(!memcmp(p.data+1, MOD_ADD, strlen(MOD_ADD))){
			if(!players[rid].has_admin) goto eoh;
			char b[16];
			char n[128];
			if(sscanf((const char*)p.data+1, "%s%s", b, n)!=2){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, INVALIDPARAMETER, strlen(INVALIDPARAMETER)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(INVALIDPARAMETER)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
				goto eoh;
			}
			CPermissions::GetInstance()->AddModerator(n, "\x7");
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
			SPDLOG_WARN("ADMIN@{} added moderator: {}", p.id.guid, n);
		} else if(!memcmp(p.data+1, MOD_DEL, strlen(MOD_DEL))){
			if(!players[rid].has_admin) goto eoh;
			char b[16];
			char n[128];
			if(sscanf((const char*)p.data+1, "%s%s", b, n)!=2){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, INVALIDPARAMETER, strlen(INVALIDPARAMETER)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(INVALIDPARAMETER)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
				goto eoh;
			}
			CPermissions::GetInstance()->DeleteModerator(n);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
			SPDLOG_WARN("ADMIN@{} deleted moderator: {}", p.id.guid, n);
		} else if(!memcmp(p.data+1, MOD_SET, strlen(MOD_SET))){
			if(!players[rid].has_admin) goto eoh;
			char b[16];
			char n[128];
			char pass[128];
			if(sscanf((const char*)p.data+1, "%s%s%s", b,n,pass)!=3){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, INVALIDPARAMETER, strlen(INVALIDPARAMETER)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(INVALIDPARAMETER)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
				goto eoh;
			}
			if(CPermissions::GetInstance()->SetPassword(n,pass)){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
				SPDLOG_WARN("ADMIN@{} changed moderator {} password to {}", p.id.guid, n, pass);
			} else{
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
				goto eoh;
			}
		} else if(!memcmp(p.data+1, HP_REGEN, strlen(HP_REGEN))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(HP_REGEN)) goto eoh;
			auto hp_regeneration = config_.Get<std::int32_t>("hp_regeneration");
			if(sscanf((const char*)p.data+1+strlen(HP_REGEN), "%d", &hp_regeneration)!=1){
				goto eoh;
			}
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
			SPDLOG_WARN("{}@{} changed hp regeneration: {}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid, hp_regeneration);
			goto eoh;
			//sprintf((char*)szLog.data()+1, "AD
		}else if(!memcmp(p.data+1, MP_REGEN, strlen(MP_REGEN))){
			if(!players[rid].has_admin) if(!CPermissions::GetInstance()->IsPermitted(MP_REGEN)) goto eoh;
			auto mp_regeneration = config_.Get<std::int32_t>("mp_regeneration");
			if(sscanf((const char*)p.data+1+strlen(MP_REGEN), "%d", &mp_regeneration)!=1) goto eoh;
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
			SPDLOG_WARN("{}@{} changed mp regeneration: {}", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p.id.guid, mp_regeneration);
			for(size_t i=0; i<players.size(); i++) if(players[i].is_ingame) SendGameInfo(players[i].id);
			goto eoh;
		}else{
			//cool story, bro!
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, WTF, strlen(WTF)+1);
			g_net_server->Send((unsigned char*)szLog.data(), static_cast<int>(2+strlen(WTF)), MEDIUM_PRIORITY, RELIABLE, 13, p.id);
		}
	}
eoh:
	szLog.clear();
}

void MakeHTTPReq(char *file)
{
	char listAddr[512] = {};
	sprintf(listAddr, "http://%s", lobbyAddress);
	httplib::Client cli(listAddr);
	cli.Get(file);
}

void CGmpServ::AddToPublicListHTTP()
{
	using namespace std::chrono_literals;

  char* szBuff = new char[256];
  memset(szBuff, 0, 256);
  while (pSrv)
  {
    if (pSrv->IsPublic())
    {
      auto server_name = pSrv->config_.Get<std::string>("name");
      for (size_t i = 0; i <= strlen(server_name.c_str()); i++)
        if ((*((unsigned char*)server_name.c_str() + i) < 0x20) &&
            (*((unsigned char*)server_name.c_str() + i) != 0x07))
          *((unsigned char*)server_name.data() + i) = 0;
      memset(szBuff, 0, 256);
      sprintf(
          szBuff, "%s?sn=%s&port=%d&crt=%u&mx=%d&map=%s", lobbyFile, server_name.c_str(),
          pSrv->config_.Get<std::int32_t>("port"), static_cast<unsigned int>(pSrv->players.size()),
          pSrv->config_.Get<std::int32_t>("slots"), pSrv->config_.Get<std::string>("map").c_str());
      MakeHTTPReq(szBuff);
    }
    std::this_thread::sleep_for(5s);
  }
  delete[] szBuff;
}

void CGmpServ::HandleGameInfo(Packet p){
	SendGameInfo(p.id);
}

//void CGmpServ::HandleGameInfo(Packet p){
void CGmpServ::SendGameInfo(Net::PlayerId who){
	std::string szData;
	int len=7;
	szData.reserve(64);
	*((unsigned char*)szData.data())=PT_GAME_INFO;
	GothicClock::Time game_time = clock_->GetTime();
	memcpy((char*)szData.data()+1, &game_time, 4);
	*((unsigned char*)szData.data()+5)=(unsigned char)config_.Get<std::int32_t>("game_mode");;
	*((unsigned char*)szData.data()+6)=0;
	if(config_.Get<bool>("quick_pots")) *((unsigned char*)szData.data()+6)|=QUICK_POTS;
	if(config_.Get<bool>("allow_dropitems")) *((unsigned char*)szData.data()+6)|=DROP_ITEMS;
	if(config_.Get<bool>("hide_map")) *((unsigned char*)szData.data()+6)|=HIDE_MAP;
	auto mp_regeneration = (short)config_.Get<std::int32_t>("mp_regeneration");
	if(mp_regeneration > 0){ *((unsigned char*)szData.data()+6)|=MANA_REGENERATION; memcpy((char*)szData.data()+len, &mp_regeneration, 2); len+=2;}
	g_net_server->Send((unsigned char*)szData.data(), len, MEDIUM_PRIORITY, RELIABLE, 9, who);
	szData.clear();
}

void CGmpServ::HandleMapNameReq(Packet p)
{
  std::string szData;
  szData.reserve(256);
  *((unsigned char*)szData.data()) = PT_MAP_NAME;
  auto map = config_.Get<std::string>("map");
  memcpy((char*)szData.data() + 1, map.c_str(), strlen(map.data()) + 1);
  g_net_server->Send((char*)szData.data(), static_cast<int>(2 + strlen(map.data())), MEDIUM_PRIORITY,
               RELIABLE, 9, p.id);
  szData.clear();
}
void CGmpServ::SendDisconnectionInfo(uint64_t disconnected_id)
{
  std::string szData;
  szData.reserve(32);
  *((unsigned char*)szData.data()) = PT_LEFT_GAME;
  memcpy((char*)szData.data() + 1, &disconnected_id, sizeof(uint64_t));
  for (size_t i = 0; i < players.size(); i++)
  {
    if (players[i].is_ingame)
      if (players[i].id.guid != disconnected_id)
      {
        g_net_server->Send(szData.data(), 1 + sizeof(uint64_t), IMMEDIATE_PRIORITY, RELIABLE, 8,
                     players[i].id);
      }
  }
  szData.clear();
}

bool CGmpServ::IsPublic()
{
  return (config_.Get<bool>("public")) ? true : false;
}

void CGmpServ::DoRespawns(){
	auto respawn_time = config_.Get<std::int32_t>("respawn_time_seconds");
	if(respawn_time > 0){
		if(respawn_time&0x80000000){
			//last stand
			switch(config_.Get<std::int32_t>("game_mode")){
				case 1:	//TDM
					if((!last_stand_timer) && (players.size())){
						const char *living_team=NULL;
						size_t living_players=0, total_ingame=0;
						for(size_t i=0; i<players.size(); i++){
							if(players[i].is_ingame) total_ingame++;
						}
						for(size_t i=0; i<players.size(); i++){
							if((players[i].is_ingame) && (!players[i].tod)){
								living_players++;
								if(!living_team) living_team=classmgr->class_array[players[i].char_class].team_name;
								else{
									if(memcmp(living_team, classmgr->class_array[players[i].char_class].team_name, strlen(living_team)+1)) return;
								}
							}
						}
						if((!total_ingame) || (living_players==total_ingame)) return;
						last_stand_timer=time(NULL)+8;
					} else{
						if((last_stand_timer<=time(NULL)) && (players.size())){
							for(size_t i=0; i<players.size(); i++){
								if((players[i].is_ingame) && (players[i].tod)){
									players[i].tod=0;
									players[i].health=classmgr->class_array[players[i].char_class].abilities[HP];
									SendRespawnInfo(players[i].id.guid);
								}
							}
							last_stand_timer=0;
						}
					}
					break;
				default:
					{
						size_t living_players=0;
						for(size_t i=0; i<players.size(); i++){
							if((players[i].is_ingame) && (!players[i].tod)) living_players++;
						}
						if(living_players>1) last_stand_timer=0;
						if((living_players<=1) && (!last_stand_timer)){
							last_stand_timer=time(NULL)+4;
						}
						if((living_players<=1) && (last_stand_timer<=time(NULL))){	//do resurrection; http://www.youtube.com/watch?v=J5t0iLqqgiI&t=55s
							for(size_t i=0; i<players.size(); i++){
								if((players[i].is_ingame) && (players[i].tod)){
									players[i].flags=0;
									players[i].tod=0;
									players[i].health=classmgr->class_array[players[i].char_class].abilities[HP];
									SendRespawnInfo(players[i].id.guid);
								}
							}
							last_stand_timer=0;
						}
					}
					break;
			}
		}
		else{
			//timed respawn
			for(size_t i=0; i<players.size(); i++){
				if((players[i].is_ingame) && (players[i].tod)){
					if(players[i].tod+respawn_time<time(NULL)){
						players[i].flags=0;
						players[i].tod=0;
						players[i].health=classmgr->class_array[players[i].char_class].abilities[HP];
						SendRespawnInfo(players[i].id.guid);
					}
				}
			}
		}
	}
}

void CGmpServ::SendSpamMessage(){
	auto loop_time = config_.Get<std::int32_t>("message_of_the_day_interval_seconds");
	if(loop_time > 0){
		if(spam_time<time(NULL)){
			spam_time=time(NULL)+loop_time;
			if(players.size()){
				std::string szData;
				szData.reserve(512);
				*((unsigned char*)szData.data())=PT_SRVMSG;
				memcpy((char*)szData.data()+1, loop_msg.c_str(), loop_msg.length()+1);
				for(size_t i=0; i<players.size(); i++){
					if(players[i].is_ingame) g_net_server->Send(szData.data(), static_cast<int>(loop_msg.length()+2), MEDIUM_PRIORITY, UNRELIABLE, 11, players[i].id);
				}
				szData.clear();
			}
		}
	}
}

void CGmpServ::SendDeathInfo(uint64_t deadman){
	unsigned char buffer[9];
	buffer[0]=PT_DODIE;
	memcpy(buffer+1, &deadman, 8);
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame) g_net_server->Send((unsigned char*)buffer, 9, IMMEDIATE_PRIORITY, RELIABLE, 13, players[i].id);
	}
}

void CGmpServ::SendRespawnInfo(uint64_t luckyguy){
	unsigned char buffer[9];
	buffer[0]=PT_RESPAWN;
	memcpy(buffer+1, &luckyguy, 8);
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame) g_net_server->Send((unsigned char*)buffer, 9, IMMEDIATE_PRIORITY, RELIABLE, 13, players[i].id);
	}
}

CGmpServ::sPlayer* CGmpServ::FindPlayer(const char *nickname){
	sPlayer* ret=NULL;
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame){
			if(!memcmp(nickname, players[i].name.c_str(), players[i].name.length()+1)){
				ret=&players[i];
				break;
			}
		}
	}
	return ret;
}

void CGmpServ::RegenerationHPMP(){	//hp is server side
	auto hp_regeneration = (short)config_.Get<std::int32_t>("hp_regeneration");
	if(!(hp_regeneration)) return;
	if(regen_time==time(NULL)) return;
	regen_time=time(NULL);
	for(size_t i=0; i<this->players.size(); i++){
		if((players[i].is_ingame) && (players[i].health>0)){
			players[i].health+=hp_regeneration;
			if(players[i].health>classmgr->class_array[players[i].char_class].abilities[HP]) players[i].health=classmgr->class_array[players[i].char_class].abilities[HP];
			if(players[i].health<=0){
				players[i].tod=time(NULL);
				SendDeathInfo(players[i].id.guid);
			}
		}
	}
}