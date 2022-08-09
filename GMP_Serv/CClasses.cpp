
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CClasses.h"

const char *NONE="none";
const char *TAG_CHAR="<character>";
const char *ETAG_CHAR="</character>";
const char *TAG_CLASS="<class>";
const char *ETAG_CLASS="</class>";
const char *TAG_NAME="<name>";
const char *ETAG_NAME="</name>";
const char *TAG_DESCRIPTION="<description>";
const char *ETAG_DESCRIPTION="</description>";
const char *TAG_TEAM="<team>";
const char *ETAG_TEAM="</team>";
const char *TAG_STR="<strength>";
const char *ETAG_STR="</strength>";
const char *TAG_DEX="<dexterity>";
const char *ETAG_DEX="</dexterity>";
const char *TAG_MP="<mana>";
const char *ETAG_MP="</mana>";
const char *TAG_HP="<health>";
const char *ETAG_HP="</health>";
const char *TAG_1H="<1hweapon>";
const char *ETAG_1H="</1hweapon>";
const char *TAG_2H="<2hweapon>";
const char *ETAG_2H="</2hweapon>";
const char *TAG_BOW="<bow>";
const char *ETAG_BOW="</bow>";
const char *TAG_XBOW="<crossbow>";
const char *ETAG_XBOW="</crossbow>";
const char *TAG_MAGIC="<magic_lvl>";
const char *ETAG_MAGIC="</magic_lvl>";
const char *TAG_ARMOR="<armor>";
const char *ETAG_ARMOR="</armor>";
const char *TAG_PWEP="<prim_wep>";
const char *ETAG_PWEP="</prim_wep>";
const char *TAG_SWEP="<sec_wep>";
const char *ETAG_SWEP="</sec_wep>";
const char *TAG_SNEAK="<sneaking>";
const char *ETAG_SNEAK="</sneaking>";
const char *TAG_LOCKPICK="<lockpicking>";
const char *ETAG_LOCKPICK="</lockpicking>";
const char *TAG_ACROBATICS="<acrobatics>";
const char *ETAG_ACROBATICS="</acrobatics>";
const char *TAG_PICKPOCKET="<pickpocket>";
const char *ETAG_PICKPOCKET="</pickpocket>";

long CClassManager::GetFileLength(){
	return this->file_length;
}

long CClassManager::GetBufferSize(){
	return this->file_buffer_size;
}

char *CClassManager::GetFilePart(unsigned long part){
	return this->file_buffer+(part*1000);
}

void SClass::Print(){
	puts("---------------------");
	printf("Class name: %s\nTeam: %s\nArmor: %s\nPrimary weapon: %s\nSecondary weapon: %s\n", this->name, this->team_name, this->armor, this->prim_wep, this->sec_wep);
}

void CClassManager::PrintAll(){
	for(short i=0; i<this->num_of_classes; i++) this->class_array[i].Print();
}

