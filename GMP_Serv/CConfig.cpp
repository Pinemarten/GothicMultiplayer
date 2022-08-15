
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

#pragma warning (disable : 4996)
#include "CConfig.h"
#include <slikenet/sleep.h>
#include <fstream>
#include <cstring>
#include <cstdio>

const char *DEFAULT_SRV_NAME="G-MP\x7Server";
const char *DEFAULT_MAP="NEWWORLD\\NEWWORLD.ZEN";
const char *DEFAULT_MSG="See us at www.your-site.com";
const char *DEFAULT_LOGFILE="log.txt";

STime *pTime=NULL;

RAK_THREAD_DECLARATION(STime::RunClock){
	while(1){
		RakSleep(4000);
		if(pTime){
			if(++pTime->min>59){
				pTime->min=0;
				if(++pTime->hour>23){
					pTime->hour=0;
					pTime->day++;
				}
			}
		}
	}
	return 0;
}

CConfig::CConfig() {
	this->Load();
	pTime=&game_time;
}

CConfig::~CConfig() {
	this->Unload();
}

void CConfig::Load(){
	this->SetDefault();
	std::string file_buffer;
	file_buffer.reserve(1024);
	std::ifstream fs("config.txt");
	if(!fs.is_open()){
		return;
	}
	do{
		fs.getline((char*)file_buffer.data(), 1024);
		for(size_t i=0; i<=file_buffer.length(); i++) if(*(file_buffer.data()+i)<0x20) *((unsigned char*)file_buffer.data()+i)=0;
		if((file_buffer[0]>0) && (file_buffer[0]!='#')){
			if(!memcmp(file_buffer.c_str(), "name ", 5)){
                if(!this->name.empty()) this->name.clear();
				//for(size_t lol=5; lol<file_buffer.length(); lol++) if(file_buffer[lol]==' ') *((unsigned char*)file_buffer.data()+lol)=0x07;
				unsigned char szTmp[128];
				memset(szTmp, 0, 128);
				memcpy(szTmp, file_buffer.data()+5, strlen(file_buffer.data()+4));
				for(int lol=0; lol<strlen(file_buffer.data()+4); lol++) if(szTmp[lol]==' ') szTmp[lol]=0x07;
				this->name=(char*)szTmp;
			}
			if(!memcmp(file_buffer.c_str(), "public ", 7)){
				sscanf(file_buffer.c_str()+7, "%d", &this->is_public);
			}
			if(!memcmp(file_buffer.c_str(), "slots ", 6)){
				sscanf(file_buffer.c_str()+6, "%d", &this->slots);
				if(this->slots<0) this->slots*=(-1);
			}
			if(!memcmp(file_buffer.c_str(), "log_file ", 9)){
				this->logfile=file_buffer.c_str()+9;
			}
			if(!memcmp(file_buffer.c_str(), "game_port ", 10)){
				sscanf(file_buffer.c_str()+10, "%hu", &this->game_port);
			}
			if(!memcmp(file_buffer.c_str(), "admin_passwd ", 13)){
				this->admin_passwd=file_buffer.c_str()+13;
			}
			if(!memcmp(file_buffer.c_str(),"time ", 5)){
				unsigned short h, m;
				sscanf(file_buffer.c_str()+5, "%hu%hu", &h, &m);
				this->game_time.hour=(unsigned char)h;
				this->game_time.min=(unsigned char)m;
			}
			if(!memcmp(file_buffer.c_str(), "map ", 4)){
				sscanf(file_buffer.c_str()+4, "%s", (char*)file_buffer.data());
				this->map=file_buffer.c_str();
			}
			if(!memcmp(file_buffer.c_str(), "respawn_time ", 12)){
				sscanf(file_buffer.c_str()+12, "%d", &this->respawn_time);
			}
			if(!memcmp(file_buffer.c_str(), "loop_time ", 10)){
				sscanf(file_buffer.c_str()+10, "%d", &this->loop_time);
			}
			if(!memcmp(file_buffer.c_str(), "loop_msg ", 9)){
				this->loop_msg=file_buffer.c_str()+9;
			}
			if(!memcmp(file_buffer.c_str(), "gamemode ", 9)){
				sscanf(file_buffer.c_str()+9, "%hd", &this->game_mode);
			}
			if(!memcmp(file_buffer.c_str(), "log_mode ", 9)){
				sscanf(file_buffer.c_str()+9, "%hd", &this->log_mode);
			}
			if(!memcmp(file_buffer.c_str(), "allow_modification ", 19)){
				sscanf(file_buffer.c_str()+19, "%hd", &this->allow_modification);
			}
			if(!memcmp(file_buffer.c_str(), "allow_dropitems ", 16)){
				sscanf(file_buffer.c_str()+16, "%hd", &this->allow_dropitems);
			}
			if(!memcmp(file_buffer.c_str(), "quick_potions ", 14)){
				sscanf(file_buffer.c_str()+14, "%hu", &this->quick_pots);
			}
			if(!memcmp(file_buffer.c_str(), "real_chat ", 10)){
				sscanf(file_buffer.c_str()+10, "%hu", &this->real_chat);
			}
			if(!memcmp(file_buffer.c_str(), "hide_map ", 9)){
				sscanf(file_buffer.c_str()+9, "%hu", &this->hide_map);
			}
			if(!memcmp(file_buffer.c_str(), "hp_regeneration ", 16)){
				sscanf(file_buffer.c_str()+16, "%hd", &this->hp_regeneration);
			}
			if(!memcmp(file_buffer.c_str(), "mp_regeneration ", 16)){
				sscanf(file_buffer.c_str()+16, "%hd", &this->mp_regeneration);
			}
			if(!memcmp(file_buffer.c_str(), "observation ", 12)){
				sscanf(file_buffer.c_str()+12, "%hd", &this->observation);
			}
			if(!memcmp(file_buffer.c_str(), "md5 ", 4)){
				char hex[3]={0,0,0};
				const char *szMD5=file_buffer.c_str()+4;
				for(int i=0; i<16; i++){
					hex[0]=szMD5[i*2];
					hex[1]=szMD5[1+i*2];
                    unsigned int gcc_fix;
					sscanf(hex, "%x", &gcc_fix);
                    map_md5[i]=(unsigned char)gcc_fix;
				}
			}
			if(!memcmp(file_buffer.c_str(), "unconcious ", 11)){
				sscanf(file_buffer.c_str()+11, "%hd", &this->be_unconcious_before_dead);
			}
			if(!memcmp(file_buffer.c_str(), "host_ip ", 8)){
				this->host_addr=file_buffer.c_str()+8;
			}
#ifndef WIN32
			if(!memcmp(file_buffer.c_str(), "daemon ", 7)){
				sscanf(file_buffer.c_str()+7, "%hd", &this->daemon);
			}
#endif
		}
		memset((void*)file_buffer.data(), 0, 1024);
	} while(!fs.eof());
	file_buffer.clear();
	fs.close();
}

