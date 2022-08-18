
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
#include "CLog.h"
#include "CPermissions.h"
#include "CCompression.h"
#include <RakThread.h>
#include <fstream>
#include <string>
#include <stack>
#include <cstdlib>
#include <httplib.h>

CGmpServ *pSrv=NULL;
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

CGmpServ::CGmpServ(const char* password, int argc, char **argv){
/*#ifndef WIN32
	if(this->daemon) System::MakeMeDaemon(false);
#endif*/
	this->log = new CLog(this->log_mode, this->logfile.c_str());
	server = RakNet::RakPeerInterface::GetInstance();
	this->serverPassword.append(password);
	pSrv=this;
	arg_count=argc;
	arg_vec=argv;
}

CGmpServ::CGmpServ(int port, const char * password, unsigned short maxConnections)
{
/*#ifndef WIN32
	if(this->daemon) System::MakeMeDaemon(false);
#endif*/
	this->log = new CLog(this->log_mode, this->logfile.c_str());
	server = RakNet::RakPeerInterface::GetInstance();
	this->game_port = static_cast<unsigned short>(port);
	this->serverPassword.append(password);
	this->slots = (int)maxConnections;
	pSrv=this;
}

CGmpServ::~CGmpServ(void)
{
	delete spawnlist;
	delete classmgr;
	if(wb_mgr) delete wb_mgr;
	delete this->log;
	server->Shutdown(300);
	RakNet::RakPeerInterface::DestroyInstance(server);
	pSrv=NULL;
}
bool CGmpServ::Init()
{
#ifndef WIN32
    if(this->daemon) System::MakeMeDaemon(false);
#endif
	if(arg_count>1) ParseCmdLine(arg_count, arg_vec);
	CPermissions *perms=new CPermissions();
	perms=NULL;
	this->spam_time=time(NULL)+10;
	this->spawnlist = new CSpawnList();
	this->wb_mgr = new CWBFile();
	this->classmgr = new CClassManager;
	server->SetIncomingPassword(this->serverPassword.c_str(), (int)this->serverPassword.length());
	server->SetTimeoutTime(1000,RakNet::UNASSIGNED_SYSTEM_ADDRESS);  //1s styknie nie?
	server->SetMaximumIncomingConnections(this->slots);
	RakNet::SocketDescriptor socketDescriptors[2];

	socketDescriptors[0].port=game_port;
	socketDescriptors[0].socketFamily=AF_INET; // Klient
	if(!this->host_addr.empty()) memcpy(socketDescriptors[0].hostAddress, this->host_addr.c_str(), this->host_addr.length()+1);

	bool b = server->Startup(this->slots, socketDescriptors, 1)==RakNet::RAKNET_STARTED;
	if (!b) return false;
	
	server->SetOccasionalPing(true);
	server->SetUnreliableTimeout(1000);
	server->GetSockets(sockets);
	log->Write(LOG_NORMAL, "GMP server(Walpurgisnacht) started!");
	LoadBanList();
	RakNet::RakThread::Create(STime::RunClock, NULL);
	RakNet::RakThread::Create(CGmpServ::AddToPublicListHTTP, NULL);
	this->last_stand_timer=0;
	return true;
}

