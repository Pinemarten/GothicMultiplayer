
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
// skejt23, 5th of June, 2011

class zCTexture;

class zCFont
{
public:
	zCTexture* GetFontTexture(void){XCALL(0x0078A3F0)};
	zSTRING GetFontName(void){XCALL(0x007894A0);};
	int GetFontY(void){XCALL(0x007894E0);};
	int GetFontX(const zSTRING&){XCALL(0x007894F0);};
	int GetWidth(char){XCALL(0x00789520);};
	int GetSizeX(void){XCALL(0x007895E0);};
	void SetFontY(int sizey){*(int*)((DWORD)this+0x14) = sizey;};
	int GetSizeY(void){XCALL(0x00789610);};
};

class zCFontMan
{
public:
	zCFont* GetFont(int){XCALL(0x007884B0)};
	int Load(const zSTRING& font){XCALL(0x007882D0);};
	inline static zCFontMan* GetFontManager(void) { return *(zCFontMan**)0x00AB39D4; };
};