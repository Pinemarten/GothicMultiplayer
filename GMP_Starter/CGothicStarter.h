
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

#pragma once


#include <windows.h>
#include <string>



class CGothicStarter
{
public:
	enum 
	{
		WINDOW_MODE = 1,
		NO_SOUND = 2,
		NO_MUSIC = 4,
		REPARSE_SCRIPTS = 8,
		TEX_CONVERT = 16,
		AUTO_CONVERT_DATA = 32,
		VDFS_PHYSICAL_FIRST = 64,
		DEV_MODE = 128,
		NO_MENU = 256
	};
public:
	CGothicStarter(std::string GothicSystemDir);
	~CGothicStarter();

	bool StartGothic(std::string GameName, UINT StartOptions, std::string DllName);

private:
	std::string MakeCMDString(std::string GameName, UINT opt);
	std::string GetG2ExePath();
	bool CreateProc(std::string CmdLine);
	bool InjectDll(std::string DllName);
	std::string GetDllPath(std::string DllName);
	std::string SystemDir;
	HANDLE hProcess;
	HANDLE hThread;
	HANDLE hRemoteThread;
	LPVOID hRemoteMem;
};