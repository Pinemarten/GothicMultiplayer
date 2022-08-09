
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

#include "CLog.h"
#include "common.h"
#include <io.h>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
using namespace std;
#pragma warning(disable:4996)

#define COLOR_SUCCESS "#99FF99"
#define COLOR_ERROR "#FF6666"
#define COLOR_INFORMATION "#66CCFF"
#define COLOR_CHAT "#FFFFFF"

void AttachConsole(const char* title);
CLog::CLog()
{	
	//AttachConsole("Debug Window");
	this->file.open("GMLog.html");

		int width = 0;
		int height = 0;
		RECT rect;
		if(GetWindowRect(GetConsoleWindow(), &rect))
		{
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;
		};
		SetWindowPos(GetConsoleWindow(), NULL, (GetSystemMetrics(SM_CXSCREEN) - width + 100), (height - 340), width, height, 0);
		time_t  currtime;  
		char timesay[512];
		time(&currtime);   
		strftime(timesay,sizeof(timesay)-1,"STARTING LOG. DATE : %c",localtime(&currtime)); 
		this->Write(LV_INFORMATION, timesay);
}

CLog::~CLog(void)
{
	if(this->file.is_open()) this->file.close();
}
bool CLog::Write(MSG_LEVEL msgLevel, const char * format, ...)
{
	char line[1024];
	va_list args;
	va_start(args, format);
	vsprintf(line, format, args);
	va_end(args);
	switch(msgLevel)
	{
	case LV_ERROR:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED| FOREGROUND_INTENSITY);
		break;
	case LV_SUCCESS:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN| FOREGROUND_INTENSITY);
		break;
	case LV_INFORMATION:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |  FOREGROUND_GREEN | FOREGROUND_RED);
		break;
	case LV_CHAT:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN);
		break;
	default : return false;
	}
	puts(line);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN);

	if(this->file.is_open())
	{
		this->file << "<div style=\"background-color:"; 
		switch(msgLevel)
		{
		case LV_SUCCESS:
			this->file << COLOR_SUCCESS;
			break;
		case LV_INFORMATION:
			this->file << COLOR_INFORMATION;
			break;
		case LV_ERROR:
			this->file << COLOR_ERROR;
			break;
		case LV_CHAT:
			this->file << COLOR_CHAT;
		break;
		default:
			return false;
		}
		this->file << ";\"><li style=\"margin: 10px;\">";
		this->file << line <<  "</div>" << endl;

		return true;
	}
	else return false;
}

void AttachConsole(const char* title)
{
	AllocConsole();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);		// return an OS file handle
	int hCrt = _open_osfhandle((long)handle,0);	// return a runtime file handle
	FILE * hf = _fdopen( hCrt, "w" );					// stream
	char buf[2];
	setvbuf( hf, buf, _IONBF, 1 );
	*stdout = *hf;
	SetConsoleTitleA(title);
}