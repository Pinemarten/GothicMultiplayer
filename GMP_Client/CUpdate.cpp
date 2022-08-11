
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

#include "CUpdate.h"
#include <urlmon.h>
#include <cstdio>
#include "CMainMenu.h"
#include <WinCrypt.h>
#include <windows.h>
char* Version = "2.0.0b";
zSTRING CheckingUpdate = "Checking for updates...";
zSTRING DownloadingUpdate = "Downloading update. Please wait...";
const char* DllName = ".\\System\\GMP.dll";
HANDLE hProcess;
HANDLE hThread;
bool DevNoUpdate = true;

#pragma comment(lib, "urlmon.lib")

CUpdate::CUpdate() // Constructor
{
	RequiresMasterUpdate = false;
	if(DevNoUpdate) return;
	Screen = zCView::GetScreen();
	Screen->PrintCXY(CheckingUpdate);
	if(URLDownloadToFile(NULL, "yoursite.com", ".\\Update.gmp", 0, NULL)==S_OK){
		FILE* fH = fopen(".\\Update.gmp", "rb");
		fseek(fH, 0, SEEK_END);
		long file_size=ftell(fH);
		fseek(fH, 0, SEEK_SET);
		fread(buffer, file_size, 1, fH);
		fclose(fH);
		remove(".\\Update.gmp");
		for(int i=0; i<128; i++){
			if(buffer[i]==0) break;
				buffer[i]^=0xFF;
		}
		char GotVersion[6];
		char chs[64];
		memcpy(chs, buffer, 32);
		memcpy(GotVersion, buffer+33, 6);
		if(!memcmp(Version, GotVersion, 6)){} // wersja dobra
		else {
			RequiresMasterUpdate = true;
			return;
		}
		ZeroMemory(GotVersion, 5);
		std::string checksum;
		checksum=chs;
		ZeroMemory(chs, 64);
		FILE* file_exist=NULL;
		file_exist=fopen(DllName, "rb");
		if(file_exist==NULL){
			DownloadUpdate();
		}
		else{
			fclose(file_exist);
			std::string new_checksum="";
			BYTE* MD5_sum=MD5_from_file(DllName);
			char hex[3];
			for(int zxcvb=0; zxcvb<16; zxcvb++){
				sprintf(hex, "%02x", MD5_sum[zxcvb]);
				new_checksum+=hex;
			} 
			if(checksum.compare(new_checksum.c_str())!=0){
				DownloadUpdate();
			}
			new_checksum.clear();
		}
	}
	else zCParser::GetParser()->Error(zSTRING("Couldn't connect with update server! Check if your internet is working!"), 0);
};

CUpdate::~CUpdate() // Destructor
{
	ZeroMemory(buffer, 39);
};

void CUpdate::DownloadUpdate()
{
	zCView::GetScreen()->PrintCXY(DownloadingUpdate);
	if(URLDownloadToFile(NULL, "yoursite-invalid------------.com/GMP.dll", ".\\System\\GMPNew.dll", 0, NULL)==S_OK){
		if(!CreateProc(".\\System\\Update.exe")){
			remove(".\\System\\GMPNew.dll");
			zCParser::GetParser()->Error(zSTRING("Autoupdate failed. Couldn't launch Update.exe. Download update at your-site.com"), 0);
		}
	}
	else zCParser::GetParser()->Error(zSTRING("Autoupdate failed. Download update at your-site.com"), 0);
};

BYTE* CUpdate::MD5_from_file(const char *file){
	long file_size;
	unsigned char *file_buffer;
	FILE *hFile=fopen(file, "rb");
	if(hFile==NULL) return NULL;
	fseek(hFile, 0, SEEK_END);
	file_size=ftell(hFile);
	fseek(hFile, 0, SEEK_SET);
	file_buffer=new unsigned char[file_size];
	fread(file_buffer, file_size, 1, hFile);
	fclose(hFile);

	HCRYPTPROV hCryptProv;
	HCRYPTHASH hHash;
	BYTE *pData=NULL;
	DWORD dwHashLength;
	DWORD dwLength;

	if(CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)){
		if(CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash)){
			if(CryptHashData(hHash, file_buffer, file_size, 0)){
				CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&dwHashLength, &dwLength, 0);
				pData = new BYTE[dwHashLength];
				CryptGetHashParam(hHash, HP_HASHVAL, pData, &dwHashLength, 0);
				CryptDestroyHash(hHash);
				CryptReleaseContext(hCryptProv, 0);
				delete file_buffer;
				return pData;
			}
		}
	}
	return NULL;
}

bool CUpdate::CreateProc(std::string CmdLine)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	
	BOOL ret = CreateProcessA( 
		NULL,   // No module name (use command line)
		(LPSTR)CmdLine.c_str(),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi );           // Pointer to PROCESS_INFORMATION structure

	if (ret != NULL) 
	{

		hThread = pi.hThread;
		hProcess = pi.hProcess;
		return true;
	}
	else return false;

}