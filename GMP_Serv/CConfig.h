
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

#ifndef CCONFIG_H
#define	CCONFIG_H
#include <string>
#include "RakThread.h"

union STime{
	int time;
	struct{
		unsigned short day;
		unsigned char hour ,min;
	};

	static RAK_THREAD_DECLARATION(RunClock);
};

class CConfig {
public:
	CConfig(void);
	~CConfig(void);
private:
	void SetDefault(void);
protected:
	void Load(void);
	void Unload(void);
	void ParseCmdLine(int argc, char **argv);
	
	std::string name;
	std::string logfile;
	std::string admin_passwd;
	std::string map;
	std::string loop_msg;
	std::string host_addr;
	unsigned short game_port;
	unsigned short quick_pots;
	int slots;
	int respawn_time;
	int loop_time;
	int is_public;
	STime game_time;
	short hp_regeneration;
	short mp_regeneration;
	short game_mode;
	short log_mode;
	short allow_modification;
	short allow_dropitems;
	short be_unconcious_before_dead;	//specjalnie na życzenie ojczulka
	short real_chat;	//tylko słychać z bliska? RP fanboy related
	short hide_map;		//wyłącz mape
	short observation;
	unsigned char map_md5[16];
#ifndef WIN32
	short daemon;
#endif
};

#endif

