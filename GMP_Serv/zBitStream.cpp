
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

//zBitStream - klasa przechowuj¹ca strumieñ bitów(przystowana pod k¹tem pakietów, wiêc max rozmiar to 1280B)
//autorstwa pampiego
//12.2011
#include "zBitStream.h"
#include "CCompression.h"
#include <cstring>
#ifndef BYTE
#define BYTE unsigned char
#endif

zBitStream::zBitStream(){
	Clear();
}

zBitStream::~zBitStream(){
	memset(data, 0, MAX_BS_SIZE);
}

void zBitStream::Compress(){
	unsigned char temp[MAX_BS_SIZE];
	memcpy(temp, data, (size&0x0007)?static_cast<unsigned int>((size>>3)+1):size>>3);
	unsigned int outl=0;
	CCompression::GetInstance()->Compress(temp, data, (size&0x0007)?static_cast<unsigned int>((size>>3)+1):size>>3, outl);
	size=static_cast<short>(outl<<3);
}
void zBitStream::Clear(){
	WHOCARE=0;
	err=0;
	memset(data, 0, MAX_BS_SIZE);
}
void zBitStream::Decompress(){
	unsigned char temp[MAX_BS_SIZE];
	memcpy(temp, data, size>>3);
	unsigned int outl=0;
	CCompression::GetInstance()->Decompress(temp, data, size>>3, outl);
	size=static_cast<short>(outl<<3);
}
unsigned int zBitStream::GetData(void*& ptr){
	ptr=(void*)data;
	return static_cast<unsigned int>((size&0x0007)?(size>>3)+1:size>>3);
}
bool zBitStream::ReadBit(){
	bool ret_val= (data[it/8]&(unsigned char)(1<<(7-(int)(it&0x0007))))?true:false;
	it++;
	return ret_val;
}
void zBitStream::Rewind(){it=0;}
void zBitStream::WriteBit(bool d){
	if(d) data[it>>3]|= 1<< (int)(7-static_cast<int>(it&0x0007));
	it++;
	size++;
}
void zBitStream::operator<< (const char* d){
	if(!d) return;
	char *ptr=(char*)d;
	do{
		wb((BYTE)(*ptr));
	} while((*ptr++));
}
void zBitStream::operator<< (BYTE d){wb(d);}
void zBitStream::operator<< (unsigned short d){wb((BYTE)d);wb((BYTE)(d>>8));}
void zBitStream::operator<< (unsigned int d){wb((BYTE)d);wb((BYTE)(d>>8));wb((BYTE)(d>>16));wb((BYTE)(d>>24));}
void zBitStream::operator<< (unsigned long d){wb((BYTE)d);wb((BYTE)(d>>8));wb((BYTE)(d>>16));wb((BYTE)(d>>24));}
void zBitStream::operator<< (unsigned long long d){wb((BYTE)d);wb((BYTE)(d>>8));wb((BYTE)(d>>16));wb((BYTE)(d>>24));wb((BYTE)(d>>32));wb((BYTE)(d>>40));wb((BYTE)(d>>48));wb((BYTE)(d>>56));}	//dobrze ¿e nie ma typu 128bitowego
void zBitStream::operator<< (char d){wb((unsigned char)d);}
void zBitStream::operator<< (short d){wb((BYTE)d);wb((BYTE)(d>>8));}
void zBitStream::operator<< (int d){wb((BYTE)d);wb((BYTE)(d>>8));wb((BYTE)(d>>16));wb((BYTE)(d>>24));}
void zBitStream::operator<< (long d){wb((BYTE)d);wb((BYTE)(d>>8));wb((BYTE)(d>>16));wb((BYTE)(d>>24));}
void zBitStream::operator<< (long long d){wb((BYTE)d);wb((BYTE)(d>>8));wb((BYTE)(d>>16));wb((BYTE)(d>>24));wb((BYTE)(d>>32));wb((BYTE)(d>>40));wb((BYTE)(d>>48));wb((BYTE)(d>>56));}
void zBitStream::operator<< (bool d){ WriteBit(d); }
void zBitStream::operator<< (float d){//floaty s¹ jakieœ inne...
	int *ptr=(int*)&d;
	wb((BYTE)(*ptr));
	wb((BYTE)((*ptr)>>8));
	wb((BYTE)((*ptr)>>16));
	wb((BYTE)((*ptr)>>24));
}
void zBitStream::operator>> (BYTE& d){d=rb();}
void zBitStream::operator>> (unsigned short& d){d=rb(); d|= rb()<<8;}
void zBitStream::operator>> (unsigned int& d){d=rb(); d|= rb()<<8; d|= rb()<<16; d|= rb()<<24;}
void zBitStream::operator>> (unsigned long& d){d=rb(); d|= rb()<<8; d|= rb()<<16; d|= rb()<<24;}
void zBitStream::operator>> (unsigned long long& d){
	BYTE *ptr=(BYTE*)&d;
	for(int i=0; i<8; i++) ptr[i]=rb();
}
void zBitStream::operator>> (char& d){d=rb();}
void zBitStream::operator>> (short& d){d=rb(); d|= rb()<<8;}
void zBitStream::operator>> (int& d){d=rb(); d|= rb()<<8; d|= rb()<<16; d|= rb()<<24;}
void zBitStream::operator>> (long& d){d=rb(); d|= rb()<<8; d|= rb()<<16; d|= rb()<<24;}
void zBitStream::operator>> (long long& d){
	BYTE *ptr=(BYTE*)&d;
	for(int i=0; i<8; i++) ptr[i]=rb();
}
void zBitStream::operator>> (bool& d){ d=ReadBit();}
void zBitStream::operator>> (float& d){
	BYTE *ptr=(BYTE*)&d;
	ptr[0]=rb();
	ptr[1]=rb();
	ptr[2]=rb();
	ptr[3]=rb();
}
void zBitStream::operator>> (std::string& d){
	char c;
	if(!d.empty()) d.clear();
	d="";
	do{
		c=(char)rb();
		d+=c;
	}while(c!=0);
}

void zBitStream::wb(BYTE d){
	if(it&0x0007){
		data[it>>3]|= d>>static_cast<int>(it&0x0007);
		it+=8;
		data[it>>3]|=d<< (int)(8-static_cast<int>(it&0x0007));
	} else data[it>>3]=(BYTE)d;
	it+=8;
	size+=8;
}
BYTE zBitStream::rb(){
	BYTE ret_val;
	if(it+8>size){ err=1; return 0xFF; }
	if(it&0x0007){
		ret_val=0;
		ret_val|=data[it>>3] << (int)(8-static_cast<int>(it&0x0007));
		it+=8;
		ret_val|=data[it>>3] >> (int)(it&0x0007);
	} else ret_val=data[it>>3];
	it+=8;
	return ret_val;
}