CClassManager::CClassManager(){
	this->file_buffer=NULL;
	FILE *hFile=fopen("class.xml", "r");
	if(!hFile){
		puts("Critical error! Could not open file class.xml!");
		exit(EXIT_FAILURE);
	}
	fseek(hFile, 0, SEEK_END);
	this->file_length=ftell(hFile);
	this->file_buffer_size=((this->file_length%1000)==0)?this->file_length+1:((this->file_length/1000)+1)*1000;
	rewind(hFile);
	this->file_buffer=new char[this->file_buffer_size];
	fread(this->file_buffer, this->file_length, 1, hFile);
	EraseComment();
	//file loaded
	char *pSz=strstr((char*)this->file_buffer, TAG_CHAR);
	if(pSz==NULL){
		puts("Failed while parsing XML.");
		exit(EXIT_FAILURE);
	}
	pSz+=strlen(TAG_CHAR);
	this->num_of_classes=0;
	do{
		if(strstr(pSz, TAG_CLASS)){
			pSz=strstr(pSz, TAG_CLASS)+strlen(TAG_CLASS);
			if(strstr(pSz, ETAG_CLASS)){
				pSz=strstr(pSz, ETAG_CLASS)+strlen(ETAG_CLASS);
				this->num_of_classes++;
			}
			else break;
		}
		else break;
	} while(pSz<(char*)(this->file_buffer+this->file_length));
	this->class_array=new SClass[this->num_of_classes];
	pSz=strstr((char*)this->file_buffer, TAG_CHAR)+strlen(TAG_CHAR);
	for(short i=0; i<this->num_of_classes;i++){
		this->class_array[i].class_id=(unsigned char)i;
		this->class_array[i].name=NULL;
		this->class_array[i].team_name=NULL;
		this->class_array[i].armor=NULL;
		this->class_array[i].prim_wep=NULL;
		this->class_array[i].sec_wep=NULL;
		for(short j=0; j<13; j++) this->class_array[i].abilities[j]=0;
	}
	for(int i=0; i<this->num_of_classes;i++){
		pSz=strstr(pSz, TAG_CLASS)+strlen(TAG_CLASS);
		while(memcmp(pSz, ETAG_CLASS+1, strlen(ETAG_CLASS+1))!=0){
			pSz=strstr(pSz, "<")+1;
			if(memcmp(pSz, TAG_NAME+1,strlen(TAG_NAME+1))==0){
				pSz+=strlen(TAG_NAME+1);
				this->class_array[i].name=new char[(strstr(pSz, ETAG_NAME)-pSz)+1];
				memcpy(this->class_array[i].name, pSz, strstr(pSz, ETAG_NAME)-pSz);
				this->class_array[i].name[strstr(pSz, ETAG_NAME)-pSz]=0;
				pSz=strstr(pSz, ETAG_NAME)+strlen(ETAG_NAME);
			}
			if(memcmp(pSz, TAG_DESCRIPTION+1,strlen(TAG_DESCRIPTION+1))==0){
				pSz+=strlen(TAG_DESCRIPTION+1);
				this->class_array[i].description=new char[(strstr(pSz, ETAG_DESCRIPTION)-pSz)+1];
				memcpy(this->class_array[i].description, pSz, strstr(pSz, ETAG_DESCRIPTION)-pSz);
				this->class_array[i].description[strstr(pSz, ETAG_DESCRIPTION)-pSz]=0;
				pSz=strstr(pSz, ETAG_DESCRIPTION)+strlen(ETAG_DESCRIPTION);
			}
			if(memcmp(pSz, TAG_TEAM+1,strlen(TAG_TEAM+1))==0){
				pSz+=strlen(TAG_TEAM+1);
				this->class_array[i].team_name=new char[(strstr(pSz, ETAG_TEAM)-pSz)+1];
				memcpy(this->class_array[i].team_name, pSz, strstr(pSz, ETAG_TEAM)-pSz);
				this->class_array[i].team_name[strstr(pSz, ETAG_TEAM)-pSz]=0;
				pSz=strstr(pSz, ETAG_TEAM)+strlen(ETAG_TEAM);
			}
			if(memcmp(pSz, TAG_STR+1,strlen(TAG_STR+1))==0){
				pSz+=strlen(TAG_STR+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[STR]);
				pSz=strstr(pSz, ETAG_STR)+strlen(ETAG_STR);
			}
			if(memcmp(pSz, TAG_DEX+1,strlen(TAG_DEX+1))==0){
				pSz+=strlen(TAG_DEX+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[DEX]);
				pSz=strstr(pSz, ETAG_DEX)+strlen(ETAG_DEX);
			}
			if(memcmp(pSz, TAG_MP+1,strlen(TAG_MP+1))==0){
				pSz+=strlen(TAG_MP+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[MP]);
				pSz=strstr(pSz, ETAG_MP)+strlen(ETAG_MP);
			}
			if(memcmp(pSz, TAG_HP+1,strlen(TAG_HP+1))==0){
				pSz+=strlen(TAG_HP+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[HP]);
				pSz=strstr(pSz, ETAG_HP);
			}
			if(memcmp(pSz, TAG_1H+1,strlen(TAG_1H+1))==0){
				pSz+=strlen(TAG_1H+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[WEP1H]);
				pSz=strstr(pSz, ETAG_1H);
			}
			if(memcmp(pSz, TAG_2H+1,strlen(TAG_2H+1))==0){
				pSz+=strlen(TAG_2H+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[WEP2H]);
				pSz=strstr(pSz, ETAG_2H)+strlen(ETAG_2H);
			}
			if(memcmp(pSz, TAG_BOW+1,strlen(TAG_BOW+1))==0){
				pSz+=strlen(TAG_BOW+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[BOW]);
				pSz=strstr(pSz, ETAG_BOW);
			}
			if(memcmp(pSz, TAG_XBOW+1,strlen(TAG_XBOW+1))==0){
				pSz+=strlen(TAG_XBOW+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[XBOW]);
				pSz=strstr(pSz, ETAG_XBOW);
			}
			if(memcmp(pSz, TAG_MAGIC+1,strlen(TAG_MAGIC+1))==0){
				pSz+=strlen(TAG_MAGIC+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[MLVL]);
				pSz=strstr(pSz, ETAG_MAGIC);
			}
			if(memcmp(pSz, TAG_ARMOR+1,strlen(TAG_ARMOR+1))==0){
				pSz+=strlen(TAG_ARMOR+1);
				this->class_array[i].armor=new char[(strstr(pSz, ETAG_ARMOR)-pSz)+1];
				memcpy(this->class_array[i].armor, pSz, strstr(pSz, ETAG_ARMOR)-pSz);
				this->class_array[i].armor[strstr(pSz, ETAG_ARMOR)-pSz]=0;
				pSz=strstr(pSz, ETAG_ARMOR)+strlen(ETAG_ARMOR);
			}
			if(memcmp(pSz, TAG_PWEP+1,strlen(TAG_PWEP+1))==0){
				pSz+=strlen(TAG_PWEP+1);
				this->class_array[i].prim_wep=new char[(strstr(pSz, ETAG_PWEP)-pSz)+1];
				memcpy(this->class_array[i].prim_wep, pSz, strstr(pSz, ETAG_PWEP)-pSz);
				this->class_array[i].prim_wep[strstr(pSz, ETAG_PWEP)-pSz]=0;
				pSz=strstr(pSz, ETAG_PWEP)+strlen(ETAG_PWEP);
			}
			if(memcmp(pSz, TAG_SWEP+1,strlen(TAG_SWEP+1))==0){
				pSz+=strlen(TAG_SWEP+1);
				this->class_array[i].sec_wep=new char[(strstr(pSz, ETAG_SWEP)-pSz)+1];
				memcpy(this->class_array[i].sec_wep, pSz, strstr(pSz, ETAG_SWEP)-pSz);
				this->class_array[i].sec_wep[strstr(pSz, ETAG_SWEP)-pSz]=0;
				pSz=strstr(pSz, ETAG_SWEP)+strlen(ETAG_SWEP);
			}
			if(!memcmp(pSz, TAG_SNEAK+1, strlen(TAG_SNEAK+1))){
				pSz+=strlen(TAG_SNEAK+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[SNEAK]);
				pSz=strstr(pSz, ETAG_SNEAK)+strlen(ETAG_SNEAK);
			}
			if(!memcmp(pSz, TAG_LOCKPICK+1, strlen(TAG_LOCKPICK+1))){
				pSz+=strlen(TAG_LOCKPICK+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[LOCKPICKING]);
				pSz=strstr(pSz, ETAG_LOCKPICK)+strlen(ETAG_LOCKPICK);
			}
			if(!memcmp(pSz, TAG_ACROBATICS+1, strlen(TAG_ACROBATICS+1))){
				pSz+=strlen(TAG_ACROBATICS+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[ACROBATICS]);
				pSz=strstr(pSz, ETAG_ACROBATICS)+strlen(ETAG_ACROBATICS);
			}
			if(!memcmp(pSz, TAG_PICKPOCKET+1, strlen(TAG_PICKPOCKET+1))){
				pSz+=strlen(TAG_PICKPOCKET+1);
				sscanf(pSz, "%hd", &this->class_array[i].abilities[PICKPOCKET]);
				pSz=strstr(pSz, ETAG_PICKPOCKET)+strlen(ETAG_PICKPOCKET);
			}
		}
	}
}

