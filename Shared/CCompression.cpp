
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

#include <cstring>
#include <cstdio>
#include "CCompression.h"

CCompression::CCompression(){
	ratio=9;
	data_type=DT_BINARY;
}

CCompression::~CCompression(){
}

void CCompression::ChangeType(unsigned char type){
	if(type>=DT_DEFAULT) data_type=DT_BINARY;
	else data_type=type;
}

UINT32 CCompression::Compress(unsigned char *src, unsigned char *dst, UINT32 src_len, UINT32 &dst_len, BYTE method){
	switch(method){
		default:
		case DEFLATE:
				strm.zfree=Z_NULL;
				strm.zalloc=Z_NULL;
				strm.opaque=Z_NULL;
				strm.next_in=Z_NULL;
				if(deflateInit(&strm, ratio)!=Z_OK) printf("WTF?\n");

				dst_len=0;
				strm.next_in=src;
				strm.next_out=dst;
				strm.avail_out=1<<10;
				strm.avail_in=src_len;
				deflate(&strm, Z_FINISH);
				deflateEnd(&strm);
				dst_len=static_cast<UINT32>(1<<10) - strm.avail_out;
				return (UINT32)(static_cast<UINT32>(1<<10) - strm.avail_out);
			break;
	}
	return 0;	//should never happen
}

UINT32 CCompression::Decompress(BYTE *src, BYTE *dst, UINT32 src_len, UINT32 &dst_len, BYTE method){
	switch(method){
		default:
		case DEFLATE:
			strm.zfree=Z_NULL;
			strm.zalloc=Z_NULL;
			strm.opaque=Z_NULL;
			strm.next_in=Z_NULL;
			strm.avail_in=0;
			if(inflateInit(&strm)!=Z_OK) printf("WTF?\n");

			dst_len=0;
			strm.avail_in=src_len;
			strm.next_in=src;
			strm.avail_out=1<<10;
			strm.next_out=dst;
			inflate(&strm, Z_NO_FLUSH);
			inflateEnd(&strm);
			dst_len=static_cast<UINT32>(1<<10) - strm.avail_out;
			return (UINT32)(static_cast<UINT32>(1<<10) - strm.avail_out);
		break;
	}
	return 0;	//should never happen
}