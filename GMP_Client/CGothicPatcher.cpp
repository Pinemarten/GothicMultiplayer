
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

#include "common.h"
#include <cstdio>
#include "patch.h"
#include "InjectMage.h"
#include "mod.h"
#include "GMPSplash.h"


void Patch::oCNpc::DropItem()
{
	CreateHook(0x00744DD0, (DWORD)OnDropItem, 1, true);
}

void Patch::NoStartupLogo()
{
	BYTE buffer[] = {0xC2, 0x04, 0x00, 0x90, 0x90};
	WriteMemory(0x426660, buffer, 5);
	
}

void Patch::SetVersion(const char * ver)
{
	memcpy((void*)0x89DA98, ver, 11);
}
void Patch::SetWndName(const char * name)
{
	memcpy((void*)0x89DAA4, name, 23);
}
void Patch::SetStatusScreenEnabled(bool isit){
	if(!isit) EraseMemory(0x6FC270,0x90, 11);
	else{
		BYTE buffer[] = {0x8B, 0x0D, 0x84, 0x26, 0xAB, 0x00, 0xE8, 0x05, 0x17, 0x04, 0x00};
		WriteMemory(0x6FC270, buffer, 11);
	}
}
void Patch::SetOpenInventoryEnabled(bool isit){
	if(isit){
		BYTE buffer[] = {0x8B, 0x0D, 0x84, 0x26, 0xAB, 0x00, 0x53, 0xE8, 0x74, 0x5C, 0x06, 0x00};
		WriteMemory(0x6FC5D0, buffer, 12);
	} else	EraseMemory(0x6FC5D0, 0x90, 12);
}
void CGothicPatcher::SetLogScreenEnabled(bool isit){
	if(isit){
		BYTE buffer[] = {0x8B, 0x0D, 0x84, 0x26, 0xAB, 0x00, 0xE8, 0x0C, 0x13, 0x04, 0x00};
		WriteMemory(0x6FC2A9, buffer, 11);
	} else EraseMemory(0x6FC2A9, 0x90, 11);
}
void CGothicPatcher::SetFirstPersonCameraEnabled(bool isit){
	if(isit){
		BYTE buffer[] = {0xE8, 0xB6, 0xEE, 0xFF, 0xFF};
		WriteMemory(0x4A5225, buffer, 5);
	} else EraseMemory(0x4A5225, 0x90, 5);
}

void CGothicPatcher::DisablePause(){
	BYTE buffer[]={0xC2,0x04,0x00};
	WriteMemory(0x6C8AD0, buffer, 3);
}

void CGothicPatcher::CanNotUseMessageEnabled(bool isit){
	if(isit){
		BYTE buffer[] = {0xE8, 0x52, 0x0D, 0x06, 0x00};
		WriteMemory(0x731C99, buffer, 5);
	} else 
	{
		BYTE buffer[] = {0x90, 0x90, 0x90, 0x90, 0x90};
		WriteMemory(0x731C99, buffer, 5);
	}
}

void CGothicPatcher::DisableAbnormalExit(){
	BYTE buffer[]={0};
	WriteMemory(0x470AEA,buffer,1);
}

void CGothicPatcher::DisableGothicMainMenu(){
	BYTE buffer[]={0xC2, 0x04, 0};
	WriteMemory(0x4292D0, buffer, 3);
}

void CGothicPatcher::DisablePlayBink(){
	BYTE buffer[]={0xC2, 0x14, 0};
	WriteMemory(0x42B940, buffer, 3);
}

void CGothicPatcher::ChangeLevelEnabled(bool isit){
	if(isit){
		BYTE buffer[] = {0x6A, 0xFF, 0x68};
		WriteMemory(0x6C7290, buffer, 3);
	} else 
	{
		BYTE buffer[] = {0xC2, 0x08, 0x00};
		WriteMemory(0x6C7290, buffer, 3);
	}
}

void CGothicPatcher::ErrorInCheckClassSizeEnabled(bool isit){
	if(!isit)EraseMemory(0x7946D1,0x90, 12);
	else {
		BYTE buffer[]={0x6A, 0xFF, 0x8D, 0x44, 0x24, 0x2C, 0x50, 0xE8, 0x93, 0x9B, 0xFF, 0xFF};
		WriteMemory(0x7946D1, buffer, 12);
	}
}

void CGothicPatcher::InitNewSplash()
{
	JmpPatch(0x426660, (DWORD)SplashFunc);
}
void CGothicPatcher::DisableStartupScript()
{
	EraseMemory(0x6C9FD2,0x90, 7);
}

void CGothicPatcher::DisableCheat(){
	const static char *GUARDIAN_0="If you cheat, you won't get a piece of cake.\0";
	const static char *GUARDIAN_1="We do what we must, because we can.\0";
	const static char *GUARDIAN_2="For the good of all of us, except for the ones who are dead.\0";
	const static char *GUARDIAN_3="But there's no sense crying over every mistake.\0";
	const static char *GUARDIAN_4="You just keep on trying till you run out of cake.\0";
	const static char *GUARDIAN_5="and the science gets done and you make a neat gun.\0";
	const static char *GUARDIAN_6="For the people who are still alive!\0";
	memcpy((void*)0x8915B0, "Can't unleash force\0", 20);	//string wyœwietlany przy wpisaniu Marvina(musi byæ mniejszy lub równy 23)
	WriteMemory(0x434498, (PBYTE)"\0", 1);	//blokada testmode
	EraseMemory(0x433247,0x90, 5);	//blokada "guardiana"
	EraseMemory(0x433DB2,0x90, 10); //blokada "southpark"
	EraseMemory(0x433DFB,0x90, 10); //blokada "Clerks"
	//OFFTOP
	//Guardian - "Still Alive"
	float fTime=5000.0f;
	WriteMemory(0x433274, (PBYTE)&fTime, 4);
	WriteMemory(0x433260, (PBYTE)&GUARDIAN_0, 4);
	fTime=8000.0f;
	WriteMemory(0x4332C4, (PBYTE)&fTime, 4);
	WriteMemory(0x4332B0, (PBYTE)&GUARDIAN_1, 4);
	fTime=11000.0f;
	WriteMemory(0x433314, (PBYTE)&fTime, 4);
	WriteMemory(0x433300, (PBYTE)&GUARDIAN_2, 4);
	fTime=14000.0f;
	WriteMemory(0x433364, (PBYTE)&fTime, 4);
	WriteMemory(0x433350, (PBYTE)&GUARDIAN_3, 4);
	fTime=17000.0f;
	WriteMemory(0x4333B4, (PBYTE)&fTime, 4);
	WriteMemory(0x4333A0, (PBYTE)&GUARDIAN_4, 4);
	fTime=20000.0f;
	WriteMemory(0x433404, (PBYTE)&fTime, 4);
	WriteMemory(0x4333F0, (PBYTE)&GUARDIAN_5, 4);
	fTime=23000.0f;
	WriteMemory(0x433454, (PBYTE)&fTime, 4);
	WriteMemory(0x433440, (PBYTE)&GUARDIAN_6, 4);
	//END OF OFFTOP
}
