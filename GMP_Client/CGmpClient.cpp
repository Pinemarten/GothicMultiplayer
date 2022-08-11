
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

#include "CGmpClient.h"
#include "CConfig.h"
#include "CIngame.h"
#include "patch.h"
#include "g2Api.h"
#include "Interface.h"
#include "CSelectClass.h"
#include "CChat.h"
#include "../GMP_Serv/CCompression.h"
#include <vector>
#include <list>
#include <string>
#include <ctime>
#include <cstdlib>
#include "CLocalPlayer.h"

CConfig *user_config=NULL;
zSTRING CYL="Choose your language:";
zSTRING STUMBLE = "T_STUMBLE";
zSTRING GOTHIT = "T_GOTHIT";
zSTRING HitSound = "SVM_15_AARGH_4";
zSTRING Arrows = "ITRW_ARROW";
zSTRING Bolt = "ITRW_BOLT";
zSTRING BowSound = "BOWSHOOT";
zSTRING CrossbowSound = "CROSSBOWSHOOT";
zSTRING Door =  "DOOR";
zSTRING Lever = "LEVER";
zSTRING Touchplate = "TOUCHPLATE";
zSTRING VWheel = "VWHEEL";
const char *LANG_DIR=".\\Multiplayer\\lang\\";
//lista u¿ywalnych itemów(chyba nic nie pomin¹³em)
const short GOTHIC_DAT_ITEMS[866]={5892,5895,5902,5909,5916,5923,5930,5937,5944,5951,5967,5968,5969,5970,5971,5972,5973,5974,5975,5976,5977,5978,5980,5981,5982,5983,5984,5985,5986,5987,5988,5989,5990,5991,5992,5993,5994,5995,5996,5997,5998,5999,6001,6002,6003,6004,6005,6006,6007,6008,6009,6010,6011,6012,6013,6014,6015,6016,6017,6018,6019,6020,6021,6022,6053,6056,6059,6062,6065,6068,6071,6074,6077,6080,6083,6086,6089,6092,6095,6098,6115,6117,6119,6122,6124,6126,6129,6130,6132,6135,6136,6137,6138,6139,6140,6141,6142,6143,6147,6148,6149,6152,6153,6154,6155,6157,6158,6160,6161,6163,6165,6166,6168,6169,6171,6173,6175,6177,6179,6181,6183,6199,6200,6201,6202,6203,6204,6205,6206,6207,6208,6209,6210,6217,6218,6219,6220,6221,6239,6240,6242,6244,6246,6247,6249,6251,6253,6260,6262,6306,6308,6310,6312,6314,6316,6318,6320,6322,6324,6326,6328,6330,6332,6334,6336,6338,6340,6342,6344,6346,6348,6350,6352,6355,6356,6357,6360,6361,6362,6363,6364,6365,6366,6367,6368,6369,6370,6371,6372,6373,6374,6375,6376,6377,6378,6379,6380,6381,6382,6383,6386,6389,6392,6395,6398,6401,6404,6407,6410,6413,6417,6420,6422,6425,6426,6427,6428,6429,6430,6431,6432,6433,6434,6435,6436,6464,6467,6470,6473,6476,6479,6482,6485,6488,6491,6494,6534,6535,6537,6538,6539,6540,6541,6542,6543,6545,6546,6548,6549,6550,6551,6552,6553,6554,6555,6556,6557,6558,6559,6560,6561,6562,6563,6564,6565,6566,6567,6568,6569,6570,6571,6572,6573,6613,6614,6615,6616,6617,6618,6619,6620,6621,6622,6623,6626,6629,6630,6631,6632,6633,6634,6635,6638,6641,6644,6647,6648,6649,6650,6651,6654,6657,6660,6663,6664,6665,6666,6667,6668,6669,6670,6671,6709,6711,6713,6715,6717,6719,6721,6723,6725,6727,6730,6732,6734,6736,6738,6740,6742,6744,6750,6751,6752,6753,6754,6755,6756,6757,6758,6759,6760,6761,6762,6763,6764,6765,6766,6767,6768,6769,6770,6771,6772,6773,6774,6775,6776,6777,6778,6779,6780,6781,6782,6783,6784,6785,6786,6787,6788,6789,6790,6791,6792,6793,6794,6795,6796,6797,6798,6799,6800,6801,6802,6803,6804,6805,6806,6807,6808,6809,6810,6811,6812,6813,6814,6815,6816,6817,6818,6819,6820,6821,6822,6823,6824,6825,6826,6827,6828,6829,6830,6831,6832,6833,6834,6835,6836,6837,6838,6839,6840,6841,6842,6843,6844,6845,6846,6847,6848,6849,6850,6851,6852,6853,6903,6904,6905,6906,6907,6908,6909,6910,6911,6912,6913,6914,6915,6916,6918,6920,6922,6924,6926,6928,6929,6930,6931,6932,6933,6934,6935,6936,6937,6938,6939,6940,6941,6942,6943,6944,6945,6946,6947,6948,6949,6950,6951,6952,6953,6954,6955,6956,6957,6958,6959,6960,6996,6997,6999,7001,7003,7005,7007,7009,7011,7013,7015,7017,7019,7021,7023,7025,7027,7029,7031,7059,7061,7063,7065,7067,7069,7071,7073,7075,7077,7079,7081,7083,7084,7085,7086,7087,7088,7089,7090,7091,7092,7093,7094,7095,7096,7097,7098,7099,7100,7101,7102,7103,7104,7153,7156,7159,7162,7165,7168,7171,7174,7177,7180,7183,7186,7189,7192,7195,7198,7201,7204,7207,7210,7262,7263,7264,7265,7266,7267,7268,7269,7270,7271,7272,7273,7274,7275,7276,7277,7278,7279,7280,7281,7282,7283,7284,7285,7286,7287,7288,7289,7290,7291,7292,7293,7294,7295,7296,7297,7298,7299,7300,7301,7302,7303,7304,7305,7306,7307,7308,7309,7310,7311,7312,7363,7364,7365,7366,7367,7368,7369,7370,7371,7372,7373,7374,7375,7376,7377,7378,7379,7380,7381,7382,7383,7384,7385,7386,7387,7388,7389,7390,7391,7392,7393,7394,7395,7396,7397,7398,7399,7400,7401,7402,7403,7404,7405,7406,7407,7408,7409,7410,7411,7412,7414,7416,7418,7420,7422,7424,7426,7429,7430,7431,7432,7439,7442,7445,7450,7453,7456,7457,7460,7461,7462,7464,7465,7468,7469,7472,7475,7478,7479,7481,7483,7485,7488,7491,7494,7497,7498,7499,7500,7501,7502,7503,7504,7505,7507,7510,7511,7514,7517,7520,7521,7522,7525,7526,7527,7528,7530,7531,7532,7533,7534,7535,7536,7537,7538,7541,7542,7543,7544,7545,7548,7551,7552,7553,7554,7555,7557,7560,7561,7563,7564,7565,7566,7567,7570,7573,7576,7579,7582,7585,7592,7595,7597,7599,7602,7604,7605,7608,7611,7614,7616,7617,7619,7620,7623,7626,7628,7629,7634,7636,7638,7641,7644,7646,7647,7648,7651,7652,7653,7656,7659,7662,7667,7670,7671,7673,7674,7675,7676,7677,7681,7683,7687,7690,7693,7694,7696,7699,7702,7705,7708,7711,7712,7713,7715,7716,7719,7721,7724,7727,7730,7731,7732,7733,7734,7737,7738,7741,7744,7747,7748,7753,7754,7755,7756,7759,7765,7766,7767,7768,7769,7772,7776,7779,7780,7784,7785,7786,7787,7788,7790,7791,7795,7799,7800,7806,7807,7810,7811,7812,7815,7818,7821,7824,7827,7830,7831,7833,7836,7839,7840,7841,7844,7845,7848,7850};
std::vector<zSTRING> vec_choose_lang;
std::vector<std::string> vec_lang_files;
const BYTE GDI_MD5[16]={0x34,0x16,0x98,0x62,0x43,0xB0,0x3D,0x36,0xD6,0xB0,0x1A,0x4D,0x2B,0xCB,0xDB,0x8B};

float fWRatio, fHRatio;
extern CConfig* user_config;
extern CIngame* global_ingame;
extern CLocalPlayer* LocalPlayer;
extern zCOLOR AQUA, RED;

CGmpClient::CGmpClient(const char *ip, CLanguage *ptr)
{
	srand(static_cast<unsigned int>(time(NULL)));
	this->client = RakNet::RakPeerInterface::GetInstance();
	this->work_layer^=this->work_layer;
	this->lang=ptr;
	this->classmgr=NULL;
	this->spawnpoint=NULL;
	this->IsAdminOrModerator = false;
	this->IgnoreFirstTimeMessage = true;
	this->DropItemsAllowed = false;
	this->connected_with_srv=FALSE;
	this->ObserveMode = CObservation::NO_OBSERVATION;
	tmp8[0]=0;
	tmp8[1]=0;
	tmp8[2]=0;
	this->clientPort=0xDEAD;
	this->clientHost=ip;
	size_t pos=this->clientHost.find_last_of(':');
	if(pos!=std::string::npos){
		sscanf(this->clientHost.c_str()+pos+1, "%d", &this->clientPort);
		this->clientHost.erase(pos);
	}
}

