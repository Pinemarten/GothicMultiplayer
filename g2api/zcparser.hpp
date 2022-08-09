/*////////////////////////////////////////////////////////////////////////////

This file is part of the G2Ext SDK headers.

//////////////////////////////////////////////////////////////////////////////

The G2Ext SDK headers

Copyright © 2009, 2010 by Paindevs and Patrick Vogel

All Rights reserved.

THE WORK (AS DEFINED BELOW) IS PROVIDED
UNDER THE TERMS OF THIS CREATIVE COMMONS
PUBLIC LICENSE ("CCPL" OR "LICENSE").
THE WORK IS PROTECTED BY COPYRIGHT AND/OR
OTHER APPLICABLE LAW. ANY USE OF THE WORK
OTHER THAN AS AUTHORIZED UNDER THIS LICENSE
OR COPYRIGHT LAW IS PROHIBITED.

BY EXERCISING ANY RIGHTS TO THE WORK PROVIDED
HERE, YOU ACCEPT AND AGREE TO BE BOUND BY THE
TERMS OF THIS LICENSE. TO THE EXTENT THIS
LICENSE MAY BE CONSIDERED TO BE A CONTRACT,
THE LICENSOR GRANTS YOU THE RIGHTS CONTAINED
HERE IN CONSIDERATION OF YOUR ACCEPTANCE OF
SUCH TERMS AND CONDITIONS.

Full license at http://creativecommons.org/licenses/by-nc/3.0/legalcode

// Modified by Gothic Multiplayer Team (pampi, skejt23, mecio)

/////////////////////////////////////////////////////////////////////////////*/

#pragma once
#include "ztypes.hpp"
#include "macros.hpp"
#include "zcpar_symbol.hpp"

#pragma warning(disable:4731) // -- suppress C4731:"frame pointer register 'ebp' modified by inline assembly code"

class zCPar_File;
class zCPar_TreeNode;
class zCViewProgressBar;
class zCView;

typedef struct 
{
	zCPar_Symbol*	preAllocatedSymbols;
	int				nextPreAllocated;
} zCPar_SymbolTable;

typedef enum  
{
	zCPVOID, 
	zCPFLOAT, 
	zCPINT, 
	zCPSTRING, 
	zCPCLASS, 
	zCPFUNC,
	zCPPROTOTYPE, 
	zCPINSTANCE
} zCParser_CallType;

#define zMAX_SYM_DATASTACK 2048

class zCParser
{
private:
	void (__cdecl * msgfunc)( zSTRING );

	zCArray<zCPar_File*>	file;
	zCPar_SymbolTable		symtab;
	zCArray<zCPar_Symbol*>  table;
	zCArraySort<int>		tablesort;

	zCPar_Symbol*			lastsym;
	zCPar_Symbol*			firstsym; 

	//zCPar_StringTable stringtab;
	zCArray<zSTRING*>		stringtab;

	//zCPar_Stack			   stack;
	int stack_stack;							//0x0048 zBYTE*  // Stackbegin
	int stack_stackptr;						 //0x004C zBYTE* or zWORD* or int*
	int stack_stacklast;						//0x0050 zBYTE* or zWORD* or int*
	int stack_stacksize;						//0x0054 int //Stack size in Bytes

	//zCPar_DataStack		   datastack;
	int stack[zMAX_SYM_DATASTACK];				//0x0058
	int sptr;								   //0x085C int

	zCPar_Symbol* _self;

	zSTRING fname;
	zSTRING mainfile;
	int compiled;		  
	int treesave;			   
	int datsave;			  
	int parse_changed;
	int treeload;
	int mergemode;			 
	int linkingnr;
	int linec;
	int line_start;
	int ext_parse;
	char* pc_start;
	char* pc;
	char* oldpc;
	char* pc_stop;
	char* oldpc_stop;
	int params;
	int in_funcnr;
	int in_classnr;
	int stringcount;
	zCPar_Symbol* in_func;
	zCPar_Symbol* in_class;
	int error;
	int stop_on_error;
	int errorline;
	char* prevword_index[16];
	int prevline_index[16];
	int prevword_nr;
	zSTRING  aword;	 
	int _instance;		   
	int instance_help;
	zCViewProgressBar* progressBar;
	zCPar_TreeNode* tree;
	zCPar_TreeNode* treenode;
	zCView* win_code;
	int debugmode;
	int curfuncnr;
	int labelcount;
	int* labelpos;

	//zCList <zSTRING>*	 add_funclist;
	zCList<zSTRING*> add_funclist;
	int add_funclist_data;							//zString*
	int add_funclist_next;							//zCList<zSTRING>*

	zSTRING add_filename;							//zSTRING						  
	int add_created;								//zBOOL
public:
	zCParser(int pA = 100){XCALL(0x0078DED0);};
	void CreatePCode(void){XCALL(0x007900E0);};

	//.text:007929F0 ; int __cdecl zCParser__CallFunc(int, ...)
	/** Insert description. 
	* @return	A *pointer* to the returnvalue (int/float)
	*/
	void* __cdecl CallFunc(zCParser* pA, int FuncIndex,  ...){XCALL(0x007929F0);};