CClassManager::~CClassManager(){
	if(this->file_buffer){
		delete this->file_buffer;
		this->file_buffer=NULL;
	}
	this->file_buffer_size=0;
	this->file_length=0;
	if(this->class_array){
		for(int i=0; i<this->num_of_classes; i++){
			this->class_array[i].class_id=0;
			if(this->class_array[i].name) delete this->class_array[i].name;
			if(this->class_array[i].team_name) delete this->class_array[i].team_name;
			if(this->class_array[i].armor) delete this->class_array[i].armor;
			if(this->class_array[i].prim_wep) delete this->class_array[i].prim_wep;
			if(this->class_array[i].sec_wep) delete this->class_array[i].sec_wep;
		}
		delete this->class_array;
		this->class_array=NULL;
	}
}

void CClassManager::EraseComment(){
	char comment=0;
	char *ptr1=NULL, *ptr2=NULL;
	for(long i=0; i<this->file_length; i++){
		if(!(comment&0x01)){
			if(!memcmp("<!--", file_buffer+i, 4)){
				ptr1=file_buffer+i;
				comment|=0x01;
				i+=3;
			}
		}
		if((!(comment&0x10)) && (comment&0x01)){
			if(!memcmp("-->", file_buffer+i, 3)){
				i+=3;
				ptr2=file_buffer+i;
				comment|=0x10;
			}
		}
		if(!(comment^0x11)){
			memcpy(ptr1, ptr2, file_length-(ptr2-file_buffer));
			file_length-=(long)(ptr2-ptr1);
			comment=0;
		}
	}
	if(file_length%1000){
		file_buffer_size=file_length/1000;
		file_buffer_size++;
		file_buffer_size*=1000;
	} else file_buffer_size=file_length;
}