CGmpClient::~CGmpClient(void)
{
	if(this->classmgr){
		delete this->classmgr;
		this->classmgr=NULL;
	}
	if(this->spawnpoint){
		delete this->spawnpoint;
		this->spawnpoint=NULL;
	}
	this->clientHost.clear();
	//this->client->Shutdown(300);
	//RakNet::RakPeerInterface::DestroyInstance(this->client);
}

bool CGmpClient::Connect(){ return Connect(clientHost.c_str(), "yourpass", clientPort); }

bool CGmpClient::Connect(std::string hostAddress, std::string hostPassword, int hostPort)
{
	disconnected=FALSE;
	interrupted=FALSE;
	game_mode=0;
	mp_restore=0;
	client->SetTimeoutTime(1500, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	RakNet::SocketDescriptor socketDescriptor(0,0);
	socketDescriptor.socketFamily=AF_INET;
	client->Startup(1,&socketDescriptor, 1);
	client->SetOccasionalPing(true);

	RakNet::ConnectionAttemptResult car = client->Connect(hostAddress.c_str(), hostPort, hostPassword.c_str(), hostPassword.length());
	RakNet::Packet *p=NULL;
	do{
		p=client->Receive();
	} while(!p);
	error=(p->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)?0:p->data[0];
	if(p->data[0]==ID_CONNECTION_REQUEST_ACCEPTED){
		connected_with_srv=TRUE;
		srvAddr=p->systemAddress;
	}
	client->DeallocatePacket(p);
	return (error==0);//(car==RakNet::CONNECTION_ATTEMPT_STARTED);
}
/* [D]jak bêdzie potrzbne to siê odkomentuje
bool CGmpClient::Send(std::string message)
{
	client->Send(message.c_str(), message.length()+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	return true;
}*/
/* [D] to samo tutaj
bool CGmpClient::Receive(sPacket & packet)
{
	RakNet::Packet *p = client->Receive();
	if(p==0)return false;
	unsigned char packetIdentifier = GetPacketIdentifier(p);
	switch (packetIdentifier)
	{
	case ID_DISCONNECTION_NOTIFICATION:
		break;
	case ID_NEW_INCOMING_CONNECTION:
		printf("ID_NEW_INCOMING_CONNECTION from %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
		break;
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
		break;
	case ID_CONNECTION_LOST:
		printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString(true));
		break;
	default:
		packet.type = PT_MSG;
		packet.data.clear();
		packet.data.append((char *)p->data);
		break;
	}
	client->DeallocatePacket(p);
	return true;
}*/
// [D] to siê mo¿e w przysz³oœci odkomentuje
int CGmpClient::GetPing()
{
	return client->GetAveragePing(client->GetSystemAddressFromIndex(0));
}

void CGmpClient::HandleNetwork(){
	if((this->connected_with_srv) && (!this->error)){
		switch(this->work_layer){
			case WL_PREPARE_TO_JOIN:
				//0a - miejsce na crc32
				{	//0b i 0c - klasy i miejsca odradzania
					RakNet::Packet *p;
					BYTE buffer[1024];
					memset(buffer, 0, 1024);
					buffer[0]=PT_WHOAMI;
					client->Send((const char*)buffer, 1, IMMEDIATE_PRIORITY, RELIABLE, 0, srvAddr, false);
					do{
						p=client->Receive();
					} while(!p);
					if(p->data[0]==PT_WHOAMI){
						memcpy(&this->playerID, p->data+1, sizeof(uint64_t));
					} else{	//coœ siê popsu³o
						this->error=p->data[0];
						printf("We've got trouble: %d!\n", (int)this->error);
						client->DeallocatePacket(p);
						return;
					}
					client->DeallocatePacket(p);

					//
					//WB
					//WB - len
					BYTE wb_req[8];
					wb_req[0]=PT_REQUEST_FILE_LENGTH;
					wb_req[1]=WB_FILE;
					client->Send((const char*)wb_req, 2, IMMEDIATE_PRIORITY, RELIABLE, 0, srvAddr, false);
					do{ p=client->Receive(); Sleep(1);} while(!p);
					unsigned short wb_parts, wb_complete=0, last_part=0;
					unsigned int wb_length=0;
					if((p->data[0]==PT_REQUEST_FILE_LENGTH) && (p->data[1]!=NULL_SIZE)){
						memcpy(&wb_parts, p->data+2, 2);
						client->DeallocatePacket(p);
					} else{
						if(p->data[0]!=PT_REQUEST_FILE_LENGTH){
							this->error=p->data[0];
							printf("Could not receive file length: %d!\n", (int)this->error);
						}
						else{
							printf("No wb file!\n");
							string filename=".\\Multiplayer\\Data\\";
							filename+=srvAddr.ToString(false);
							filename+="_";
							char lol[8];
							sprintf(lol, "%hu", srvAddr.GetPort());
							filename+=lol;
							lol[1]=0;
							FILE *check=fopen(filename.c_str(), "wb");
							if(check){
								fclose(check);
								remove(filename.c_str());
							}
							filename.clear();
						}
						client->DeallocatePacket(p);
						goto skip_wb;
					}
					//pobieranie...
					BYTE *wb_file=new BYTE[wb_parts*1000];
					BYTE ongoing_req=0;
					do{
						if((ongoing_req<5) && (last_part<wb_parts)){	//make new req
							wb_req[0]=PT_REQUEST_FILE_PART;
							wb_req[1]=WB_FILE;
							memcpy(wb_req+2, &last_part, 2);
							client->Send((LPSTR)wb_req, 4, IMMEDIATE_PRIORITY, RELIABLE, 0, srvAddr, false);
							last_part++;
							ongoing_req++;
						}
						p=client->Receive();
						if(p){
							if(p->data[0]==PT_REQUEST_FILE_PART){
								unsigned short part;
								unsigned int out_len=0;
								//BYTE usable_data[1024];
								memcpy(&part, p->data+2, 2);
								CCompression::GetInstance()->Decompress(p->data+4, wb_file+(part*1000), p->length-4, out_len);
								//memcpy(wb_file+(part*1000), p->data+4, p->length-4);
								ongoing_req--;
								wb_complete++;
								wb_length+=out_len;
							} else{
								this->error=p->data[0];
								printf("Error while downloading wb data: %d!\n", (int)this->error);
								client->DeallocatePacket(p);
								return;
							}
							client->DeallocatePacket(p);
						}
					} while(wb_complete!=wb_parts);
					//zapisywanie...
					{
						string filename=".\\Multiplayer\\Data\\";
						filename+=srvAddr.ToString(false);
						filename+="_";
						char lol[8];
						sprintf(lol, "%hu", srvAddr.GetPort());
						filename+=lol;
						lol[1]=0;
						FILE *pfile=fopen(filename.c_str(), "wb");
						fwrite(wb_file, wb_length, 1, pfile);
						fclose(pfile);
						filename.clear();
					//end WB
					}
skip_wb:

					buffer[0]=PT_REQUEST_FILE_LENGTH;
					buffer[1]=CLASS_FILE;
					client->Send((const char*)buffer, 2, IMMEDIATE_PRIORITY, RELIABLE, 0, srvAddr, false);
					BYTE parts=0;
					do{ p=client->Receive(); } while(!p);
					if(p->data[0]==PT_REQUEST_FILE_LENGTH){
						parts=p->data[2];
					} else{
						this->error=p->data[0];
						printf("Could not receive file length: %d!\n", (int)this->error);
						client->DeallocatePacket(p);
						return;
					}
					client->DeallocatePacket(p);

					char *file_buffer=new char[parts*1000];
					memset(file_buffer,0,parts*1000);
					for(short i=0; i<parts; i++){
						buffer[0]=PT_REQUEST_FILE_PART;
						buffer[1]=CLASS_FILE;
						buffer[2]=(BYTE)i;
						client->Send((LPSTR)buffer, 3, IMMEDIATE_PRIORITY, RELIABLE, 0, srvAddr, false);
					}
					BYTE downloaded=0;
					//used deflate compression
					do{
						do{	p=client->Receive(); } while(!p);
						if((p->data[0]==PT_REQUEST_FILE_PART) && (p->data[1]==CLASS_FILE)){
							BYTE usable_data[1024];
							UINT32 out_len=0;
							CCompression::GetInstance()->Decompress(p->data+3, usable_data, p->length-3, out_len);
							memcpy(file_buffer+(1000*(int)p->data[2]), usable_data, out_len);
							client->DeallocatePacket(p);
							downloaded++;
						} else{
							this->error=p->data[0];
							printf("We've got a problem(GMPFT): %d!\n", this->error);
							client->DeallocatePacket(p);
							delete file_buffer;
							return;
						}
					} while(downloaded<parts);

					this->classmgr=new CHeroClass(file_buffer, parts);
					//printf("Number of available classes: %lu\n", this->classmgr->GetSize());
					delete file_buffer;

					buffer[0]=PT_REQUEST_FILE_LENGTH;
					buffer[1]=SPAWN_FILE;
					client->Send((LPSTR)buffer, 2, IMMEDIATE_PRIORITY, RELIABLE, 0, srvAddr, false);
					do{ p=client->Receive(); } while(!p);
					if(p->data[0]==PT_REQUEST_FILE_LENGTH){
						parts=p->data[2];
					} else{
						this->error=p->data[0];
						printf("Could not receive file length: %d!\n", (int)this->error);
						client->DeallocatePacket(p);
						return;
					}
					client->DeallocatePacket(p);

					file_buffer=new char[parts*1000];
					memset(file_buffer, 0, parts*1000);
					for(short i=0; i<parts; i++){
						buffer[0]=PT_REQUEST_FILE_PART;
						buffer[1]=SPAWN_FILE;
						buffer[2]=(BYTE)i;
						client->Send((LPSTR)buffer, 3, IMMEDIATE_PRIORITY, RELIABLE, 0, srvAddr, false);
					}
					downloaded=0;
					do{
						do { p=client->Receive(); } while(!p);
						if((p->data[0]==PT_REQUEST_FILE_PART) && (p->data[1]==SPAWN_FILE)){
							memcpy(file_buffer+(1000*(int)p->data[2]), p->data+3, p->length-3);
							client->DeallocatePacket(p);
							downloaded++;
						} else{
							this->error=p->data[0];
							printf("We've got a problem(GMPFT): %d!\n", this->error);
							client->DeallocatePacket(p);
							delete file_buffer;
							return;
						}
					}while(downloaded<parts);

					spawnpoint=new CSpawnPoint(file_buffer);
					printf("Number of spawnpoints: %lu\n", spawnpoint->GetSize());
					delete file_buffer;
					file_buffer=NULL;

					size_t random_spawnpoint=rand()%spawnpoint->GetSize();
					oCNpc::GetHero()->SetPosition((*spawnpoint)[random_spawnpoint]->x, (*spawnpoint)[random_spawnpoint]->y, (*spawnpoint)[random_spawnpoint]->z);
					printf("All configuration downloaded!\n");
					buffer[0]=PT_MAP_NAME;
					client->Send((char*)buffer, 1, IMMEDIATE_PRIORITY, RELIABLE, 9, this->srvAddr, false);
					do{ p=client->Receive();} while(!p);
					if(p->data[0]!=PT_MAP_NAME){
						this->error=p->data[0];
						printf("Can not receive map name: %d!\n", this->error);
						client->DeallocatePacket(p);
						delete file_buffer;
						return;
					}
					this->work_layer++;
					const char* current_map=oCGame::GetGame()->GetWorld()->GetWorldName().ToChar();
					//printf("Current map: %s\nServer map: %s\n", current_map, (char*)p->data+1);
					if(memcmp(p->data+1, current_map, (p->length-1>strlen(current_map)+1)?strlen(current_map)+1:p->length-1)){
						map=((char*)p->data+1);
					} else {if(!map.IsEmpty()) map.Clear();}
					zCOption::GetOption()->ChangeDir(WORK_DATA_WORLDS);
					zFILE_VDFS *vdf=oCObjectFactory::GetFactory()->CreateZFile((char*)p->data+1);
					if(vdf->Exists()){
						vdf->Open(false);
						BYTE *map_buffer=new BYTE[vdf->Size()];
						vdf->Read(map_buffer, vdf->Size());
						/*FILE *mapfile=fopen("G:\\map.zen", "wb");
						fwrite(map_buffer, vdf->Size(), 1, mapfile);
						fclose(mapfile);*/
						MD5Sum *md5=GetMD5(map_buffer, vdf->Size());
						vdf->Close();
						delete map_buffer;
						vdf=NULL;
						if(md5){
							/*FILE *fh=fopen("G:\\gmp-md5.bin", "wb");
							fwrite(md5, 16, 1, fh);
							fclose(fh);*/
							BYTE packet_buffer[18];
							memset(packet_buffer, 0, 18);
							packet_buffer[0]=PT_CHECKSUM;
							packet_buffer[1]=AreDefaultItems();
							memcpy(packet_buffer+2, md5, 16);
							client->Send((LPCSTR)packet_buffer, 18, IMMEDIATE_PRIORITY, RELIABLE, 1, srvAddr, false);
							memset(packet_buffer, 0, 18);
							memset(md5, 0, 16);
							delete md5;
						} else printf("lol\n");
					} else{
						printf("Are you sure that you have this map?\n");
						BYTE packet_buffer[18];
						memset(packet_buffer, 0, 18);
						packet_buffer[0]=PT_CHECKSUM;
						packet_buffer[1]=AreDefaultItems();
						client->Send((LPCSTR)packet_buffer, 18, IMMEDIATE_PRIORITY, RELIABLE, 1, srvAddr, false);
					}

					//dodaj tutaj funkcje ladujaca swiat z WB, plik znajduje sie w Multiplayer/Data/<IP>_<PORT>
				}
				break;
			case WL_JOIN_TO_GAME:
				break;
			case WL_INGAME:
				{
					if((mp_restore) && (!disconnected)){
						if(last_mp_regen!=time(NULL)){
							last_mp_regen=time(NULL);
							if(oCNpc::GetHero()->GetMaxMana()!=oCNpc::GetHero()->GetMana()){
								oCNpc::GetHero()->SetMana(oCNpc::GetHero()->GetMana()+mp_restore);
								if(oCNpc::GetHero()->GetMana()>oCNpc::GetHero()->GetMaxMana()) oCNpc::GetHero()->SetMana(oCNpc::GetHero()->GetMaxMana());
							}
						}
					}
					RakNet::Packet *p;
					do{
						p=this->client->Receive();
						if(p){
							switch(p->data[0]){
								case PT_ACTUAL_STATISTICS:
									{
										if(game_mode==1){
											for(size_t i=1; i<player.size(); i++){
												if(!memcmp((*classmgr)[player[i]->char_class]->team_name.ToChar(), (*classmgr)[player[0]->char_class]->team_name.ToChar(), (*classmgr)[player[0]->char_class]->team_name.Length())){
													player[i]->npc->variousFlags=1<<1;
													player[i]->SameTeamAsLocalPlayer = true;
												}
											}
										}
										bool found;
										unsigned int pIt=1;
										while(pIt<p->length){
											size_t i;
											found=false;
											for(i=0; i<this->player.size(); i++) if(!memcmp(p->data+pIt, &this->player[i]->id, sizeof(uint64_t))) { found=true; break; };
											if(found){
												if(i){
												pIt+=sizeof(uint64_t);
												pIt++;	//miejsce na przetwarzanie flag
												zVEC3 pos;
												memcpy(&pos, p->data+pIt, 12); pIt+=12;
												if(!this->player[i]->enable){
													this->player[i]->npc->Enable(pos);
													this->player[i]->enable=TRUE;
												}
												else this->player[i]->AnalyzePosition(pos);
												float nrot[3];
												memcpy(nrot, p->data+pIt, 12); pIt+=12;
												if(!this->player[i]->npc->IsDead()) this->player[i]->npc->SetAngle(-nrot[0], nrot[1], 0.0f, nrot[0], nrot[1], nrot[2]);
												short word;
												memcpy(&word, p->data+pIt, 2); pIt+=2;
												if(word==0){
													if(this->player[i]->npc->GetLeftHand()){
														oCItem *lptr=this->player[i]->npc->GetLeftHand();
														oCItem *ptr=this->player[i]->npc->GetRightHand();
														this->player[i]->npc->DropAllInHand();
														lptr->RemoveVobFromWorld();
														if(ptr) this->player[i]->npc->SetRightHand(ptr);
													}
												} else if (word > 5892 && word < 7850){
													if(!this->player[i]->npc->GetLeftHand()){
														oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
														this->player[i]->npc->SetLeftHand(New);
														CheckForSpecialEffects(this->player[i]->npc->GetLeftHand(), this->player[i]->npc);
													} else{
														if(this->player[i]->npc->GetLeftHand()->GetInstance()!=static_cast<int>(word)){
															oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
															oCItem* Old = this->player[i]->npc->GetLeftHand();
															this->player[i]->npc->SetLeftHand(New);
															Old->RemoveVobFromWorld();
															CheckForSpecialEffects(this->player[i]->npc->GetLeftHand(), this->player[i]->npc);
														}
													}
												}
												//sprawdzanie lewej rêki
												memcpy(&word, p->data+pIt, 2); pIt+=2;
												if(word==0){
													if(this->player[i]->npc->GetRightHand()){
														oCItem *rptr=this->player[i]->npc->GetRightHand();
														oCItem *ptr=this->player[i]->npc->GetLeftHand();
														this->player[i]->npc->DropAllInHand();
														if(ptr) this->player[i]->npc->SetLeftHand(ptr);
														PlayDrawSound(rptr, this->player[i]->npc, false);
														rptr->RemoveVobFromWorld();
													}
												} else if (word > 5892 && word < 7850){
													if(!this->player[i]->npc->GetRightHand()){
														oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
														this->player[i]->npc->SetRightHand(New);
														PlayDrawSound(this->player[i]->npc->GetRightHand(), this->player[i]->npc, true);
													} else{
														if(this->player[i]->npc->GetRightHand()->GetInstance()!=static_cast<int>(word)){
															oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
															oCItem* Old = this->player[i]->npc->GetRightHand();
															this->player[i]->npc->SetRightHand(New);
															Old->RemoveVobFromWorld();
															PlayDrawSound(this->player[i]->npc->GetRightHand(), this->player[i]->npc, true);
														}
													}
												}
												//sprawdzanie prawej rêki
												memcpy(&word, p->data+pIt, 2); pIt+=2;
												/*if(oCNpc::GetHero()->GetFocusNpc() == this->player[i]->npc){
													if(this->player[i]->npc->GetEquippedArmor()) printf("\nName : %s, Word  : %d, Armor : %d", this->player[i]->npc->GetName().ToChar(), word, this->player[i]->npc->GetEquippedArmor()->GetInstance());
													else printf("\nName : %s, Word  : %d", this->player[i]->npc->GetName().ToChar(), word);
												}*/
												if(word==0){
													if(this->player[i]->npc->GetEquippedArmor()){
															this->player[i]->npc->UnequipItem(this->player[i]->npc->GetEquippedArmor());
													}
												} else if (word > 5892 && word < 7850){
													if(!this->player[i]->npc->GetEquippedArmor()){
														if(!this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1)){
															oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
															this->player[i]->npc->GetInventory()->Insert(New);
															this->player[i]->npc->Equip(New);
														}
														else this->player[i]->npc->Equip(this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1));
													} else{
														if(this->player[i]->npc->GetEquippedArmor()->GetInstance()!=static_cast<int>(word)){
															if(!this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1)){
																oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
																this->player[i]->npc->GetInventory()->Insert(New);
																this->player[i]->npc->Equip(New);
															}
															else this->player[i]->npc->Equip(this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1));
														}
													}
												}
												//sprawdzanie zbroi
												memcpy(&word, p->data+pIt, 2); pIt+=2;
												if(this->player[i]->npc && word > 0 && word < 1400){
													zCModelAni* Ani = this->player[i]->npc->GetModel()->GetAniFromAniID(static_cast<int>(word));
													if(Ani && !this->player[i]->npc->IsDead()){
														if(!IsBadCodePtr((FARPROC)&Ani->GetAniName())){
															if(!Ani->GetAniName().IsEmpty()){
																if(Ani->GetAniName().Search(Door) == 2){
																	if(!this->player[i]->npc->GetInteractMob()){
																		oCMobLockable* ALocked = static_cast<oCMobLockable*>(this->player[i]->npc->FindMobInter(Door));
																		if(ALocked){
																			ALocked->SetMobBodyState(this->player[i]->npc);
																			ALocked->AI_UseMobToState(this->player[i]->npc, !ALocked->GetState());
																		}
																	}
																	else{
																		this->player[i]->npc->GetInteractMob()->SetMobBodyState(this->player[i]->npc);
																		this->player[i]->npc->GetInteractMob()->AI_UseMobToState(this->player[i]->npc, !this->player[i]->npc->GetInteractMob()->GetState());
																	}
																}
																else if(this->player[i]->npc->GetInteractMob()){
																	if(this->player[i]->npc->GetInteractMob()->GetState() == 0)
																		this->player[i]->npc->GetInteractMob()->SendEndInteraction(this->player[i]->npc, 0, 1);
																	else this->player[i]->npc->GetInteractMob()->SendEndInteraction(this->player[i]->npc, 1, 0);
																}
																if(!this->player[i]->npc->GetModel()->IsAnimationActive(Ani->GetAniName())){
																	this->player[i]->npc->GetModel()->StartAnimation(Ani->GetAniName());
																	if(!memcmp("T_BOWRELOAD", Ani->GetAniName().ToChar(), 11)){
																		oCItem* Bullet = oCObjectFactory::GetFactory()->CreateItem(zCParser::GetParser()->GetIndex(Arrows));
																		zCSoundSystem::GetSoundSystem()->PlaySound3D(BowSound, this->player[0]->npc, 2);
																		this->player[i]->npc->SetRightHand(Bullet);
																		oCItem* Arrowe = this->player[i]->npc->GetInventory()->IsIn(7083, 1);
																		if(Arrowe) this->player[i]->npc->GetInventory()->Remove(7083, 1);
																		this->player[i]->npc->DoShootArrow(1);
																	}
																	if(!memcmp("T_CBOWRELOAD", Ani->GetAniName().ToChar(), 12)){
																		oCItem* Bullet = oCObjectFactory::GetFactory()->CreateItem(zCParser::GetParser()->GetIndex(Bolt));
																		zCSoundSystem::GetSoundSystem()->PlaySound3D(CrossbowSound, this->player[0]->npc, 2);
																		this->player[i]->npc->SetLeftHand(Bullet);
																		oCItem* Bolte = this->player[i]->npc->GetInventory()->IsIn(7084, 1);
																		if(Bolte) this->player[i]->npc->GetInventory()->Remove(7084, 1);
																		this->player[i]->npc->DoShootArrow(1);
																	}
																	if(!memcmp("T_LEVER_S0_2_S1", Ani->GetAniName().ToChar(), 15)){
																		oCMobInter* LeverSwitch = this->player[i]->npc->FindMobInter(Lever);
																		if(LeverSwitch){
																			if(!LeverSwitch->GetTriggerName().IsEmpty()){
																				zCMover* Mover = static_cast<zCMover*>(oCGame::GetGame()->GetGameWorld()->SearchVobByName(LeverSwitch->GetTriggerName()));
																				if(Mover)Mover->TriggerMover(Mover);
																			}
																		}
																	}
																	if(!memcmp("T_TOUCHPLATE_S0_2_S1", Ani->GetAniName().ToChar(), 20) || !memcmp("T_TOUCHPLATE_S1_2_S0", Ani->GetAniName().ToChar(), 20)){
																		oCMobInter* LeverSwitch = this->player[i]->npc->FindMobInter(Touchplate);
																		if(LeverSwitch){
																			if(!LeverSwitch->GetTriggerName().IsEmpty()){
																				zCMover* Mover = static_cast<zCMover*>(oCGame::GetGame()->GetGameWorld()->SearchVobByName(LeverSwitch->GetTriggerName()));
																				if(Mover)Mover->TriggerMover(Mover);
																			}
																		}
																	}
																	if(!memcmp("T_VWHEEL_S0_2_S1", Ani->GetAniName().ToChar(), 16)){
																		oCMobInter* LeverSwitch = this->player[i]->npc->FindMobInter(VWheel);
																		if(LeverSwitch){
																			if(!LeverSwitch->GetTriggerName().IsEmpty()){
																				zCMover* Mover = static_cast<zCMover*>(oCGame::GetGame()->GetGameWorld()->SearchVobByName(LeverSwitch->GetTriggerName()));
																				if(Mover)Mover->TriggerMover(Mover);
																			}
																		}
																	}
																}
															}
														}
													}
												}
												//if(!this->player[i]->npc->GetModel()->IsAnimationActive(this->player[i]->npc->GetModel()->GetAniFromAniID(static_cast<int>(word))->GetAniName())) this->player[i]->npc->GetModel()->StartAnimation(this->player[i]->npc->GetModel()->GetAniFromAniID(static_cast<int>(word))->GetAniName());
												//sprawdzanie animacji
												memcpy(&word, p->data+pIt, 2); pIt+=2;
												if(word>this->player[i]->npc->GetHealth()){
													if(this->player[i]->npc->GetHealth()==1){
														this->player[i]->npc->RefreshNpc();
														this->player[i]->npc->SetHealth(1);
													}
												}
												if((!this->player[i]->hp) && (word==this->player[i]->npc->GetMaxHealth())){
													this->player[i]->hp=word;
													//this->player[i]->npc->RefreshNpc();
													this->player[i]->npc->ResetPos(this->player[i]->npc->GetPosition());
												} else if((this->player[i]->npc->GetHealth()>0) && (word==0)){ /*this->player[i]->npc->DoDie();*/ this->player[i]->hp=0; }
												else{
													if(this->player[i]->update_hp_packet>=5){
														this->player[i]->hp=word;
														this->player[i]->npc->SetHealth(static_cast<int>(word));
														this->player[i]->update_hp_packet=0;
													} else this->player[i]->update_hp_packet++;
												}
												//update hp
												memcpy(&word, p->data+pIt, 2); pIt+=2;
												this->player[i]->npc->SetMana(static_cast<int>(word));
												// update many
												memcpy(&word, p->data+pIt, 1); pIt++;
												BYTE SpellNr = static_cast<BYTE>(word);
												if(SpellNr != this->player[i]->npc->GetActiveSpellNr() && SpellNr > 0 && SpellNr < 100) 
												{
													for (int s = 0; s < this->player[i]->npc->GetSpellBook()->GetNoOfSpells(); s++){
														this->player[i]->npc->Equip(this->player[i]->npc->GetSpellBook()->GetSpellItem(s));
													}
													oCItem* SpellItem = this->player[i]->npc->GetSpellItem((int)SpellNr);
													if(SpellItem){
														this->player[i]->npc->Equip(SpellItem);
														this->player[i]->npc->GetSpellBook()->Open(0);
													}
												}
												else if (SpellNr == 0 && this->player[i]->npc->GetActiveSpellNr() > 0) this->player[i]->npc->GetSpellBook()->Close(1);
												else if (this->player[i]->npc->IsDead()) this->player[i]->npc->GetSpellBook()->Close(1);
												// update spella w rêkach
												if((BYTE)player[i]->npc->GetWeaponMode()!=p->data[pIt]) player[i]->npc->SetWeaponMode((oCNpc_WeaponMode)p->data[pIt]); pIt++;
												//sprawdzenie pozycji bojowej
												switch((CPlayer::HeadState)p->data[pIt])
												{
													case CPlayer::HEAD_NONE:
														player[i]->npc->GetAnictrl()->SetLookAtTarget(0.5f,0.5f);
													break;
													case CPlayer::HEAD_LEFT:
														player[i]->npc->GetAnictrl()->SetLookAtTarget(0.0f,0.5f);
													break;
													case CPlayer::HEAD_RIGHT:
														player[i]->npc->GetAnictrl()->SetLookAtTarget(1.0f,0.5f);
													break;
													case CPlayer::HEAD_UP:
														player[i]->npc->GetAnictrl()->SetLookAtTarget(0.5f,0.0f);
													break;
													case CPlayer::HEAD_DOWN:
														player[i]->npc->GetAnictrl()->SetLookAtTarget(0.5f,1.0f);
													break;
												}
												pIt++;
												// synchro obrotu g³owy
												memcpy(&word, p->data+pIt, 2); pIt+=2;
												if(word==0){
													if(this->player[i]->npc->GetEquippedRangedWeapon()){
														this->player[i]->npc->UnequipItem(this->player[i]->npc->GetEquippedRangedWeapon());
													}
												} else if (word > 5892 && word < 7850){
													if(!this->player[i]->npc->GetEquippedRangedWeapon()){
														if(!this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1)){
															oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
															this->player[i]->npc->GetInventory()->Insert(New);
															this->player[i]->npc->Equip(New);
														}
														else this->player[i]->npc->Equip(this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1));
													} else{
														if(this->player[i]->npc->GetEquippedRangedWeapon()->GetInstance()!=static_cast<int>(word)){
															if(!this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1)){
																oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
																this->player[i]->npc->GetInventory()->Insert(New);
																this->player[i]->npc->Equip(New);
															}
															else this->player[i]->npc->Equip(this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1));
														}
													}
												}
												//sprawdzanie broni dystansowej
												memcpy(&word, p->data+pIt, 2); pIt+=2;
												if(word==0){
													if(this->player[i]->npc->GetEquippedMeleeWeapon()){
														this->player[i]->npc->UnequipItem(this->player[i]->npc->GetEquippedMeleeWeapon());
													}
												} else if (word > 5892 && word < 7850){
													if(!this->player[i]->npc->GetEquippedMeleeWeapon()){
														if(!this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1)){
															oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
															this->player[i]->npc->GetInventory()->Insert(New);
															this->player[i]->npc->Equip(New);
														}
														else this->player[i]->npc->Equip(this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1));
													} else{
														if(this->player[i]->npc->GetEquippedMeleeWeapon()->GetInstance()!=static_cast<int>(word)){
															if(!this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1)){
																oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(word));
																this->player[i]->npc->GetInventory()->Insert(New);
																this->player[i]->npc->Equip(New);
															}
															else this->player[i]->npc->Equip(this->player[i]->npc->GetInventory()->IsIn(static_cast<int>(word), 1));
														}
													}
												}
												//sprawdzanie broni do walki w rêcz
												} else{
													pIt+=sizeof(uint64_t);
													if(this->player[0]->update_hp_packet>=5){
														short last_hp=this->player[0]->hp;
														memcpy(&this->player[0]->hp, p->data+pIt, 2); pIt+=2;
														if((!last_hp) && (this->player[0]->hp==this->player[0]->npc->GetMaxHealth())){
															this->player[0]->npc->RefreshNpc();
															this->player[0]->npc->SetMovLock(0);
															if(game_mode==0){
																this->player[0]->npc->ResetPos(*(*spawnpoint)[rand()%spawnpoint->GetSize()]);
															}
															if(game_mode==1){
																std::vector<const char*> team_list;
																team_list.push_back((*classmgr)[0]->team_name.ToChar());
																for(size_t x=1; x<classmgr->GetSize(); x++){
																	bool match=false;
																	for(size_t y=0; y<team_list.size(); y++) if(!memcmp((*classmgr)[x]->team_name.ToChar(), team_list[y], strlen(team_list[y])+1)){ match=true; break;}
																	if(!match) team_list.push_back((*classmgr)[x]->team_name.ToChar());
																}
																size_t z;
																for(z=0; z<team_list.size(); z++) if(!memcmp((*classmgr)[player[0]->char_class]->team_name.ToChar(), team_list[z], strlen(team_list[z])+1)) break;
																this->player[0]->npc->ResetPos(*(*spawnpoint)[(rand()%(spawnpoint->GetSize()/team_list.size()))*team_list.size()+z]);
																team_list.clear();
															}
														}
														else this->player[0]->SetHealth(static_cast<int>(this->player[0]->hp));
														this->player[0]->update_hp_packet=0;
														if(static_cast<int>(this->player[0]->hp) < this->HeroLastHp){
															if(!this->player[0]->npc->IsDead() && this->player[0]->npc->GetBodyState() != 10 && this->player[0]->npc->GetBodyState() != 12 && this->player[0]->npc->GetBodyState() != 22 && !this->player[0]->npc->GetAnictrl()->IsInWater()){
																if(this->player[0]->npc->GetAnictrl()->IsRunning()) this->player[0]->npc->GetModel()->StartAnimation(GOTHIT);
																else this->player[0]->npc->GetModel()->StartAnimation(STUMBLE);
																int RandomSound = rand() % 2 + 1;
																if(RandomSound == 2) zCSoundSystem::GetSoundSystem()->PlaySound3D(HitSound, this->player[0]->GetNpc(), 2);
															}
															this->HeroLastHp = static_cast<int>(this->player[0]->hp);
														}
														else if(static_cast<int>(this->player[0]->hp) != this->HeroLastHp) this->HeroLastHp = static_cast<int>(this->player[0]->hp);
													} else{
														this->player[0]->update_hp_packet++;
														pIt+=2;
													}
												}
											} else pIt+=sizeof(uint64_t)+51;
										}
									}
									break;
								case PT_MAP_ONLY:
									{
										bool found;
										unsigned int pIt=1;
										while(pIt<p->length){
											size_t i;
											found=false;
											for(i=0; i<this->player.size(); i++) if(!memcmp(p->data+pIt, &this->player[i]->id, sizeof(uint64_t))) { found=true; break; }
											if(found){
												pIt+=sizeof(uint64_t);
												float pos[2];
												memcpy(pos, p->data+pIt, 8);
												this->player[i]->npc->SetPosition(pos[0], this->player[i]->npc->GetPosition().y, pos[1]);
												this->player[i]->DisablePlayer();
												pIt+=8;
											} else pIt+=8+sizeof(uint64_t);
										}
									}
									break;
								case PT_DODIE:
									for(size_t i=0; i<player.size(); i++){
										if(!memcmp(&player[i]->id, p->data+1, sizeof(uint64_t))){
											player[i]->hp=0;
											player[i]->update_hp_packet=0;
											player[i]->SetHealth(0);
										}
									}
									break;
								case PT_RESPAWN:
									for(size_t i=0; i<player.size(); i++){
										if(!memcmp(&player[i]->id, p->data+1, sizeof(uint64_t))){
											player[i]->RespawnPlayer();
										}
									}
									break;
								case PT_CASTSPELL:
									{
										short SpellId = 0;
										memcpy(&SpellId, p->data+1+sizeof(uint64_t), 1);
										if(SpellId >= 0 && SpellId < 100){
											for(size_t i=0; i<player.size(); i++){
												if(!memcmp(&player[i]->id, p->data+1, sizeof(uint64_t))){
													oCSpell* Spell = oCSpell::_CreateNewInstance();
													Spell->InitValues(SpellId);
													Spell->Setup(player[i]->GetNpc(), 0, 0);
													RunSpellLogic(SpellId, player[i]->GetNpc(), 0);
													RunSpellScript(Spell->GetSpellInstanceName(SpellId).ToChar(), player[i]->GetNpc());
													Spell->Cast();
												}
											}
										}
									}
									break;
								case PT_CASTSPELLONTARGET:
									{
										uint64_t TargetId;
										short SpellId = 0;
										memcpy(&TargetId, p->data+1+sizeof(uint64_t), 8);
										memcpy(&SpellId, p->data+9+sizeof(uint64_t), 1);
										if(SpellId >= 0 && SpellId < 100){
											for(size_t i=0; i<player.size(); i++){
												if(!memcmp(&player[i]->id, p->data+1, sizeof(uint64_t))){
													for(size_t s=0; s<player.size(); s++){
														if(!memcmp(&player[s]->id, &TargetId, sizeof(uint64_t))){
															oCSpell* Spell = oCSpell::_CreateNewInstance();
															Spell->InitValues(SpellId);
															Spell->Setup(player[i]->GetNpc(), player[s]->GetNpc(), 0);
															RunSpellLogic(SpellId, player[i]->GetNpc(), player[s]->GetNpc());
															RunSpellScript(Spell->GetSpellInstanceName(SpellId).ToChar(), player[i]->GetNpc(), player[s]->GetNpc());
															Spell->Cast();
														}
													}
												}
											}
										}
									}
									break;
								case PT_DROPITEM:
									{
										short instance;
										short amount;
										memcpy(&instance, p->data+1+sizeof(uint64_t), 2);
										memcpy(&amount, p->data+3+sizeof(uint64_t), 2);
										if(instance > 5892 && instance < 7850){
											for(size_t i=0; i<player.size(); i++){
												if(!memcmp(&player[i]->id, p->data+1, sizeof(uint64_t))){
													oCWorld* world = oCGame::GetGame()->GetGameWorld();
													oCItem* NpcDrop = oCObjectFactory::GetFactory()->CreateItem(instance);
													NpcDrop->SetAmount(amount);
													zVEC3 startPos = this->player[i]->npc->GetTrafoModelNodeToWorld("ZS_RIGHTHAND").GetTranslation();
													NpcDrop->SetPosition(startPos);
													world->AddVob(NpcDrop);
													NpcDrop->SetSleeping(false);
													NpcDrop->SetStaticVob(false);
													NpcDrop->SetPhysicsEnabled(true);
												}
											}
										}
									}
									break;
								case PT_TAKEITEM:
									{
										short number;
										memcpy(&number, p->data+1+sizeof(uint64_t), 2);
										if(number > 5892 && number < 7850){
											zCListSort<oCItem>* ItemList = oCGame::GetGame()->GetWorld()->GetItemList();
											int size = ItemList->GetSize();
											for(size_t x=0; x<player.size(); x++){
												if(!memcmp(&player[x]->id, p->data+1, sizeof(uint64_t))){
													for(int i=0; i<size; i++){
														ItemList = ItemList->GetNext();
														oCItem* ItemInList = ItemList->GetData();
														if(ItemInList->GetInstance() == number){
															if(player[x]->npc->GetDistanceToVob(ItemInList) < 250.0f){
																player[x]->npc->DoTakeVob(ItemInList);
																break;
															}
														}
													}
												}
											}
										}
									}
									break;
								case PT_WHISPER:
									{
										size_t i;
										bool found=false;
										for(i=0; i<this->player.size(); i++) if(!memcmp(p->data+1, &this->player[i]->id, sizeof(uint64_t))) { found=true; break; }
										if(found){
											CChat::GetInstance()->WriteMessage(WHISPER, true, zCOLOR(0, 255, 255, 255), "%s-> %s", this->player[i]->npc->GetName().ToChar(), (const char*)p->data+1+sizeof(uint64_t)*2);
										}
									}
									break;
								case PT_MSG:
									{
										size_t i=0;
										bool found=false;
										for(i=0; i<this->player.size(); i++) if(!memcmp(p->data+1, &this->player[i]->id, sizeof(uint64_t))) { found=true; break; }
										if(found){
											CChat::GetInstance()->WriteMessage(NORMAL, false, "%s: %s", this->player[i]->npc->GetName().ToChar(), (const char*)p->data+1+sizeof(uint64_t));
										}
									}
									break;
								case PT_SRVMSG:
									CChat::GetInstance()->WriteMessage(NORMAL, false, zCOLOR(255, 128, 0, 255), "(SERVER): %s", (const char*)p->data+1);
									break;
								case PT_RCON:
									if(p->data[1]==0x41){
										IsAdminOrModerator = true;
									}
									CChat::GetInstance()->WriteMessage(ADMIN, false, RED, "%s", (char*)p->data+1);
									break;
								case PT_ALL_OTHERS:
									{
										unsigned int pIt=1;
										while(pIt<p->length){
											CPlayer* newhero = new CPlayer();
											newhero->enable=FALSE;
											memcpy(&newhero->id, p->data+pIt, sizeof(uint64_t)); pIt+=sizeof(uint64_t);
											oCNpc* npc = oCObjectFactory::GetFactory()->CreateNpc(oCNpc::GetHero()->GetInstance());
											newhero->SetNpc(npc);
											newhero->char_class=p->data[pIt];
											classmgr->EquipNPC(p->data[pIt], newhero, true); pIt++;
											newhero->npc->SetGuild(9);
											newhero->hp=static_cast<short>(newhero->GetHealth());
											zVEC3 pos;
											memcpy(&pos, p->data+pIt, 12);
											newhero->SetPosition(pos); pIt+=12;
											pIt+=6;	//pomijam l/p d³oñ+zbroje
											if(newhero->Type == CPlayer::NPC_HUMAN) newhero->SetAppearance(p->data[pIt], p->data[pIt+1], p->data[pIt+2]); pIt+=3;
											if(newhero->Type > CPlayer::NPC_DRACONIAN || newhero->Type == CPlayer::NPC_HUMAN) newhero->npc->ApplyOverlay(CPlayer::GetWalkStyleFromByte(p->data[pIt])); pIt++;
											const char* heroname=(const char*)p->data+pIt; pIt+=strlen(heroname)+1;
											newhero->SetName(heroname);
											newhero->update_hp_packet=0;
											this->player.push_back(newhero);
										}
									}
									break;
								case PT_JOIN_GAME:	//ktoœ do³¹cza
									{
										CPlayer* newhero = new CPlayer();
										memcpy(&newhero->id, p->data+1, sizeof(uint64_t));
										zVEC3 pos;
										memcpy(&pos, p->data+(2+sizeof(uint64_t)), 12);
										oCNpc* npc = oCObjectFactory::GetFactory()->CreateNpc(oCNpc::GetHero()->GetInstance());
										newhero->SetNpc(npc);
										classmgr->EquipNPC(p->data[1+sizeof(uint64_t)], newhero, true);
										newhero->char_class=p->data[1+sizeof(uint64_t)];
										newhero->npc->SetGuild(9);
										newhero->hp=static_cast<short>(newhero->GetHealth());
										newhero->SetPosition(pos);
										const char* heroname=(const char*)p->data+(38+sizeof(uint64_t));
										newhero->SetName(heroname);
										if(newhero->Type == CPlayer::NPC_HUMAN) newhero->SetAppearance(p->data[sizeof(uint64_t)+34], p->data[sizeof(uint64_t)+35], p->data[sizeof(uint64_t)+36]);
										if(newhero->Type > CPlayer::NPC_DRACONIAN || newhero->Type == CPlayer::NPC_HUMAN) newhero->npc->ApplyOverlay(CPlayer::GetWalkStyleFromByte(p->data[sizeof(uint64_t)+37]));
										CChat::GetInstance()->WriteMessage(NORMAL, false, zCOLOR(0,255,0,255),"%s%s", heroname, (*lang)[CLanguage::SOMEONE_JOIN_GAME].ToChar());
										newhero->enable=FALSE;
										newhero->update_hp_packet=0;
										//kod
										this->player.push_back(newhero);
									}
									break;
								case PT_YOUR_NAME:
									{
										zSTRING tmpStr=(const char*)(p->data+1);
										oCNpc::GetHero()->SetName(tmpStr);
										tmpStr.Clear();
									}
									break;
								case PT_GAME_INFO:
									{
										STime t;
										memcpy(&t, p->data+1, 4);
										oCGame::GetGame()->SetTime(static_cast<int>(t.day), static_cast<int>(t.hour), static_cast<int>(t.min));
										if(!IgnoreFirstTimeMessage) CChat::GetInstance()->WriteMessage(NORMAL, false, "Time set to: %d:%.2d", t.hour, t.min);
										IgnoreFirstTimeMessage = false;
										this->game_mode=p->data[5];
										UsePotionKeys=p->data[6]&0x01;
										DropItemsAllowed=p->data[6]&0x02;
										ForceHideMap=p->data[6]&0x04;
										if(p->data[6]&0x08) memcpy(&mp_restore, p->data+7, 2);
										else mp_restore=0;
									}
									break;
								case PT_LEFT_GAME:
									for(size_t i=1; i<player.size(); i++){
										if(!memcmp(p->data+1, &player[i]->id, sizeof(uint64_t))){
											CChat::GetInstance()->WriteMessage(NORMAL, false, zCOLOR(255,0,0,255), "%s%s", player[i]->GetName(), (*lang)[CLanguage::SOMEONEDISCONNECT_FROM_SERVER].ToChar());
											player[i]->LeaveGame();
											delete player[i];
											player.erase(player.begin()+i);
											break;
										}
									}
									break;
								case ID_DISCONNECTION_NOTIFICATION:
								case ID_CONNECTION_LOST:
									error=p->data[0];
									delete LocalPlayer;
									CPlayer::DeleteAllPlayers();
									if(VobsWorldBuilderMap.size() > 0){
										for(int i = 0; i < (int)VobsWorldBuilderMap.size(); i++)
										{
											VobsWorldBuilderMap[i].Vob->RemoveVobFromWorld();
										}
										VobsWorldBuilderMap.clear();
									}
									oCNpc::GetHero()->ResetPos(oCNpc::GetHero()->GetPosition());
									disconnected=TRUE;
									interrupted=TRUE;
									CChat::GetInstance()->WriteMessage(NORMAL, false, zCOLOR(255,0,0,255), "%s", (*lang)[CLanguage::DISCONNECTED].ToChar());
									break;
								default:
									printf("OP code: %d | %x", static_cast<int>(p->data[0]), static_cast<int>(p->data[0]));
									break;
							}
							this->client->DeallocatePacket(p);
						}
					} while(p);
				}
				break;
			default:	//should never execute
				break;
		}
	}
}

