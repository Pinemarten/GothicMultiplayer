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

class zCTexture;
class zCFont;
class zCViewText {};

#define VIEW_VXMAX 8192
#define VIEW_VYMAX 8192

enum zTviewID 
{
	VIEW_SCREEN,
	VIEW_VIEWPORT,
	VIEW_ITEM
};	

enum zTViewFX
{
	VIEW_FX_NONE,
	VIEW_FX_ZOOM,
	VIEW_FX_MAX	
};

enum zTRnd_AlphaBlendFunc   
{   
	zRND_ALPHA_FUNC_MAT_DEFAULT,
	zRND_ALPHA_FUNC_NONE,		   
	zRND_ALPHA_FUNC_BLEND,		  
	zRND_ALPHA_FUNC_ADD,					
	zRND_ALPHA_FUNC_SUB,					
	zRND_ALPHA_FUNC_MUL,					
	zRND_ALPHA_FUNC_MUL2,				   
	zRND_ALPHA_FUNC_TEST,		   
	zRND_ALPHA_FUNC_BLEND_TEST	  
};

class zCViewBase
{
private:
	int _vtbl;
public:
	~zCViewBase(){XCALL(0x0068DF70);};
};

class zCView : public zCViewBase
{
private:
	zTViewFX				zEViewFX;
	int						m_bFillZ;
	zCView*					next;
	zTviewID				viewID;
	int						flags;
	int						intflags;
	int						ondesk;
	zTRnd_AlphaBlendFunc	alphafunc;
	zCOLOR					color; 
	int						alpha;
	zCList<zCView>		  childs;

	//zList <zCView>		childs;
	int compare;			//(*Compare)(zCView *ele1,zCView *ele2);
	int count;				//int
	int last;				//zCView*;
	int wurzel;				//zCView*

	zCView*					owner;
	zCTexture*				backTex;

	int						vposx; 
	int 					vposy; 
	int 					vsizex;
	int 					vsizey;

	int 					pposx; 
	int 					pposy; 
	int 					psizex;
	int 					psizey;

	zCFont* 				font;
	zCOLOR					fontColor;

	int 					px1;
	int 					py1;
	int 					px2;
	int 					py2;

	int 					winx;
	int 					winy;  

	zCList<zCViewText>	  textLines;

	int 					scrollMaxTime;
	int 					scrollTimer;

	zTViewFX 				fxOpen;
	zTViewFX 				fxClose;
	float 					timeDialog;
	float 					timeOpen;
	float 					timeClose;
	float 					speedOpen;   
	float 					speedClose;
	int 					isOpen;
	int 					isClosed;
	int 					continueOpen;
	int 					continueClose;
	int 					removeOnClose;
	int 					resizeOnOpen;
	int 					maxTextLength;
	zSTRING 				textMaxLength;	  
	zVEC2 					posCurrent_0;
	zVEC2 					posCurrent_1;			
	zVEC2 					posOpenClose_0;
	zVEC2 					posOpenClose_1;