bool CGmpServ::Send(std::string message)	//ktoś z was ma zamiar tej funkcji używać?
{
	server->Send(message.c_str(), (int)message.length()+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	return true;
}
bool CGmpServ::Receive(sPacket & packet)
{
	std::string szLog;
	szLog.reserve(256);
	RakNet::Packet *p = server->Receive();
	if(p==NULL)return false;
	unsigned char packetIdentifier = GetPacketIdentifier(p);
	switch (packetIdentifier)
	{
	case ID_DISCONNECTION_NOTIFICATION:
		SendDisconnectionInfo(p->guid.g);
		DeleteFromPlayerList(p->guid);
		sprintf((char*)szLog.data(), "%s disconnected. Still connected %u users.", p->systemAddress.ToString(true), static_cast<unsigned int>(players.size()));
		log->Write(LOG_NORMAL, szLog.c_str());
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
			pl.id=p->guid;
			pl.mute=0;
			players.push_back(pl);
		}
		sprintf((char*)szLog.data(), "ID_NEW_INCOMING_CONNECTION from %s with GUID %s. Now we have %u connected users.", p->systemAddress.ToString(true), p->guid.ToString(), static_cast<unsigned int>(players.size()));
		log->Write(LOG_NORMAL, szLog.c_str());
		break;
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		log->Write(LOG_WARNING, "ID_INCOMPATIBLE_PROTOCOL_VERSION");
		break;
	case ID_CONNECTION_LOST:
		SendDisconnectionInfo(p->guid.g);
		DeleteFromPlayerList(p->guid);
		sprintf((char*)szLog.data() ,"Connection lost from %s. Still connected %u users.", p->systemAddress.ToString(true), static_cast<unsigned int>(players.size()));
		log->Write(LOG_WARNING, szLog.c_str());
		break;
	case PT_REQUEST_FILE_LENGTH:
	case PT_REQUEST_FILE_PART:
		HandleFileReq(p);
		break;
	case PT_WHOAMI: //zwraca id gracza(czyli jego guid)
		{
			unsigned char val[9];
			val[0]=PT_WHOAMI;
			memcpy(val+1, &p->guid.g, sizeof(uint64_t));
			server->Send((const char*)val, 9, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p, false);
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
			if((!memcmp(p->data+2, map_md5, 16)) && (!p->data[1])){
				players[FindIDOnList(p->guid.g)].passed_crc_test=1;
				unsigned char val[2]={PT_CHECKSUM, 0xFF};
				server->Send((const char*)val, 2, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, p, false);
			}
			else{
				players.erase(players.begin()+FindIDOnList(p->guid.g));
				server->AddToBanList(p->systemAddress.ToString(false), 3600000);
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
	default:
		/*packet.type = CGmpServ::PT_MSG;
		packet.data.clear();
		packet.data.append((char *)p->data);*/
		sprintf((char*)szLog.data(),"(S)He or it try to do something strange. It's packet ID: %i\n", packetIdentifier);
		log->Write(LOG_WARNING, szLog.c_str());
		break;
	}
	server->DeallocatePacket(p);
	if(!szLog.empty()) szLog.clear();
	return true;
}
unsigned char CGmpServ::GetPacketIdentifier(RakNet::Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char) p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else
		return (unsigned char) p->data[0];
}

void CGmpServ::LoadBanList(){
	std::string szTmp;
	szTmp.reserve(32);
	std::ifstream ifs("bans.txt");
	if(ifs.is_open()){
		while(1){
			ifs.getline((char*)szTmp.data(), 32);
			if(ifs.eof()) break;
			if(szTmp.length()>=7){
				this->ban_list.push_back(szTmp.c_str());
			}
		}
		szTmp.clear();
		ifs.close();
	}
	else{
		log->Write(LOG_WARNING, "bans.txt which contains active IP bans not exist");
		return;
	}
	if(!this->ban_list.empty()){
		for(size_t i=0; i<this->ban_list.size(); i++){
			this->server->AddToBanList(this->ban_list[i].c_str(), 0);
		}
	}
	log->Write(LOG_NORMAL, "Active bans loaded.");
}

void CGmpServ::SaveBanList(){
	if(!this->ban_list.empty()){
		std::ofstream ofs("bans.txt");
		if(!ofs.is_open()){
			log->Write(LOG_ERROR, "Could not save active bans to file bans.txt!");
			return;
		}
		for(size_t i=0; i<this->ban_list.size(); i++){
			ofs << this->ban_list[i].c_str() << "\n";
		}
		ofs.clear();
	}
	log->Write(LOG_NORMAL, "Bans written to bans.txt.");
}

void CGmpServ::DeleteFromPlayerList(RakNet::RakNetGUID guid){
	for(size_t i=0; i<players.size(); i++){
		if(guid.g==players[i].id.g){
			if(players[i].is_ingame){
			}
			players.erase(players.begin()+i);
			break;
		}
	}
}

void CGmpServ::HandleFileReq(RakNet::Packet* p){
	//TO DO
	if(p){
		unsigned char val[1024];
		switch(p->data[0]){
			case PT_REQUEST_FILE_LENGTH:
				val[0]=PT_REQUEST_FILE_LENGTH;
				switch(p->data[1]){
					case CLASS_FILE:
						val[1]=CLASS_FILE;
						val[2]=static_cast<unsigned char>(classmgr->GetBufferSize()/1000);
						server->Send((const char*)val, 3, HIGH_PRIORITY, RELIABLE, 1, p, false);
						break;
					case SPAWN_FILE:
						val[1]=SPAWN_FILE;
						val[2]=static_cast<unsigned char>(spawnlist->GetBufferSize()/1000);
						server->Send((const char*)val, 3, HIGH_PRIORITY, RELIABLE, 1, p, false);
						break;
					case WB_FILE:
						if(!wb_mgr->GetBufferSize()){
							val[1]=NULL_SIZE;
							server->Send((const char*)val, 2, HIGH_PRIORITY, RELIABLE, 1, p, false);
						}else{
							val[1]=WB_FILE;
							unsigned short s=static_cast<unsigned short>(wb_mgr->GetBufferSize()/1000);
							memcpy(val+2, &s, 2);
							server->Send((const char*)val, 4, HIGH_PRIORITY, RELIABLE, 1, p, false);
						}
						break;
					default: //nie powinno się wykonać
						break;
				}
				break;
			case PT_REQUEST_FILE_PART:
				val[0]=PT_REQUEST_FILE_PART;
				switch(p->data[1]){
					case CLASS_FILE:	//deflate
						if(p->data[2]<static_cast<unsigned char>(classmgr->GetBufferSize()/1000)){
							unsigned int out_len=0;
							val[1]=CLASS_FILE;
							val[2]=p->data[2];
							CCompression::GetInstance()->Compress((unsigned char*)classmgr->GetFilePart((unsigned long)p->data[2]), val+3, (((classmgr->GetBufferSize()/1000)-1)==p->data[2])?classmgr->GetFileLength()%1000:1000, out_len);
							server->Send((const char*)val, 3+out_len, HIGH_PRIORITY, RELIABLE, 1, p, false);
						}
						break;
					case SPAWN_FILE:	//uncompressed
						if(p->data[2]<static_cast<unsigned char>(spawnlist->GetBufferSize()/1000)){
							val[1]=SPAWN_FILE;
							val[2]=p->data[2];
							memcpy(val+3, spawnlist->GetFilePart((unsigned long)p->data[2]), (int)((((spawnlist->GetBufferSize()/1000)-1)==p->data[2])?spawnlist->GetFileLength()%1000:1000));
							server->Send((const char*)val, 3+(int)((((spawnlist->GetBufferSize()/1000)-1)==p->data[2])?spawnlist->GetFileLength()%1000:1000), HIGH_PRIORITY, RELIABLE, 1, p, false);
						}
						break;
					case WB_FILE:	//deflate
						{
							unsigned short part;
							memcpy(&part, p->data+2, 2);
							if(part<static_cast<unsigned short>(wb_mgr->GetBufferSize()/1000)){
								unsigned int out_len=0;
								memcpy(val+2, p->data+2, 2);
								CCompression::GetInstance()->Compress(wb_mgr->GetFilePart(part), val+4, (((wb_mgr->GetBufferSize()/1000)-1)==part)?wb_mgr->GetFileLength()%1000:1000, out_len);
								server->Send((const char*)val, 4+out_len, HIGH_PRIORITY, RELIABLE, 1, p, false);
								//memcpy(val+4, wb_mgr->GetFilePart((unsigned long)p->data[2]), (int)((((wb_mgr->GetBufferSize()/1000)-1)==p->data[2])?wb_mgr->GetFileLength()%1000:1000));
								//server->Send((const char*)val, 4+(int)((((wb_mgr->GetBufferSize()/1000)-1)==p->data[2])?wb_mgr->GetFileLength()%1000:1000), HIGH_PRIORITY, RELIABLE, 1, p, false);
							}
						}
						break;
					default: //nie powinno się wykonać
						break;
				}
				break;
			default://jak ktoś próbuje kombinować to default powinien się wykonać
				break;
		}
	}
}

void CGmpServ::SomeoneJoinGame(RakNet::Packet* p){
	size_t pos_in_list=FindIDOnList(p->guid.g);
	if(!allow_modification){
		if(!players[pos_in_list].passed_crc_test){
			players.erase(players.begin()+pos_in_list); //nie ma szans, usuwamy z listy
			server->AddToBanList(p->systemAddress.ToString(false), 3600000);//i dorzucamy banana na 1h
			return;
		}
	}
	players[pos_in_list].tod=0;
	players[pos_in_list].char_class=p->data[1];
	players[pos_in_list].health=this->classmgr->class_array[p->data[1]].abilities[HP];
	memcpy(players[pos_in_list].pos, p->data+2, 12);
	memcpy(players[pos_in_list].nrot, p->data+14, 12);
	memcpy(&players[pos_in_list].left_hand, p->data+26, 2);
	memcpy(&players[pos_in_list].right_hand, p->data+28, 2);
	memcpy(&players[pos_in_list].armor, p->data+30, 2);
	memcpy(&players[pos_in_list].animation, p->data+32, 2);
	players[pos_in_list].head=p->data[34];
	players[pos_in_list].skin=p->data[35];
	players[pos_in_list].body=p->data[36];
	players[pos_in_list].walkstyle=p->data[37];
	players[pos_in_list].name=(const char*)(p->data+38);
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
	nick_packet[0]=PT_YOUR_NICKNAME;
	server->Send((const char*)nick_packet, 2+(int)players[pos_in_list].name.length(), IMMEDIATE_PRIORITY, RELIABLE, 0, p->systemAddress, false);
	std::string inform_packet;
	inform_packet.reserve(1024);
	char *szTmpPtr=(char*)inform_packet.c_str();
	szTmpPtr[0]=p->data[0];
	memcpy((char*)inform_packet.data()+(1+sizeof(uint64_t)), p->data+1, p->length-1);
	memcpy((char*)inform_packet.data()+1, &p->guid.g, sizeof(uint64_t));
	//informowanie innych o tym że ktoś dołączył się do gej party
	memcpy((char*)inform_packet.data()+(38+sizeof(uint64_t)), players[pos_in_list].name.c_str(), players[pos_in_list].name.length());
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame){
			server->Send(inform_packet.data(), 40+(int)players[pos_in_list].name.length()+sizeof(uint64_t), IMMEDIATE_PRIORITY, RELIABLE, 3, players[i].id, false);
		}
	}
	//generowanie i słanie pakietów o graczach(max 10 graczy per pakiet)
	memset((char*)inform_packet.data(), 0, 1024);
	unsigned char it=0;
	size_t szIt=1;
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame){
			it++;
			memcpy((char*)inform_packet.data()+szIt, &players[i].id.g, (sizeof(uint64_t))); szIt+=sizeof(uint64_t);
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
			server->Send(inform_packet.data(), (int)szIt, IMMEDIATE_PRIORITY, RELIABLE, 2, p, false);
			it=0;
			szIt=1;
		}
	}
	if(it>0){
		*(unsigned char*)(inform_packet.data())=PT_ALL_OTHERS;
		server->Send(inform_packet.data(), (int)szIt, IMMEDIATE_PRIORITY, RELIABLE, 2, p, false);
		it=0;
		szIt=1;
	}
	players[pos_in_list].is_ingame=1;
}

