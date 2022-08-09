
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

/*#include <stdio.h>
#include "CSpawnList.h"

CSpawnList::CSpawnList(){
	//load all
	FILE *fh=fopen("spawnpoint", "rb");
	fseek(fh, 0, SEEK_END);
	this->file_size=ftell(fh);
	rewind(fh);
	this->file_buffer_size=this->file_size/1000;
	if(this->file_size%1000) this->file_buffer_size++;
	this->file_buffer_size*=1000;
	this->file_buffer=new char[this->file_buffer_size];
	fread(this->file_buffer, this->file_size, 1, fh);
	fclose(fh);
}

CSpawnList::~CSpawnList(){
	//free all
	delete this->file_buffer;
}

char *CSpawnList::GetFilePart(unsigned long part){
	if(part<static_cast<unsigned long>(this->file_buffer_size/1000)){
		char *ret_val=this->file_buffer+(part*1000);
		return ret_val;
	}
	return NULL;
}

long CSpawnList::GetBufferSize(){return this->file_buffer_size;}
long CSpawnList::GetFileLength(){return this->file_size;}
*/
#include "CSpawnList.h"
CSpawnList::CSpawnList(){
	Load("spawnpoint");
}