	char					_data[256];
public:
	zCView(int pA, int pB, int pC, int pD, zTviewID pE = VIEW_ITEM){XCALL(0x007A5700);};
	zCView(){XCALL(0x007A5640);};
	virtual ~zCView(){XCALL(0x007A57C0);};
	void Close(void){XCALL(0x007A6E30);};
	void ClrPrintwin(void){XCALL(0x007AA140);};
	void Create(zSTRING& background, zTViewFX fx0, zTViewFX fx1, float unk0, int unk1){XCALL(0x007A6810);};
	void DrawItems(void){XCALL(0x007A6750);};
	void Dialog(int x, int y, zSTRING& text, float time, zCOLOR* color){XCALL(0x007A8D40);};
	void DialogCX(int y, zSTRING& text, float time, zCOLOR* color){XCALL(0x007A8E10);};
	void DialogCXY(zSTRING& text, float time, zCOLOR* color){XCALL(0x007A8F60);};
	void DialogCY(int x, zSTRING& text, float time, zCOLOR* color){XCALL(0x007A8E70);};
	void DialogMessage(zSTRING& headline, zSTRING& text, float time, zCOLOR* color){XCALL(0x007A8FB0);};
	void DialogMessageCXY(zSTRING& headline, zSTRING& text, float time, zCOLOR* color){XCALL(0x007A9240);};
	int FontSize(zSTRING & pA){XCALL(0x007A9A10);};
	int FontY(void){XCALL(0x007A99F0);};
	void Init(int pA, int pB, int pC, int pD, zTviewID pE = VIEW_ITEM){XCALL(0x007A5B20);};
	void Init(int pA, int pB, zSTRING & pD, zTviewID pE){XCALL(0x007A5B20);};
	void Init(void){XCALL(0x007A5990);};
	void InsertBack(zSTRING const & pA){XCALL(0x007A6130);};
	void InsertItem(void* Element, int StayOnTop = false){XCALL(0x007ABAD0);};
	zCOLOR GetColor(void){XCALL(0x007A6090);};
	void GetExtends(int & x, int & y, int & cx, int & cy){XCALL(0x007A7860);};
	void GetPixelExtends(float & x, float & y, float & cx, float & cy){XCALL(0x007A7910);};
	void GetPixelPos(int & x, int & y){XCALL(0x007A76C0);};
	void GetPixelSize(int & x, int & y){XCALL(0x007A7830);};
	void GetPos(int & x, int & y){XCALL(0x007A7660);};
	void GetScreenExtends(int & x, int & y, int & cx, int & cy){XCALL(0x007A78A0);};
	void GetSize(int & x, int & y){XCALL(0x007A7810);};
	void Line(int x, int y, int x1, int y1, const zCOLOR& col){XCALL(0x007ABF70);};
	void Move(int pA, int pB){XCALL(0x007A76E0);};
	void Open(void){XCALL(0x007A6C00);};
	void Print(zINT x, zINT y, const zSTRING & message){XCALL(0x007A9A40);};
	void PrintCX(int pA, const zSTRING & pB){XCALL(0x007A9E50);};
	void PrintCXY(zSTRING const & pA){XCALL(0x007AA030);};
	void PrintCY(int pA, const zSTRING & pB){XCALL(0x007A9F40);};
	void PrintMessage( const zSTRING & pA, const zSTRING & pB, float pC, zCOLOR & pD){XCALL(0x007A8140);};
	void PrintMessageCXY(const zSTRING & pA, const zSTRING & pB, float pC, zCOLOR & pD){XCALL(0x007A8450);};
	void PrintWin(const zSTRING & text){XCALL(0x007AA8D0)};
	void PrintTimed(int x, int y, const zSTRING & text, float time, zCOLOR* color){XCALL(0x007A7D20)};
	void PrintTimedCX(int y, const zSTRING & text, float time, zCOLOR* color){XCALL(0x007A7DB0)};
	void PrintTimedCY(int x, const zSTRING & text, float time, zCOLOR* color){XCALL(0x007A7F00)};
	void PrintTimedCXY(const zSTRING & text, float time, zCOLOR* color){XCALL(0x007A7FC0)};
	void RemoveItem(void* Element){XCALL(0x007ABD10);};
	void Render(void){XCALL(0x007AC210);};
	void SetAlphaBlendFunc(const zTRnd_AlphaBlendFunc & p1){XCALL(0x007A6070);};
	int __stdcall SetAutoScroll(float p1){XCALL(0x007A5F20);};
	void SetColor(const zCOLOR & color){XCALL(0x007A6080);};
	void SetFont(const zSTRING & p1){XCALL(0x007A9920);};
	void SetFont(zCFont*){XCALL(0x007A98F0);};
	void SetFontColor(const zCOLOR & p1){XCALL(0x007A9910);};
	void SetFlags(int p1){XCALL(0x007A60A0);};
	void SetPos(int pA, int pB){XCALL(0x007A75B0);};
	void SetSize(int pA, int pB){XCALL(0x007A77A0);};
	void SetTransparency(int pA){XCALL(0x007A6060);};
	void Top(void){XCALL(0x007A6790);};
	int anx(int pA){XCALL(0x007A5E80);};
	int any(int pA){XCALL(0x007A5EC0);};
	int nax(int pA){XCALL(0x007A5E00);};
	int nay(int pA){XCALL(0x007A5E40);};
	zCView* GetPrintScreen(void){return *(zCView**)((DWORD)this+0x34);};
	inline static zCView* GetScreen() { return *(zCView**)0x00AB6468; };
};

class zCViewProgressBar
{
private:
	char m_cData[8];
	int  m_nProgress;
	char m_cData2[24];
public:
	void SetPercent(int pA, zSTRING pB = zSTRING("")){XCALL(0x0046EEC0);};
	void HandleChange(void){XCALL(0x0046F190);};
};