size_t CGmpServ::FindIDOnList(uint64_t guid){
	for(size_t i=0; i<players.size(); i++){
		if(guid==players[i].id.g) return i;
	}
	size_t ret=0;
	ret= ~ret;
	return ret;
}

void CGmpServ::HandlePlayerUpdate(RakNet::Packet* p){
	double rp, x1, y1;  //<- zmienne do sprawdzenia czy dana osoba mieści się w kole o r=5000.0f
	size_t pos_in_list=FindIDOnList(p->guid.g);
	size_t forbidenid=0;
	forbidenid= ~forbidenid;
	if(pos_in_list==forbidenid) return;
	memcpy(players[pos_in_list].pos, p->data+1, 12);
	memcpy(players[pos_in_list].nrot, p->data+13, 12);
	memcpy(&players[pos_in_list].left_hand, p->data+25, 2);
	memcpy(&players[pos_in_list].right_hand, p->data+27, 2);
	memcpy(&players[pos_in_list].armor, p->data+29, 2);
	memcpy(&players[pos_in_list].animation, p->data+31, 2);
	memcpy(&players[pos_in_list].mana, p->data+33, 2);
	players[pos_in_list].figth_pos=p->data[35];
	players[pos_in_list].spellhand=p->data[36];
	players[pos_in_list].headstate=p->data[37];
	memcpy(&players[pos_in_list].rangedeq, p->data+38, 2);
	memcpy(&players[pos_in_list].meleeeq, p->data+40, 2);
	
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
					memcpy((char*)msg.data()+szIt[0], &players[i].id.g, sizeof(uint64_t)); szIt[0]+=sizeof(uint64_t);
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
					memcpy((char*)msg.data()+szIt[1], &players[i].id.g, sizeof(uint64_t)); szIt[1]+=sizeof(uint64_t);
					memcpy((char*)msg.data()+szIt[1], &players[i].pos[0], 4); szIt[1]+=4; //float x,y
					memcpy((char*)msg.data()+szIt[1], &players[i].pos[2], 4); szIt[1]+=4;
				}
			}
			else{//to jesteśmy my, starczy tylko info o hp
				it[0]++;
				memcpy((char*)msg.data()+szIt[0], &p->guid.g, sizeof(uint64_t)); szIt[0]+=sizeof(uint64_t);
				memcpy((char*)msg.data()+szIt[0], &players[pos_in_list].health, 2); szIt[0]+=2;
			}
			if(it[0]==10){
				*(unsigned char*)(msg.data())=PT_ACTUAL_STATISTICS;
				server->Send(msg.data(), (int)szIt[0], IMMEDIATE_PRIORITY, UNRELIABLE, 0, p, false);
				it[0]=0;
				szIt[0]=1;
			}
			if(it[1]==60){
				*(unsigned char*)(msg.data()+1024)=PT_MAP_ONLY;
				server->Send(msg.data()+1024, (int)szIt[1]-1024, IMMEDIATE_PRIORITY, UNRELIABLE, 0, p, false);
				it[1]=0;
				szIt[1]=1025;
			}
		}
	}
	if(it[0]>0){
		*(unsigned char*)(msg.data())=PT_ACTUAL_STATISTICS;
		server->Send(msg.data(), (int)szIt[0], IMMEDIATE_PRIORITY, UNRELIABLE, 0, p, false);
		it[0]=0;
	}
	if(it[1]>0){
		*(unsigned char*)(msg.data()+1024)=PT_MAP_ONLY;
		server->Send(msg.data()+1024, (int)szIt[1]-1024, IMMEDIATE_PRIORITY, UNRELIABLE, 32, p, false);
		it[1]=0;
	}
}

