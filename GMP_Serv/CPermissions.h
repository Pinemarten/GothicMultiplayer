
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

#ifndef CPERMISSIONS_H
#define CPERMISSIONS_H

#include <list>
#include "../common/singleton.h"

struct SModerator{
	char* nickname;
	char* passwd;
};

class CPermissions:public TSingleton<CPermissions>{
private:
	std::list<SModerator*> mod_list;

	union{
		short data[12];
		struct{
			short say;
			short kick;
			short ban;
			short banip;
			short unban;
			short savebans;
			short mute;
			short settime;
			short kill;
			short loopmsg;
			short hp_regen;
			short mp_regen;
		};
	};
	void Load(void);
	void SetDefaults(void);
public:
	CPermissions(void);
	~CPermissions(void);

	void AddModerator(const char* nickname, const char* password);
	bool DeleteModerator(const char* nickname);
	SModerator *GetModerator(const char* nickname);
	int IsModerator(const char* nickname, const char* password);
	int IsPermitted(const char* command);
	bool SetPassword(const char* mod_nick, const char* password);
	bool SetPassword(SModerator* mod, const char* password);
};

#endif