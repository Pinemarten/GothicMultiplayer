
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

/*****************************************************************************
**																			**
**	File name:		Interface/CChat.h		   								**
**																			**
**	Created by:		29/06/11	-	skejt23									**
**																			**
**	Description:	Multiplayer chat functionallity	 						**
**																			**
*****************************************************************************/

#pragma once
#include <vector>
#include "g2Api.h"
#include "CConfig.h"

struct MsgStruct
{
	zSTRING Message;
	zCOLOR MsgColor;
};

enum MsgType
{
	NORMAL,
	WHISPER,
	ADMIN
};

class CChat : public TSingleton<CChat>
{
public:
	std::vector<MsgStruct> ChatMessages;
	std::vector<MsgStruct> WhisperMessages;
	std::vector<MsgStruct> AdminMessages;
	MsgType PrintMsgType;
	zCInput* Input;
	zSTRING tmp;
	zSTRING tmpnickname;
	zSTRING tmpanimname;
	char buffer[32];
	bool ShowHow;
public:
	CChat();
	~CChat();
	void ClearChat();
	void StartChatAnimation(int anim);
	void SetWhisperTo(std::string& whisperto);
	void WriteMessage(MsgType type, bool PrintTimed, zCOLOR& rgb, const char * format, ...);
	void WriteMessage(MsgType type, bool PrintTimed, const char * format, ...);
	void PrintChat();
};