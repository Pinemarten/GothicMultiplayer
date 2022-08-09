
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

#include "CGothicStarter.h"
#include <direct.h>
#include <cstdio>



CGothicStarter::CGothicStarter(std::string GothicSystemDir)
{
	this->SystemDir = GothicSystemDir;
	this->hProcess = NULL;
	this->hThread = NULL;
	this->hRemoteMem = NULL;
	this->hRemoteThread = NULL;
}

CGothicStarter::~CGothicStarter()
{
	if (this->hProcess != NULL)CloseHandle(this->hProcess);
	if(this->hThread != NULL) CloseHandle(this->hThread);
	if(this->hRemoteMem != NULL) VirtualFreeEx(this->hRemoteMem, hRemoteMem, NULL, MEM_RELEASE);
	if(this->hRemoteThread != NULL) CloseHandle(this->hRemoteThread);
}

std::string CGothicStarter::GetDllPath(std::string DllName)
{
	std::string workDir(_getcwd(NULL,NULL));
	workDir.append("\\");
	workDir.append(DllName);
	return workDir;
}



bool CGothicStarter::InjectDll(std::string DllName)
{	
	
	std::string DllPath = this->GetDllPath(DllName);

	HMODULE hLocKernel32 = GetModuleHandleA("Kernel32");
	if (hLocKernel32 == NULL) return false;

	FARPROC hLocLoadLibrary = GetProcAddress(hLocKernel32, "LoadLibraryA");
	if (hLocLoadLibrary == NULL) return false;

	this->hRemoteMem = VirtualAllocEx(this->hProcess, NULL, DllPath.size(), MEM_COMMIT, PAGE_READWRITE);
	if (this->hRemoteMem == NULL) return false;

	BOOL WrPrMemRet = WriteProcessMemory(this->hProcess, this->hRemoteMem, DllPath.c_str(), DllPath.size(), NULL);
	if (WrPrMemRet == FALSE) return false; 

	this->hRemoteThread = CreateRemoteThread(this->hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)hLocLoadLibrary, hRemoteMem, 0, NULL);
	if (this->hRemoteThread == NULL) return false;

	if (WaitForSingleObject(hRemoteThread, 10000) == WAIT_FAILED) return false;
	

	return true;

}


bool CGothicStarter::StartGothic(std::string GameName, UINT StartOptions, std::string DllName)
{
	
	std::string CMDLine = 	this->GetG2ExePath() + this->MakeCMDString(GameName, StartOptions);

	if (!this->CreateProc(CMDLine)) return false;
	

	if (!this->InjectDll(DllName)) return false;

	

	return true;
}

std::string CGothicStarter::MakeCMDString(std::string GameName, UINT opt)
{
	std::string cmdLine;
	cmdLine.append(" -game:");
	cmdLine.append(GameName);
	cmdLine.append(" -start");

	if(opt&WINDOW_MODE) cmdLine.append(" -zwindow");
	if(opt&NO_SOUND) cmdLine.append(" -znosound");
	if(opt&NO_MUSIC) cmdLine.append(" -znomusic");
	if(opt&REPARSE_SCRIPTS) cmdLine.append(" -zreparse");
	if(opt&TEX_CONVERT) cmdLine.append(" -ztexconvert");
	if(opt&AUTO_CONVERT_DATA) cmdLine.append(" -zautoconvertdata");
	if(opt&VDFS_PHYSICAL_FIRST) cmdLine.append(" -vdfs:physicalfirst");
	if(opt&DEV_MODE) cmdLine.append(" -devmode");
	if(opt&NO_MENU) cmdLine.append(" -nomenu");

	return cmdLine;

}

std::string CGothicStarter::GetG2ExePath()
{
	std::string Gothic2ExePath;
	Gothic2ExePath.append(this->SystemDir);
	Gothic2ExePath.append("\\");
	Gothic2ExePath.append("Gothic2.exe");
	return Gothic2ExePath;
}

bool CGothicStarter::CreateProc(std::string CmdLine)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	
	BOOL ret = CreateProcessA( 
		NULL,   // No module name (use command line)
		(LPSTR)CmdLine.c_str(),		// Command line
		NULL,		   // Process handle not inheritable
		NULL,		   // Thread handle not inheritable
		FALSE,		  // Set handle inheritance to FALSE
		0,			  // No creation flags
		NULL,		   // Use parent's environment block
		NULL,		   // Use parent's starting directory 
		&si,			// Pointer to STARTUPINFO structure
		&pi );		   // Pointer to PROCESS_INFORMATION structure

	if (ret != NULL) 
	{

		this->hThread = pi.hThread;
		this->hProcess = pi.hProcess;
		return true;
	}
	else return false;

}