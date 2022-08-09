
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "CGenericFile.h"
#ifndef WIN32
//potrzebne include'y do pokedeksu
#include <sys/types.h>
#include <dirent.h>
#endif

void CGenericFile::Load(const char *file){
	this->file_buffer_size=0;
	this->file_size=0;
	this->file_buffer=NULL;
	char b[128];
#ifdef WIN32
	sprintf(b, "dir /B %s > temp\0", file);
	system(b);
	FILE *is_that_exist=fopen("temp", "r");
	if(!is_that_exist) return;
	memset(b, 0, 128);
	if(fscanf(is_that_exist, "%s", b)!=1){ fclose(is_that_exist); remove("temp"); return; }
	fclose(is_that_exist);
	remove("temp");
	if(strlen(b)<3) return;
#else
	memset(b, 0, 128);
	DIR *dir;
	struct dirent *entry;
	
	if ((dir = opendir("./")) == NULL) return;
	else {
		FILE *fh;
		while ((entry = readdir(dir)) != NULL){
		fh=fopen(entry->d_name, "r");
		//printf("%s type: %s\n", entry->d_name, (fh)?"file":"folder");
		if((strlen(entry->d_name)==strlen(file)) && (fh)){
			char upper[128];
			char of[128];
			memcpy(upper, entry->d_name, strlen(entry->d_name)+1);
			memcpy(of, file, strlen(file)+1);
			for(unsigned int i=0; i<strlen(file); i++){
			  if((upper[i]>='a') && (upper[i]<='z')) upper[i]-=('a'-'A');
			  if((of[i]>='a') && (of[i]<='z')) of[i]-=('a'-'A');
			}
			if(!memcmp(upper, of, strlen(file))){
				memcpy(b, entry->d_name, strlen(file)+1);
				fclose(fh);
				break;
			}
		}
		if(fh) fclose(fh);
		}
		closedir(dir);
	}
//stare i nie jare
/*	system("ls > temp");
	char upper[128];
	char of[128];
	bool match=false;
	FILE *is_that_exist=fopen("temp", "r");
	if(!is_that_exist) return;
	memset(b, 0, 128);
	memset(upper, 0, 128);
	memset(of, 0, 128);
	for(int i=0; i<strlen(file); i++) if((of[i]>='a') && ((of[i]<='z'))) of[i]-=('a'-'A');
	while((fscanf(is_that_exist, "%s", b)==1) && (!match)){
		if(strlen(file)==strlen(b)){
			memcpy(upper, b, strlen(b));
			for(int i=0; i<strlen(b); i++) if((upper[i]>='a') && ((upper[i]<='z'))) upper[i]-=('a'-'A');
			if(!memcmp(of, upper, strlen(b))) match=true;
		}
	}
	fclose(is_that_exist);
	if(!match) return;
	remove("temp");*/
#endif
	FILE *fh=fopen(b, "rb");
	if(!fh) return;
	fseek(fh, 0, SEEK_END);
	this->file_size=ftell(fh);
	rewind(fh);
	this->file_buffer_size=this->file_size/1000;
	if(this->file_size%1000) this->file_buffer_size++;
	this->file_buffer_size*=1000;
	this->file_buffer=new unsigned char[this->file_buffer_size];
	fread(this->file_buffer, this->file_size, 1, fh);
	fclose(fh);
}
CGenericFile::~CGenericFile(){ delete this->file_buffer; }
unsigned char *CGenericFile::GetFilePart(unsigned long part){
	if(part<static_cast<unsigned long>(this->file_buffer_size/1000)){
		unsigned char *ret_val=this->file_buffer+(part*1000);
		return ret_val;
	}
	return NULL;
}
long CGenericFile::GetFileLength(){return this->file_size;}
long CGenericFile::GetBufferSize(){return this->file_buffer_size;}