void CConfig::Unload(){
	if(!this->admin_passwd.empty()) this->admin_passwd.clear();
	this->logfile.clear();
	this->loop_msg.clear();
	this->map.clear();
	this->name.clear();
}

void CConfig::SetDefault(){
	if(!this->admin_passwd.empty()) this->admin_passwd.clear();
	if(!this->logfile.empty()) this->logfile.clear();
	if(!this->loop_msg.empty()) this->loop_msg.clear();
	if(!this->map.empty()) this->map.clear();
	if(!this->name.empty()) this->name.clear();
	if(!this->host_addr.empty()) this->host_addr.clear();
	this->logfile=DEFAULT_LOGFILE;
	this->loop_msg=DEFAULT_MSG;
	this->map=DEFAULT_MAP;
	this->name=DEFAULT_SRV_NAME;
	this->quick_pots=0;
	this->game_mode=0;
	this->game_port=0xDEAD;
	this->game_time.hour=8; //8.00
	this->game_time.min=0;
	this->game_time.day=1;
	this->is_public=0;
	this->loop_time=60;
	this->respawn_time=5;
	this->slots=4;  //czy może dać więcej?
	this->allow_modification=1; //niech se czitują za pomocą gmdk.
	this->allow_dropitems=1;
	this->hide_map=0;
	this->real_chat=0;
	this->be_unconcious_before_dead=0;
	this->hp_regeneration=0;
	this->mp_regeneration=0;
	this->observation=0;
#ifndef WIN32
	this->daemon=1;
#endif
}

void CConfig::ParseCmdLine(int argc, char **argv){
	for(int i=1; i<argc; i++){
		if(!memcmp("-n", argv[i], 2)){
			if(!this->name.empty()) this->name.clear();
			this->name=argv[++i];
			goto eol;
		}
		if(!memcmp("-s", argv[i], 2)){
			sscanf(argv[++i], "%d", &this->slots);
			goto eol;
		}
		if(!memcmp("-m", argv[i], 2)){
			if(!this->map.empty()) this->map.clear();
			this->map=argv[++i];
			goto eol;
		}
		if(!memcmp("-a", argv[i], 2)){
			if(!this->admin_passwd.empty()) this->admin_passwd.clear();
			this->admin_passwd=argv[++i];
			goto eol;
		}
		if(!memcmp("-h", argv[i], 2)){
			if(!this->host_addr.empty()) this->host_addr.clear();
			this->host_addr=argv[++i];
			goto eol;
		}
		if(!memcmp("-g", argv[i], 2)){
			sscanf(argv[++i], "%hd", &this->game_mode);
			goto eol;
		}
		if(!memcmp("-p", argv[i], 2)){
			sscanf(argv[++i], "%hu", &this->game_port);
			goto eol;
		}
		if(!memcmp("-P", argv[i], 2)){
			this->is_public=1;
			goto eol;
		}
		if(!memcmp("-l", argv[i], 2)){
			if(!this->logfile.empty()) this->logfile.clear();
			this->logfile=argv[++i];
			goto eol;
		}
		if(!memcmp("-L", argv[i], 2)){
			if(!this->loop_msg.empty()) this->loop_msg.clear();
			this->loop_msg=argv[++i];
			goto eol;
		}
		if(!memcmp("-r", argv[i], 2)){
			sscanf(argv[++i], "%d", &this->respawn_time);
			goto eol;
		}
		if(!memcmp("-c", argv[i], 2)){
			this->allow_modification=0;
			memcpy(this->map_md5, argv[++i], 16);
			goto eol;
		}
		if(!memcmp("-q", argv[i], 2)){
			this->quick_pots=1;
			goto eol;
		}
eol:
		continue;
	}
}
