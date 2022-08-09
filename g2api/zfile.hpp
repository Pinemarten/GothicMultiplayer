
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
// 8th of may 2011, skejt23 

#ifndef __API_G2_ZFILE_H__
#define __API_G2_ZFILE_H__

#include "zcbuffer.hpp"

class zFILE
{
public:
	zFILE(const zSTRING & pA){XCALL(0x00442F30);};
	zFILE(){XCALL(0x00442EF0);};
	~zFILE(){XCALL(0x00442F50);};
	static zCBuffer* GetFreeBuffer(void){XCALL(0x00447F40);};
	static void DirectFileAccess(bool){XCALL(0x00442530);};
	static void DirectFileAccess(bool, zSTRING & pA){XCALL(0x00442760);};
};

class zFILE_FILE : public zFILE
{
public:
	zFILE_FILE(const zSTRING & pA){XCALL(0x00443450);};
	zFILE_FILE(){XCALL(0x00442FD0);};
	void Init(const zSTRING & pA){XCALL(0x00442F60);};
	int Opene(bool){XCALL(0x00443C90);};
	int Open(const zSTRING & pA, bool){XCALL(0x00443360);};
	int Close(void){XCALL(0x00444010);}
	bool IsOpened(void){XCALL(0x004432F0);};
	bool Exists(void){XCALL(0x004436B0);};
	bool Exists(const zSTRING & pA){XCALL(0x004433D0);};
	zSTRING GetPath(void){XCALL(0x00443160);};
	zSTRING GetFullPath(void){XCALL(0x00446360);};
	zSTRING GetExt(void){XCALL(0x0043EDA0);};
	zSTRING GetFilename(void){XCALL(0x004432A0);};
	zSTRING GetFile(void){XCALL(0x0043ECF0);};
	struct _iobuf* GetFileHandle(void){XCALL(0x00443150);};
	long Size(void){XCALL(0x004438A0);};
};

class zFILE_VDFS
{
public:
	zFILE_VDFS(const zSTRING & pA){XCALL(0x00448E20);};
	int Close(void){XCALL(0x004493A0);};
	int Open(bool){XCALL(0x00449120);};
	bool Exists(void){XCALL(0x00449020);};
	int Seek(long a){XCALL(0x00449490);};
	zSTRING SeekText(zSTRING const &text){XCALL(0x00449870);};
	long Size(void){XCALL(0x00449410);};
	bool Eof(void){XCALL(0x00449470);};
	int Read(void* buffer, size_t size){XCALL(0x0044ABF0);};
};

#endif