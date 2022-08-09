/*
LIBRARY InjectMage
AUTHOR Mecio
CopyRight 2010-2011
All Rights Reserved
*/

#ifndef IMPORT
#define IMPORT __declspec(dllimport)
#endif

#ifndef INJECTMAGE_INCLUDED
#define INJECTMAGE_INCLUDED

typedef unsigned char BYTE;
typedef BYTE * PBYTE;
typedef unsigned long DWORD;
typedef void* LPVOID;


struct sRegs
{
	int EDI,
		ESI,
		EBP,
		ESP,
		EBX,
		EDX,
		ECX,
		EAX;
};


	void IMPORT WriteMemory(DWORD dwAddress, PBYTE pbBytes, DWORD dwLength);
	void IMPORT EraseMemory(DWORD dwAddress, BYTE bValue, DWORD dwLength);
	void IMPORT JmpPatch(DWORD dwAddress, DWORD dwDest);
	void IMPORT CallPatch(DWORD dwAddress, DWORD dwDest, int nNops);
	bool IMPORT CreateHook(DWORD destination, DWORD callback, BYTE ParamCount, bool Registers);

#endif

	