void CGmpServ::MakeHPDiff(RakNet::Packet* p){
	size_t forbidenid=0, pwgd;
	uint64_t player_id;
	short diffed_hp;
	forbidenid= ~forbidenid;
	if(forbidenid==FindIDOnList(p->guid.g)) return;
	if(players[FindIDOnList(p->guid.g)].is_ingame){
		memcpy(&player_id, p->data+1, sizeof(uint64_t));
		memcpy(&diffed_hp, p->data+(1+sizeof(uint64_t)), 2);
		pwgd=FindIDOnList(player_id);
		if((pwgd==forbidenid) || (pwgd>=players.size())) return;
		if((players[pwgd].health<=0) || (players[pwgd].tod)) return;	//just ignore
		if(player_id==p->guid.g){
			if(players[pwgd].health) players[pwgd].health+=diffed_hp;
			if(players[pwgd].health<=0){
				players[pwgd].health=0;
				players[pwgd].tod=time(NULL);
				SendDeathInfo(players[pwgd].id.g);
			}
			if(players[pwgd].health>classmgr->class_array[players[pwgd].char_class].abilities[HP]) players[pwgd].health=classmgr->class_array[players[pwgd].char_class].abilities[HP];
		}
		else{
			if(be_unconcious_before_dead){
				size_t attacker=FindIDOnList(p->guid.g);
				switch(players[attacker].figth_pos){
					case 1:
					case 3:
					case 4: //od tych broni nasz zawodnik na pewno nie padnie
						if(players[pwgd].flags&PL_UNCONCIOUS){
							players[pwgd].flags&=0xFE;
							players[pwgd].tod=time(NULL);
							players[pwgd].health=0;
							SendDeathInfo(players[pwgd].id.g);
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
								SendDeathInfo(players[pwgd].id.g);
							}
						}
						break;
				}
			} else{
				players[pwgd].health+=diffed_hp;
				if(players[pwgd].health==1){
					players[pwgd].health=0;
					players[pwgd].tod=time(NULL);
					SendDeathInfo(players[pwgd].id.g);
				}
				if((players[pwgd].health==0) && (!players[pwgd].tod)){
					players[pwgd].tod=time(NULL);
					SendDeathInfo(players[pwgd].id.g);
				}
				if(players[pwgd].health<0){
					players[pwgd].health=0;
					players[pwgd].tod=time(NULL);
					SendDeathInfo(players[pwgd].id.g);
				}
				if(players[pwgd].health>classmgr->class_array[players[pwgd].char_class].abilities[HP]) players[pwgd].health=classmgr->class_array[players[pwgd].char_class].abilities[HP];
			}
		}
	}
	if((players[pwgd].health<=0) && (!players[pwgd].tod)){
		players[pwgd].tod=time(NULL);
		SendDeathInfo(players[pwgd].id.g);
	}
}

void CGmpServ::HandleNormalMsg(RakNet::Packet* p){
	size_t forbiden_id=0;
	std::string szMsg;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p->guid.g)==forbiden_id) return;
	else if(!players[FindIDOnList(p->guid.g)].is_ingame) return;
	if(players[FindIDOnList(p->guid.g)].mute) return;
	size_t stranger=FindIDOnList(p->guid.g);
	szMsg.reserve(1024);
	*((unsigned char*)szMsg.data())=p->data[0];
	memcpy((char*)szMsg.data()+sizeof(uint64_t)+1, p->data+1, p->length-1);
	memcpy((char*)szMsg.data()+1,(const char*)(&p->guid.g), sizeof(uint64_t));
	if(!real_chat) for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame) server->Send((const char*)szMsg.data(), p->length+sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 5, players[i].id, false);
	}
#define EQ(x) (players[i].pos[x]-players[stranger].pos[x])
	else for(size_t i=0;i<players.size(); i++){
		if(players[i].is_ingame){
			if((EQ(0)*EQ(0)+EQ(1)*EQ(1)<=25000.0f) && (EQ(0)*EQ(0)+EQ(2)*EQ(2)<=25000.0f) && (EQ(1)*EQ(1)+EQ(2)*EQ(2)<=25000.0f)){
				server->Send((const char*)szMsg.data(), p->length+sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 5, players[i].id, false);
			}
		}
	}
#undef EQ
	memset((char*)szMsg.data(), 0, 1024);
	sprintf((char*)szMsg.data(), "%s:%s", players[FindIDOnList(p->guid.g)].name.c_str(), (const char*)(p->data+1));
	log->Write(LOG_NORMAL, szMsg.c_str());
	szMsg.clear();
}

void CGmpServ::HandleWhisp(RakNet::Packet* p){
	size_t forbiden_id=0;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p->guid.g)==forbiden_id) return;
	else if(!players[FindIDOnList(p->guid.g)].is_ingame) return;
	uint64_t say_to;
	memcpy(&say_to, p->data+1, sizeof(uint64_t));
	if(FindIDOnList(say_to)==forbiden_id) return;
	std::string szMsg;
	szMsg.reserve(1024);
	*((char*)szMsg.data())=p->data[0];
	memcpy((char*)szMsg.data()+1+sizeof(uint64_t), p->data+1, p->length-1);
	memcpy((char*)(szMsg.data()+1), &p->guid.g, sizeof(uint64_t));
	server->Send((const char*)szMsg.data(), p->length+1+sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 6, p, false);
	server->Send((const char*)szMsg.data(), p->length+1+sizeof(uint64_t), LOW_PRIORITY, RELIABLE_ORDERED, 6, players[FindIDOnList(say_to)].id, false);
	memset((char*)szMsg.data(), 0, 1024);
	sprintf((char*)szMsg.data(), "(%s WHISPER TO %s) %s",players[FindIDOnList(p->guid.g)].name.c_str(),players[FindIDOnList(say_to)].name.c_str(), (const char*)(p->data+1+sizeof(uint64_t)));
	log->Write(LOG_NORMAL, szMsg.c_str());
	szMsg.clear();
}