	//.text:007929D0 ; public: void * __thiscall zCParser::CallFunc(class zSTRING const &)
	/** Insert description. 
	* @return	A *pointer* to the returnvalue (int/float)
	*/
	void* CallFunc(const zSTRING & pA){XCALL(0x007929D0);};
	int CheckClassSize(int pA, int pB){XCALL(0x00794470);};
	int CheckClassSize(zSTRING & pA, int pB){XCALL(0x00794450);};
	int CreateInstance(int pA, void * pB){XCALL(0x00792FA0);};
	int CreateInstance(zSTRING & pA, void * pB){XCALL(0x00792F20);};
	int DefineExternal(const zSTRING & a_SymbolName, zBOOL(__cdecl*a_ExternalProc)(), int a_ReturnType, int a_ParameterType, ...)
	{XCALL(0x007A0190);};
	int DefineExternalVar(const zSTRING & a_SymbolName, void* a_ExternalVar, int a_ReturnType, int a_ParameterType)
	{XCALL(0x007A0550);};
	zBOOL Error(void){XCALL(0x0078E730);};
	int GetBaseClass(int instance){XCALL(0x00793610);};
	void GetParameter(float & pA){XCALL(0x007A0770);};
	void GetParameter(int & pA){XCALL(0x007A0760);};
	void GetParameter(zSTRING & pA){XCALL(0x007A07B0);};
	void GetParameter(const zSTRING & pA){XCALL(0x00793470);};
	/** Gets an instance from Daedalus.
	* @usable In an external only
	* @param ClassId	return value of GetIndex(zSTRING& class)
	* @param Index		instance number (index of array only containing instances of this class)
	* @return			Daedalus instance index (you get this one when you pass an Instance as an Integer parameter)
	*/
	int GetInstance(int ClassId, int Index){XCALL(0x00793730);};
	/** Gets an instance from the Daedalus stack.
	 * @usable In an external only
	 * @return	A pointer to the object instance.
	 */
	void* GetInstance(void){XCALL(0x007A08F0);};
	/** Gets an instance from the Daedalus stack.
	* @usable in an external only
	*/
	void* GetInstanceAndIndex(int & pA){XCALL(0x007A0920);};
	zSTRING GetInstanceValue(int pA, int pB, void * pC, int pD){XCALL(0x00793FD0);};
	zSTRING GetInstanceValue(class zSTRING & pA, int pB, void * pC, int pD){XCALL(0x00793EE0);};
	int GetIndex(class zSTRING const & pA){XCALL(0x00793470);};
	zCPar_Symbol* GetSymbol(const zSTRING& pA){XCALL(0x007938D0);};
	zCPar_Symbol* GetSymbol(int pA){XCALL(0x007938C0);};
	zSTRING GetSymbolInfo(int pA, int & pB, int & pC){XCALL(0x007937F0);};
	static zBYTE GetVersion(void){XCALL(0x0078D010);};
	void Message(zSTRING & Message){XCALL(0x0078E5F0);};
	void Error(zSTRING & Message, int){XCALL(0x0078E270);};

	//.text:0078E900 ; public: int __thiscall zCParser::LoadDat(class zSTRING &)
	/** Loads a compiled *.dat file
	* @param  File name
	*/
	int LoadDat(zSTRING & zsFileName){XCALL(0x0078E900);};
	int MergeFile(zSTRING & zsFileName){XCALL(0x00791650);};
	/** Parses a *.d or a *.src file
	* @param  File name
	*/
	int Parse(zSTRING zsFileName){XCALL(0x0078EBA0);};
	/** Parses a *.d file
	* @param  File name
	*/
	int ParseFile(zSTRING & zsFileName){XCALL(0x0078F660);};
	/** Parses a *.src file
	* @param  File name
	*/
	int ParseSource(zSTRING & zsFileName){XCALL(0x0078EE20);};
	void Reparse(zSTRING & pA){XCALL(0x00794C30);};
	void Reset(void){XCALL(0x00793100);};
	void ResetGlobalVars(void){XCALL(0x007969C0);};
	/** Saves a compiled *.dat file
	* @param  File name
	*/
	int SaveDat(zSTRING & zsFileName){XCALL(0x0078E740);};
	void SetInstance(const zSTRING & pA, void*){XCALL(0x00794870);};
	void SetMessageFunc(void(__cdecl *f)(zSTRING)){XCALL(0x0078E250);};
	void SetReturn(float pA){XCALL(0x007A0980);};
	void SetReturn(int pA){XCALL(0x007A0960);};
	void SetReturn(void * pA){XCALL(0x007A09A0);};
	void SetReturn(zSTRING & pA){XCALL(0x007A09A0);};
	void SetStopOnError(zBOOL pA){XCALL(0x0078E260);};
	/** This method returns the current zCParser instance*/
	static zCParser* GetParser()
	{
		return (zCParser*)0x00AB40C0;
	};
};


