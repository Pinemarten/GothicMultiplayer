
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
// 15th of may 2011, skejt23 

#ifndef __API_G2_ZCPARTICLEFX_H__
#define __API_G2_ZCPARTICLEFX_H__

#include "zcvisual.hpp"

class zCParticleFXEmitter
{
public:
	void SetLooping(int isit)
	{
		*(int*)((DWORD)this+0x18) = isit;
	};
	int IsLooping()
	{
		return *(int*)((DWORD)this+0x18);
	};
};

class zCParticleFX : public zCVisual
{
public:
	zSTRING GetVisualName(void){XCALL(0x005ADD30);};
	void SetVisualUsedBy(zCVob*){XCALL(0x005ADC80);};
	zCVisual* LoadVisualVirtual(const zSTRING& file){XCALL(0x005ADC00);};
	static zCParticleFX* Load(const zSTRING& file){XCALL(0x005AEDF0);};
	zCParticleFXEmitter* GetEmitter()
	{
		return *(zCParticleFXEmitter**)((DWORD)this+0x54);
	};
	void Reset(void){XCALL(0x005B31F0);};
};

#endif