void CGmpServ::HandleCastSpell(RakNet::Packet* p, bool Target){
	size_t forbiden_id=0;
	std::string szMsg;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p->guid.g)==forbiden_id) return;
	else if(!players[FindIDOnList(p->guid.g)].is_ingame) return;
	szMsg.reserve(1024);
	if(!Target){
		*((unsigned char*)szMsg.data())=p->data[0];
		*((unsigned char*)(szMsg.data()+sizeof(uint64_t)+1))=p->data[1];
		memcpy((char*)szMsg.data()+1,(const char*)(&p->guid.g), sizeof(uint64_t));
		for(size_t i=0; i<players.size(); i++){
			if(!memcmp(&players[i].id, &p->guid.g, sizeof(uint64_t))) continue;
			if(players[i].is_ingame) server->Send((const char*)szMsg.data(), p->length+sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0, players[i].id, false);
		}
	}
	else{
		uint64_t TargetId;
		memcpy(&TargetId, p->data+1, 8);
		if(FindIDOnList(TargetId)==forbiden_id) return;
		else if(!players[FindIDOnList(TargetId)].is_ingame) return;
		*((unsigned char*)szMsg.data())=p->data[0];
		memcpy((char*)szMsg.data()+sizeof(uint64_t)+1, p->data+1, 8);
		*((unsigned char*)(szMsg.data()+sizeof(uint64_t)+9))=p->data[9];
		memcpy((char*)szMsg.data()+1,(const char*)(&p->guid.g), sizeof(uint64_t));
		for(size_t i=0; i<players.size(); i++){
			if(!memcmp(&players[i].id, &p->guid.g, sizeof(uint64_t))) continue;
			if(players[i].is_ingame) server->Send((const char*)szMsg.data(), p->length+sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0, players[i].id, false);
		}
	}
	memset((char*)szMsg.data(), 0, 1024);
	szMsg.clear();
}

void CGmpServ::HandleDropItem(RakNet::Packet* p){
	size_t forbiden_id=0;
	std::string szMsg;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p->guid.g)==forbiden_id) return;
	else if(!players[FindIDOnList(p->guid.g)].is_ingame) return;
	szMsg.reserve(1024);
	*((unsigned char*)szMsg.data())=p->data[0];
	memcpy((char*)szMsg.data()+sizeof(uint64_t)+1, p->data+1, 2);
	memcpy((char*)szMsg.data()+sizeof(uint64_t)+3, p->data+3, 2);
	memcpy((char*)szMsg.data()+1,(const char*)(&p->guid.g), sizeof(uint64_t));
	for(size_t i=0; i<players.size(); i++){
		if(!memcmp(&players[i].id, &p->guid.g, sizeof(uint64_t))) continue;
		if(players[i].is_ingame) server->Send((const char*)szMsg.data(), p->length+sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0, players[i].id, false);
	}
	memset((char*)szMsg.data(), 0, 1024);
	short amount;
	memcpy(&amount, p->data+3, 2);
	sprintf((char*)szMsg.data(), "%s DROPPED ITEM. AMOUNT : %d", players[FindIDOnList(p->guid.g)].name.c_str(), amount);
	log->Write(LOG_NORMAL, szMsg.c_str());
	szMsg.clear();
}

void CGmpServ::HandleTakeItem(RakNet::Packet* p){
	if(!allow_dropitems) return;
	size_t forbiden_id=0;
	std::string szMsg;
	forbiden_id= ~forbiden_id;
	if(FindIDOnList(p->guid.g)==forbiden_id) return;
	else if(!players[FindIDOnList(p->guid.g)].is_ingame) return;
	szMsg.reserve(1024);
	*((unsigned char*)szMsg.data())=p->data[0];
	memcpy((char*)szMsg.data()+sizeof(uint64_t)+1, p->data+1, 2);
	memcpy((char*)szMsg.data()+1,(const char*)(&p->guid.g), sizeof(uint64_t));
	for(size_t i=0; i<players.size(); i++){
		if(!memcmp(&players[i].id, &p->guid.g, sizeof(uint64_t))) continue;
		if(players[i].is_ingame) server->Send((const char*)szMsg.data(), p->length+sizeof(uint64_t), HIGH_PRIORITY, RELIABLE, 0, players[i].id, false);
	}
	memset((char*)szMsg.data(), 0, 1024);
	sprintf((char*)szMsg.data(), "%s TOOK ITEM.", players[FindIDOnList(p->guid.g)].name.c_str());
	log->Write(LOG_NORMAL, szMsg.c_str());
	szMsg.clear();
}