unsigned char CGmpClient::GetPacketIdentifier(RakNet::Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char) p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else return (unsigned char) p->data[0];
}

BOOL CGmpClient::IsConnected(){ return this->connected_with_srv; }

zSTRING& CGmpClient::GetLastError(){
	switch(this->error){
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

BOOL CGmpClient::IsReadyToJoin(){ return (this->work_layer==WL_JOIN_TO_GAME)?TRUE:FALSE; }
void CGmpClient::JoinGame(BYTE selected_class){
	if(IsReadyToJoin()){
	HooksManager::GetInstance()->AddHook(HT_RENDER, (DWORD)InterfaceLoop, false);
	HooksManager::GetInstance()->RemoveHook(HT_RENDER, (DWORD)CSelectClass::Loop);
	BYTE *buffer=new BYTE[512];
	memset(buffer, 0, 512);
	buffer[0]=PT_JOIN_GAME;
	buffer[1]=selected_class;
	memcpy(buffer+2, &oCNpc::GetHero()->GetPosition(), 12);
	zVEC3 nr(oCNpc::GetHero()->GetAngleNX(), oCNpc::GetHero()->GetAngleNY(), oCNpc::GetHero()->GetAngleNZ());
	memcpy(buffer+14, &nr, 12);
	short word;
	oCItem *itPtr=oCNpc::GetHero()->GetLeftHand();
	if(itPtr){
		word=static_cast<short>(itPtr->GetInstance());
		memcpy(buffer+26, &word, 2);
	}
	itPtr=oCNpc::GetHero()->GetRightHand();
	if(itPtr){
		word=static_cast<short>(itPtr->GetInstance());
		memcpy(buffer+28, &word, 2);
	}
	itPtr=oCNpc::GetHero()->GetEquippedArmor();
	if(itPtr){
		word=static_cast<short>(itPtr->GetInstance());
		memcpy(buffer+30, &word, 2);
	}
	word=265;
	memcpy(buffer+32, &word, 2);
	buffer[34]=static_cast<BYTE>(user_config->headmodel);
	buffer[35]=static_cast<BYTE>(user_config->skintexture);
	buffer[36]=static_cast<BYTE>(user_config->facetexture);
	buffer[37]=static_cast<BYTE>(user_config->walkstyle);
	memcpy(buffer+38, oCNpc::GetHero()->GetName().ToChar(), oCNpc::GetHero()->GetName().Length()+1);
	client->Send((const char*)buffer, 40+oCNpc::GetHero()->GetName().Length(), IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, srvAddr, false);
	memset(buffer, 0, 512);
	delete buffer;
	this->work_layer++;
	CIngame *g=new CIngame(lang);
	if(!LocalPlayer) new CLocalPlayer();
	LocalPlayer->id=this->playerID;
	LocalPlayer->enable=TRUE;
	LocalPlayer->SetNpc(oCNpc::GetHero());
	LocalPlayer->SetName(oCNpc::GetHero()->GetName());
	LocalPlayer->hp=static_cast<short>(LocalPlayer->GetHealth());
	LocalPlayer->update_hp_packet=0;
	LocalPlayer->npc->SetMovLock(0);
	LocalPlayer->char_class=selected_class;
	this->player.push_back(LocalPlayer);
	this->HeroLastHp = oCNpc::GetHero()->GetHealth();
	}
}

void CGmpClient::SendMessage(const char* msg){
	std::string szData;
	szData.reserve(1024);
	*((unsigned char*)szData.data())=PT_MSG;
	memcpy((char*)szData.data()+1, msg, strlen(msg)+1);
	this->client->Send(szData.data(), strlen(msg)+2, HIGH_PRIORITY, RELIABLE_ORDERED, 0, this->srvAddr, false);
	szData.clear();
}

void CGmpClient::SendWhisper(const char *player_name, const char *msg){
	bool found=false;
	size_t i;
	size_t length=strlen(player_name);
	for(i=0; i<this->player.size(); i++){
		if(this->player[i]->GetNameLength()==length){
			if(!memcmp(this->player[i]->GetName(), player_name, length)) {found=true; break;}
		}
	}
	if(found){
		std::string szData;
		szData.reserve(1024);
		*((unsigned char*)szData.data())=PT_WHISPER;
		memcpy((char*)szData.data()+1, &this->player[i]->id, sizeof(uint64_t));
		memcpy((char*)szData.data()+1+sizeof(uint64_t), msg, strlen(msg)+1);
		this->client->Send(szData.data(), 2+sizeof(uint64_t)+strlen(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, this->srvAddr, false);
		szData.clear();
	}
}

void CGmpClient::SendRemoteMessage(const char *msg){
	std::string szData;
	szData.reserve(1024);
	*((unsigned char*)szData.data())=PT_RCON;
	memcpy((char*)szData.data()+1, msg, strlen(msg)+1);
	this->client->Send(szData.data(), 2+strlen(msg), HIGH_PRIORITY, RELIABLE, 1, this->srvAddr, false);
	szData.clear();
}

void CGmpClient::SendCastSpell(oCNpc* Target, short SpellId){
	std::string szData;
	szData.reserve(1024);
	uint64_t PlayerId = 0;
	if(Target){
		for(int i=0; i<(int)player.size(); i++){
			if(player[i]->npc == Target){
				PlayerId = player[i]->id;
				break;
			}
		}
	}
	if(!PlayerId){
		*((unsigned char*)szData.data())=PT_CASTSPELL;
		*((unsigned char*)szData.data()+1)=(BYTE)SpellId;
		this->client->Send(szData.data(), 2, HIGH_PRIORITY, RELIABLE, 0, this->srvAddr, false);
		szData.clear();
	}
	else{
		*((unsigned char*)szData.data())=PT_CASTSPELLONTARGET;
		memcpy((char*)szData.data()+1, &PlayerId, 8);
		*((unsigned char*)szData.data()+9)=(BYTE)SpellId;
		this->client->Send(szData.data(), 10, HIGH_PRIORITY, RELIABLE, 0, this->srvAddr, false);
		szData.clear();
	}
}

void CGmpClient::SendDropItem(short Instance, short amount){
	std::string szData;
	szData.reserve(1024);
	*((unsigned char*)szData.data())=PT_DROPITEM;
	memcpy((char*)szData.data()+1, &Instance, 2);
	memcpy((char*)szData.data()+3, &amount, 2);
	this->client->Send(szData.data(), 5, HIGH_PRIORITY, RELIABLE, 0, this->srvAddr, false);
	szData.clear();
}

void CGmpClient::SendTakeItem(short Instance){
	std::string szData;
	szData.reserve(1024);
	*((unsigned char*)szData.data())=PT_TAKEITEM;
	memcpy((char*)szData.data()+1, &Instance, 2);
	this->client->Send(szData.data(), 3, HIGH_PRIORITY, RELIABLE, 0, this->srvAddr, false);
	szData.clear();
}

void CGmpClient::UpdatePlayerStats(short anim){
	oCNpc* Hero = oCNpc::GetHero();
	std::string szData;
	szData.reserve(1024);
	*((unsigned char*)szData.data())=PT_ACTUAL_STATISTICS;
	zVEC3 pos=Hero->GetPosition();
	memcpy((char*)szData.data()+1, &pos, 12);
	float nrot[3]={Hero->GetAngleNX(),Hero->GetAngleNY(),Hero->GetAngleNZ()};
	memcpy((char*)szData.data()+13, nrot, 12);
	memset((char*)szData.data()+25, 0, 6);
	if(Hero->GetLeftHand()){
		short word=static_cast<short>(Hero->GetLeftHand()->GetInstance());
		memcpy((char*)szData.data()+25, &word, 2);
	}
	if(Hero->GetRightHand()){
		short word=static_cast<short>(Hero->GetRightHand()->GetInstance());
		memcpy((char*)szData.data()+27, &word, 2);
	}
	if(Hero->GetEquippedArmor()){
		short word=static_cast<short>(Hero->GetEquippedArmor()->GetInstance());
		memcpy((char*)szData.data()+29, &word, 2);
	}
	memcpy((char*)szData.data()+31, &anim, 2);
	short mp=static_cast<short>(Hero->GetMana());
	memcpy((char*)szData.data()+33, &mp, 2);
	*((unsigned char*)szData.data()+35)=(BYTE)Hero->GetWeaponMode();
	memset((char*)szData.data()+36, 0, 6);
	if(Hero->GetActiveSpellNr() > 0){
		*((unsigned char*)szData.data()+36)=(BYTE)Hero->GetActiveSpellNr();
	}
	zVEC2 HeadVar = Hero->GetAnictrl()->GetLookAtPos();
	if(HeadVar.x == 0) *((unsigned char*)szData.data()+37)=(BYTE)CPlayer::HEAD_LEFT;
	else if(HeadVar.x == 1) *((unsigned char*)szData.data()+37)=(BYTE)CPlayer::HEAD_RIGHT;
	else if(HeadVar.y == 0) *((unsigned char*)szData.data()+37)=(BYTE)CPlayer::HEAD_UP;
	else if(HeadVar.y == 1) *((unsigned char*)szData.data()+37)=(BYTE)CPlayer::HEAD_DOWN;
	if(Hero->GetEquippedRangedWeapon()){
		short word=static_cast<short>(Hero->GetEquippedRangedWeapon()->GetInstance());
		memcpy((char*)szData.data()+38, &word, 2);
	}
	if(Hero->GetEquippedMeleeWeapon()){
		short word=static_cast<short>(Hero->GetEquippedMeleeWeapon()->GetInstance());
		memcpy((char*)szData.data()+40, &word, 2);
	}
	this->client->Send(szData.data(), 42, IMMEDIATE_PRIORITY, UNRELIABLE, 0, this->srvAddr, false);
}

void CGmpClient::SendHPDiff(size_t who, short diff){
	if(who<this->player.size()){
		std::string szData;
		szData.reserve(32);
		*((unsigned char*)szData.data())=PT_HP_DIFF;
		memcpy((char*)szData.data()+1, &this->player[who]->id, sizeof(uint64_t));
		memcpy((char*)szData.data()+1+sizeof(uint64_t), &diff, 2);
		this->client->Send(szData.data(), 3+sizeof(uint64_t), IMMEDIATE_PRIORITY, RELIABLE, 2, this->srvAddr, false);
		szData.clear();
	}
}

void CGmpClient::SyncGameTime(){
	BYTE data[2]={PT_GAME_INFO, 0};
	this->client->Send((char*)data, 1, IMMEDIATE_PRIORITY, RELIABLE, 8, this->srvAddr, false);
}
bool CGmpClient::IsDisconnected(){ return (disconnected)?true:false; }
void CGmpClient::Disconnect(){
	if(!disconnected){
		IgnoreFirstTimeMessage = true;
		global_ingame->IgnoreFirstSync = true;
		delete global_ingame->Santa;
		global_ingame->Santa = NULL;
		LocalPlayer->SetNpcType(CPlayer::NPC_HUMAN);
		client->CloseConnection(this->srvAddr, true, 11, IMMEDIATE_PRIORITY);
		Sleep(1000);
		RakNet::RakPeerInterface::DestroyInstance(this->client);
		this->client=NULL;
		delete LocalPlayer;
		CPlayer::DeleteAllPlayers();
		if(VobsWorldBuilderMap.size() > 0){
			for(int i = 0; i < (int)VobsWorldBuilderMap.size(); i++)
			{
				VobsWorldBuilderMap[i].Vob->RemoveVobFromWorld();
			}
			VobsWorldBuilderMap.clear();
		}
		CChat::GetInstance()->ClearChat();
		global_ingame->WhisperingTo.clear();
		oCNpc::GetHero()->SetWeaponMode(NPC_WEAPON_NONE);
		disconnected=TRUE;
	}
	if(this->classmgr){
		delete this->classmgr;
		this->classmgr=NULL;
	}
	if(this->spawnpoint){
		delete this->spawnpoint;
		this->spawnpoint=NULL;
	}
}
bool CGmpClient::IsInterrupted(){ return (interrupted)?true:false; }
MD5Sum *CGmpClient::GetMD5(LPBYTE data, DWORD size){
	HCRYPTPROV hCryptProv;
	HCRYPTHASH hHash;
	DWORD dwHashLen;
	DWORD len;
	MD5Sum *result=NULL;

	if(CryptAcquireContextA(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)){
		if(CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash)){
			if(CryptHashData(hHash, data, size, 0)){
				CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&dwHashLen, &len, 0);
				if(dwHashLen==16){
					result=new MD5Sum;
					CryptGetHashParam(hHash, HP_HASHVAL, (BYTE*)result, &dwHashLen, 0);
					CryptDestroyHash(hHash);
					CryptReleaseContext(hCryptProv, 0);
				}
			}
		}
	}
	return result;
}

BYTE CGmpClient::AreDefaultItems(){
	zSTRING greatStringOfPunishment="\0";
	char numBuff[16];
	Patch::ObjectDestructorPatched(true);
	for(size_t i=0; i<866; i++){
		oCItem *item=oCObjectFactory::GetFactory()->CreateItem(GOTHIC_DAT_ITEMS[i]);
		greatStringOfPunishment+=item->GetInstanceName().ToChar();
		sprintf(numBuff, "%d\0", item->hp);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->hp_max);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->mainflags);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->flags);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->weight);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->value);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->damageType);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->damageTotal);
		greatStringOfPunishment+=numBuff;
		for(int zxy=0; zxy<8; zxy++){
			sprintf(numBuff, "%d\0", item->damage[zxy]);
			greatStringOfPunishment+=numBuff;
		}
		sprintf(numBuff, "%d\0", item->wear);
		greatStringOfPunishment+=numBuff;
		for(int zx=0; zx<8; zx++){
			sprintf(numBuff, "%d\0", item->protection[zx]);
			greatStringOfPunishment+=numBuff;
		}
		sprintf(numBuff, "%d\0", item->nutrition);
		greatStringOfPunishment+=numBuff;
		for(int z=0; z<3; z++){
			sprintf(numBuff, "%d\0", item->cond_atr[z]);
			greatStringOfPunishment+=numBuff;
			sprintf(numBuff, "%d\0", item->cond_value[z]);
			greatStringOfPunishment+=numBuff;
			sprintf(numBuff, "%d\0", item->change_atr[z]);
			greatStringOfPunishment+=numBuff;
			sprintf(numBuff, "%d\0", item->change_value[z]);
			greatStringOfPunishment+=numBuff;
		}
		sprintf(numBuff, "%d\0", item->material);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->munition);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->spell);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->range);
		greatStringOfPunishment+=numBuff;
		sprintf(numBuff, "%d\0", item->mag_circle);
		greatStringOfPunishment+=numBuff;
		oCGame::GetGame()->GetGameWorld()->RemoveVob(item);
		item->Delete(1);
	}
	MD5Sum *sum=GetMD5((BYTE*)greatStringOfPunishment.ToChar(), greatStringOfPunishment.Length());
	greatStringOfPunishment.Clear();
	Patch::ObjectDestructorPatched(false);
	BYTE ret_val=0;
	for(size_t lol=0; lol<16; lol++) if(static_cast<BYTE>(sum->data[lol]^0xFF)!=GDI_MD5[lol]) ret_val++;
	memset(sum, 0, 16);
	delete sum;
	return ret_val;
}