void CGmpServ::HandleRMConsole(RakNet::Packet* p){
	std::string szLog;
	size_t forbiden_id=0;
	szLog.reserve(512);
	forbiden_id= ~forbiden_id;
	if(forbiden_id==FindIDOnList(p->guid.g)) return;
	size_t rid=FindIDOnList(p->guid.g);
	if(!memcmp("passwd ", p->data+1, 7)){
		if(memcmp(p->data+8, admin_passwd.c_str(), admin_passwd.length()+1)){
			players[rid].admin_passwd++;
			if(players[rid].admin_passwd>3){
				SendDisconnectionInfo(players[rid].id.g);
				server->AddToBanList(server->GetSystemAddressFromGuid(players[rid].id).ToString(false), 1800000);
				sprintf((char*)szLog.data(), "[#] Too much failed authorizations(admin permissions). Banned %s@%s.", players[rid].name.c_str(), server->GetSystemAddressFromGuid(players[rid].id).ToString());
				log->Write(LOG_WARNING, szLog.data());
				DeleteFromPlayerList(players[rid].id);
			}
			goto eoh;
		}
		players[rid].admin_passwd=0;
		players[rid].has_admin=1;
		*((unsigned char*)szLog.data())=PT_RCON;
		memcpy((char*)szLog.data()+1, AG, strlen(AG)+1);
		server->Send(szLog.data(), static_cast<int>(2+strlen(AG)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
	}
	else if(!memcmp("login ", p->data+1, 6)){
		if(!players[rid].is_ingame){
			server->AddToBanList(p->systemAddress.ToString(false), 1800000);
			DeleteFromPlayerList(players[rid].id);
		} else{
			char cmd[16];
			char nickname[32];
			char password[64];
			if(sscanf((const char*)p->data+1, "%s%s%s", cmd, nickname, password)==3){
				switch(CPermissions::GetInstance()->IsModerator(nickname, password)){
					case 1:
						players[rid].moderator_passwd=0;
						players[rid].moderator=CPermissions::GetInstance()->GetModerator(nickname);
						*((unsigned char*)szLog.data())=PT_RCON;
						memcpy((char*)szLog.data()+1, AG, strlen(AG)+1);
						server->Send(szLog.data(), static_cast<int>(2+strlen(AG)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
					case 0:	//lol
						break;
					case -1:
						players[rid].moderator_passwd++;
						if(players[rid].moderator_passwd>3){
							SendDisconnectionInfo(players[rid].id.g);
							server->AddToBanList(server->GetSystemAddressFromGuid(players[rid].id).ToString(false), 1800000);
							sprintf((char*)szLog.data(), "[#] Too much failed authoraztions(moderator permissions). Banned %s@%s.", players[rid].name.c_str(), server->GetSystemAddressFromGuid(players[rid].id).ToString());
							log->Write(LOG_WARNING, szLog.data());
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
		if(!memcmp(p->data+1, SAY, strlen(SAY))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(SAY)) goto eoh;
			sprintf((char*)szLog.data(), "%s@%s say %s", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false), (const char*)p->data+5);
			log->Write(LOG_NORMAL, szLog.data());
			*((unsigned char*)szLog.data())=PT_SRVMSG;
			memcpy((char*)szLog.data()+1, p->data+5, p->length-5);
			for(size_t i=0; i<players.size();i++){
				if(players[i].is_ingame) server->Send(szLog.data(), p->length-4, MEDIUM_PRIORITY, RELIABLE, 9, players[i].id, false);
			}
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, 4);
			server->Send(szLog.data(), 5, MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
		} else if(!memcmp(p->data+1, KICK,strlen(KICK))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(KICK)) goto eoh;
			size_t kid=forbiden_id;
			for(size_t i=0; i<players.size(); i++){
				if(players[i].is_ingame){
					if(!memcmp(players[i].name.c_str(), p->data+6, p->length-6)){
						if(!players[i].has_admin) kid=i;
						break;
					}
				}
			}
			if(kid==forbiden_id){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
				goto eoh;
			}
			SendDisconnectionInfo(players[kid].id.g);
			sprintf((char*)szLog.data(), "%s@%s kick %s", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false), players[kid].name.c_str());
			log->Write(LOG_NORMAL, szLog.data());
			server->AddToBanList(server->GetSystemAddressFromGuid(players[kid].id).ToString(false), 5000);
			DeleteFromPlayerList(players[kid].id);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p->systemAddress, false);
		} else if(!memcmp(p->data+1, BAN,strlen(BAN))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(BAN)) goto eoh;
			size_t bid=forbiden_id;
			for(size_t i=0; i<players.size(); i++){
				if(players[i].is_ingame){
					if(!memcmp(players[i].name.c_str(), p->data+5, p->length-5)){
						if((!players[i].has_admin) && (!players[i].moderator)) bid=i;
						break;
					}
				}
			}
			if(bid==forbiden_id){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
				goto eoh;
			}
			SendDisconnectionInfo(players[bid].id.g);
			sprintf((char*)szLog.data(), "%s@%s ban %s@%s", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false), players[bid].name.c_str(), server->GetSystemAddressFromGuid(players[bid].id).ToString());
			log->Write(LOG_WARNING, szLog.data());
			server->AddToBanList(server->GetSystemAddressFromGuid(players[bid].id).ToString(false));
			ban_list.push_back(server->GetSystemAddressFromGuid(players[bid].id).ToString(false));
			DeleteFromPlayerList(players[bid].id);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p->systemAddress, false);
		} else if(!memcmp(p->data+1, BAN_IP,strlen(BAN_IP))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(BAN_IP)) goto eoh;
			if(server->IsBanned((char*)p->data+7)){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, IPISBANNED, strlen(IPISBANNED)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(IPISBANNED)), MEDIUM_PRIORITY, RELIABLE, 9, p->systemAddress, false);
				goto eoh;
			}
			std::stack<sPlayer*> ip_players;
			for(size_t i=0; i<players.size(); i++){
				if(!memcmp(server->GetSystemAddressFromGuid(players[i].id).ToString(false), p->data+7, p->length-7)){
					ip_players.push(&players[i]);
				}
			}
			if(!ip_players.empty()){
				do{
					SendDisconnectionInfo(ip_players.top()->id.g);
					sprintf((char*)szLog.data(), "%s@%s banned because of IP by %s@%s", ip_players.top()->name.c_str(), (char*)p->data+7, (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false));
					log->Write(LOG_WARNING, szLog.data());
					DeleteFromPlayerList(ip_players.top()->id);
					ip_players.pop();
				} while(!ip_players.empty());
			}
			server->AddToBanList((char*)p->data+7);
			ban_list.push_back((char*)p->data+7);
			sprintf((char*)szLog.data(), "%s@%s ban ip: %s", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false), (char*)p->data+7);
			log->Write(LOG_WARNING, szLog.data());
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p->systemAddress, false);
		} else if(!memcmp(p->data+1, UNBAN,strlen(UNBAN))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(UNBAN)) goto eoh;
			if(server->IsBanned((char*)p->data+7)){
				sprintf((char*)szLog.data(), "%s@%s unbanned following IP: %s", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false), (char*)p->data+7);
				log->Write(LOG_WARNING, szLog.data());
				server->RemoveFromBanList((char*)p->data+7);
			} else{
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, IPNOTBANNED, strlen(IPNOTBANNED)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(IPNOTBANNED)), MEDIUM_PRIORITY, RELIABLE, 9, p->systemAddress, false);
			}
		} else if(!memcmp(p->data+1, SAVEBANS,strlen(SAVEBANS)-1)){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(SAVEBANS)) goto eoh;
			SaveBanList();
			sprintf((char*)szLog.data(), "%s@%s saved ban list!", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false));
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
		} else if(!memcmp(p->data+1, MUTE,strlen(MUTE))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(MUTE)) goto eoh;
			size_t mid=forbiden_id;
			for(size_t i=0; i<players.size(); i++){
				if(players[i].is_ingame){
					if(!memcmp(players[i].name.c_str(), p->data+6, p->length-6)){
						mid=i;
						break;
					}
				}
			}
			if(mid==forbiden_id){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
			} else{
				players[mid].mute= ~players[mid].mute;
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
			}
		} else if(!memcmp(p->data+1, SET_TIME,strlen(SET_TIME))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(SET_TIME)) goto eoh;
			char buffer[16];
			unsigned short h,m;
			if(sscanf((char*)p->data+1, "%s%hu%hu", buffer, &h, &m)==3){
				game_time.hour=static_cast<unsigned char>(h);
				game_time.min=static_cast<unsigned char>(m);
				game_time.day++;
				if(game_time.day>30000) game_time.day=1;
				for(size_t i=0; i<players.size(); i++){
					if(players[i].is_ingame){
						*((unsigned char*)szLog.data())=PT_GAME_INFO;
						memcpy((char*)szLog.data()+1, &this->game_time.time, 4);
						*((unsigned char*)szLog.data()+5)=(unsigned char)game_mode;
						*((unsigned char*)szLog.data()+6)=0;
						if(quick_pots) *((unsigned char*)szLog.data()+6)|=QUICK_POTS;
						if(allow_dropitems) *((unsigned char*)szLog.data()+6)|=DROP_ITEMS;
						server->Send((char*)szLog.data(), 7, IMMEDIATE_PRIORITY, RELIABLE, 9, players[i].id, false);
					}
				}
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
			} else{
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, INVALIDPARAMETER, strlen(INVALIDPARAMETER)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(INVALIDPARAMETER)), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
			}
		} else if(!memcmp(p->data+1, KILL, strlen(KILL))){
			if(!players[rid].has_admin) if(!CPermissions::GetInstance()->IsPermitted(KILL)) goto eoh;
			sPlayer *n00b=FindPlayer((const char*)p->data+1+strlen(KILL));
			if(n00b){
				n00b->health=0;
				n00b->tod=time(NULL);
				int r=rand()%MAX_KILL_TXT;
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, KILLED[r], strlen(KILLED[r])+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(KILLED[r])+1), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
				SendDeathInfo(n00b->id.g);
				sprintf((char*)szLog.data(), "%s@%s killed %s@%s", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false), n00b->name.c_str(), server->GetSystemAddressFromGuid(n00b->id).ToString(false));
				log->Write(LOG_WARNING, szLog.data());
			} else{
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
			}
		} else if(!memcmp(p->data+1, LOOP_MSG, strlen(LOOP_MSG))){
			if(!players[rid].has_admin) if(!CPermissions::GetInstance()->IsPermitted(LOOP_MSG)) goto eoh;
			if(!loop_msg.empty()) loop_msg.clear();
			loop_msg=((char*)p->data+strlen(LOOP_MSG)+1);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
			sprintf((char*)szLog.data(), "%s@%s changed loop message to: %s", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false),loop_msg.c_str());
			log->Write(LOG_WARNING, szLog.data());
		} else if(!memcmp(p->data+1, MOD_ADD, strlen(MOD_ADD))){
			if(!players[rid].has_admin) goto eoh;
			char b[16];
			char n[128];
			if(sscanf((const char*)p->data+1, "%s%s", b, n)!=2){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, INVALIDPARAMETER, strlen(INVALIDPARAMETER)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(INVALIDPARAMETER)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
				goto eoh;
			}
			CPermissions::GetInstance()->AddModerator(n, "\x7");
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
			sprintf((char*)szLog.data(), "ADMIN@%s added moderator: %s", p->systemAddress.ToString(false), n);
			log->Write(LOG_WARNING, szLog.data());
		} else if(!memcmp(p->data+1, MOD_DEL, strlen(MOD_DEL))){
			if(!players[rid].has_admin) goto eoh;
			char b[16];
			char n[128];
			if(sscanf((const char*)p->data+1, "%s%s", b, n)!=2){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, INVALIDPARAMETER, strlen(INVALIDPARAMETER)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(INVALIDPARAMETER)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
				goto eoh;
			}
			CPermissions::GetInstance()->DeleteModerator(n);
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
			sprintf((char*)szLog.data(), "ADMIN@%s deleted moderator: %s", p->systemAddress.ToString(false), n);
			log->Write(LOG_WARNING, szLog.data());
		} else if(!memcmp(p->data+1, MOD_SET, strlen(MOD_SET))){
			if(!players[rid].has_admin) goto eoh;
			char b[16];
			char n[128];
			char pass[128];
			if(sscanf((const char*)p->data+1, "%s%s%s", b,n,pass)!=3){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, INVALIDPARAMETER, strlen(INVALIDPARAMETER)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(INVALIDPARAMETER)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
				goto eoh;
			}
			if(CPermissions::GetInstance()->SetPassword(n,pass)){
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
				sprintf((char*)szLog.data(), "ADMIN@%s changed moderator %s password to %s", p->systemAddress.ToString(false), n, pass);
				log->Write(LOG_WARNING, szLog.data());
			} else{
				*((unsigned char*)szLog.data())=PT_RCON;
				memcpy((char*)szLog.data()+1, PDE, strlen(PDE)+1);
				server->Send(szLog.data(), static_cast<int>(2+strlen(PDE)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
				goto eoh;
			}
		} else if(!memcmp(p->data+1, HP_REGEN, strlen(HP_REGEN))){
			if(!players[rid].has_admin)	if(!CPermissions::GetInstance()->IsPermitted(HP_REGEN)) goto eoh;
			if(sscanf((const char*)p->data+1+strlen(HP_REGEN), "%hd", &hp_regeneration)!=1){
				goto eoh;
			}
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
			sprintf((char*)szLog.data(), "%s@%s changed hp regeneration: %hd", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false),hp_regeneration);
			log->Write(LOG_WARNING, szLog.data());
			goto eoh;
			//sprintf((char*)szLog.data()+1, "AD
		}else if(!memcmp(p->data+1, MP_REGEN, strlen(MP_REGEN))){
			if(!players[rid].has_admin) if(!CPermissions::GetInstance()->IsPermitted(MP_REGEN)) goto eoh;
			if(sscanf((const char*)p->data+1+strlen(MP_REGEN), "%hd", &mp_regeneration)!=1) goto eoh;
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, OK, strlen(OK)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(OK)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
			sprintf((char*)szLog.data(), "%s@%s changed mp regeneration: %hd", (const char*)((players[rid].has_admin)?"ADMIN":players[rid].moderator->nickname), p->systemAddress.ToString(false),mp_regeneration);
			log->Write(LOG_WARNING, szLog.data());
			for(size_t i=0; i<players.size(); i++) if(players[i].is_ingame) SendGameInfo(players[i].id);
			goto eoh;
		}else{
			//cool story, bro!
			*((unsigned char*)szLog.data())=PT_RCON;
			memcpy((char*)szLog.data()+1, WTF, strlen(WTF)+1);
			server->Send(szLog.data(), static_cast<int>(2+strlen(WTF)), MEDIUM_PRIORITY, RELIABLE, 13, p->guid, false);
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

RAK_THREAD_DECLARATION(CGmpServ::AddToPublicListHTTP){
	char *szBuff=new char[256];
	memset(szBuff, 0, 256);
	while(pSrv){
        if(pSrv->IsPublic()){
		for(size_t i=0; i<=strlen(pSrv->name.c_str()); i++) if((*((unsigned char*)pSrv->name.c_str()+i)<0x20) && (*((unsigned char*)pSrv->name.c_str()+i)!=0x07)) *((unsigned char*)pSrv->name.data()+i)=0;
		memset(szBuff, 0, 256);
		sprintf(szBuff, "%s?sn=%s&port=%hu&crt=%u&mx=%d&map=%s", lobbyFile, pSrv->name.c_str(), pSrv->game_port, static_cast<unsigned int>(pSrv->players.size()), pSrv->slots, pSrv->map.c_str());
        MakeHTTPReq(szBuff);
        }
		RakSleep(5000);
	}
	delete [] szBuff;
	return 0;
}

void CGmpServ::HandleGameInfo(RakNet::Packet *p){
	SendGameInfo(p->guid);
}

//void CGmpServ::HandleGameInfo(RakNet::Packet *p){
void CGmpServ::SendGameInfo(RakNet::RakNetGUID who){
	std::string szData;
	int len=7;
	szData.reserve(64);
	*((unsigned char*)szData.data())=PT_GAME_INFO;
	memcpy((char*)szData.data()+1, &this->game_time.time, 4);
	*((unsigned char*)szData.data()+5)=(unsigned char)game_mode;
	*((unsigned char*)szData.data()+6)=0;
	if(quick_pots) *((unsigned char*)szData.data()+6)|=QUICK_POTS;
	if(allow_dropitems) *((unsigned char*)szData.data()+6)|=DROP_ITEMS;
	if(hide_map) *((unsigned char*)szData.data()+6)|=HIDE_MAP;
	if(mp_regeneration){ *((unsigned char*)szData.data()+6)|=MANA_REGENERATION; memcpy((char*)szData.data()+len, &mp_regeneration, 2); len+=2;}
	server->Send((char*)szData.data(), len, MEDIUM_PRIORITY, RELIABLE, 9, who, false);
	szData.clear();
}

void CGmpServ::HandleMapNameReq(RakNet::Packet *p){
	std::string szData;
	szData.reserve(256);
	*((unsigned char*)szData.data())=PT_MAP_NAME;
	memcpy((char*)szData.data()+1, this->map.c_str(), strlen(this->map.data())+1);
	server->Send((char*)szData.data(), static_cast<int>(2+strlen(this->map.data())), MEDIUM_PRIORITY, RELIABLE, 9, p->guid, false);
	szData.clear();
}
void CGmpServ::SendDisconnectionInfo(uint64_t disconnected_id){
	std::string szData;
	szData.reserve(32);
	*((unsigned char*)szData.data())=PT_LEFT_GAME;
	memcpy((char*)szData.data()+1, &disconnected_id, sizeof(uint64_t));
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame)
			if(players[i].id.g!=disconnected_id){
				server->Send(szData.data(), 1+sizeof(uint64_t), IMMEDIATE_PRIORITY, RELIABLE, 8, players[i].id, false);
			}
	}
	szData.clear();
}
bool CGmpServ::IsPublic(){ return (is_public)?true:false;}

void CGmpServ::DoRespawns(){
	if(respawn_time){
		if(respawn_time&0x80000000){
			//last stand
			switch(game_mode){
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
									SendRespawnInfo(players[i].id.g);
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
									SendRespawnInfo(players[i].id.g);
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
						SendRespawnInfo(players[i].id.g);
					}
				}
			}
		}
	}
}

void CGmpServ::SendSpamMessage(){
	if(loop_time>0){
		if(spam_time<time(NULL)){
			spam_time=time(NULL)+loop_time;
			if(players.size()){
				std::string szData;
				szData.reserve(512);
				*((unsigned char*)szData.data())=PT_SRVMSG;
				memcpy((char*)szData.data()+1, loop_msg.c_str(), loop_msg.length()+1);
				for(size_t i=0; i<players.size(); i++){
					if(players[i].is_ingame) server->Send(szData.data(), static_cast<int>(loop_msg.length()+2), MEDIUM_PRIORITY, UNRELIABLE, 11, players[i].id, false);
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
		if(players[i].is_ingame) server->Send((char*)buffer, 9, IMMEDIATE_PRIORITY, RELIABLE, 13, players[i].id, false);
	}
}

void CGmpServ::SendRespawnInfo(uint64_t luckyguy){
	unsigned char buffer[9];
	buffer[0]=PT_RESPAWN;
	memcpy(buffer+1, &luckyguy, 8);
	for(size_t i=0; i<players.size(); i++){
		if(players[i].is_ingame) server->Send((char*)buffer, 9, IMMEDIATE_PRIORITY, RELIABLE, 13, players[i].id, false);
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
	if(!(hp_regeneration)) return;
	if(regen_time==time(NULL)) return;
	regen_time=time(NULL);
	for(size_t i=0; i<this->players.size(); i++){
		if((players[i].is_ingame) && (players[i].health>0)){
			players[i].health+=hp_regeneration;
			if(players[i].health>classmgr->class_array[players[i].char_class].abilities[HP]) players[i].health=classmgr->class_array[players[i].char_class].abilities[HP];
			if(players[i].health<=0){
				players[i].tod=time(NULL);
				SendDeathInfo(players[i].id.g);
			}
		}
	}
}