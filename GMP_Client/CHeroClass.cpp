
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
#include "CHeroClass.h"

const char *NONE="none";
const char *TAG_CHAR="<character>";
const char *ETAG_CHAR="</character>";
const char *TAG_CLASS="<class>";
const char *ETAG_CLASS="</class>";
const char *TAG_NAME="<name>";
const char *ETAG_NAME="</name>";
const char *TAG_DESCRIPTION="<description>";
const char *ETAG_DESCRIPTION="</description>";
const char *TAG_TYPE="<npctype>";
const char *ETAG_TYPE="</npctype>";
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
const char *TAG_ITEM="<item>";
const char *ETAG_ITEM="</item>";
const char *TAG_CODE="<code>";
const char *ETAG_CODE="</code>";
const char *TAG_COUNT="<count>";
const char *ETAG_COUNT="</count>";
const char *TAG_SNEAK="<sneaking>";
const char *ETAG_SNEAK="</sneaking>";
const char *TAG_LOCKPICK="<lockpicking>";
const char *ETAG_LOCKPICK="</lockpicking>";
const char *TAG_ACROBATICS="<acrobatics>";
const char *ETAG_ACROBATICS="</acrobatics>";
const char *TAG_PICKPOCKET="<pickpocket>";
const char *ETAG_PICKPOCKET="</pickpocket>";


// CHECK FOR SUMMON SPELLS 
zSTRING Sum = "TRF";

SHeroClass::~SHeroClass(void){
	if(!this->class_name.IsEmpty()) this->class_name.Clear();
	if(!this->team_name.IsEmpty()) this->team_name.Clear();
	for(size_t i=0; i<this->items.size(); i++) delete items[i];
	items.clear();
}

SHeroClass* CHeroClass::operator[](unsigned long i){ return (i<data.size())?data[i]:(SHeroClass*)NULL; }
CHeroClass::CHeroClass(const char *szData, BYTE size){
	char *pSz=(char*)strstr(szData, TAG_CHAR);
	if(!pSz){
		puts("Failed while parsing classes.");
		return;
	}
	pSz+=strlen(TAG_CHAR);
	size_t num_of_classes=0;
	do{
		if(strstr(pSz, TAG_CLASS)){
			pSz=strstr(pSz, TAG_CLASS)+strlen(TAG_CLASS);
			if(strstr(pSz, ETAG_CLASS)){
				pSz=strstr(pSz, ETAG_CLASS)+strlen(ETAG_CLASS);
				num_of_classes++;
			}
			else break;
		}
		else break;
	} while(pSz<(char*)(szData+size*1000));
	pSz=(char*)strstr(szData, TAG_CHAR)+strlen(TAG_CHAR);
	for(size_t i=0; i<num_of_classes;i++){
		pSz=strstr(pSz, TAG_CLASS)+strlen(TAG_CLASS);
		SHeroClass *tmp=new SHeroClass;
		tmp->armor.count=0;
		tmp->prim_wep.count=0;
		tmp->sec_wep.count=0;
		tmp->Type = CPlayer::NPC_HUMAN;
		tmp->mp=0;
		tmp->hp=100;
		tmp->strength=10;
		tmp->dexterity=10;
		tmp->class_description = "No description";
		for(short z=0; z<SHeroClass::AB_MAX; z++) tmp->skill[z]=0;
		char buffer[512];
		zSTRING szTmp;
		while(memcmp(pSz, ETAG_CLASS+1, strlen(ETAG_CLASS+1))!=0){
			pSz=strstr(pSz, "<")+1;
			if(memcmp(pSz, TAG_NAME+1,strlen(TAG_NAME+1))==0){
				memset(buffer, 0, 512);
				pSz+=strlen(TAG_NAME+1);
				memcpy(buffer, pSz, strstr(pSz, ETAG_NAME)-pSz);
				tmp->class_name=buffer;
				pSz=strstr(pSz, ETAG_NAME)+strlen(ETAG_NAME);
			}
			if(memcmp(pSz, TAG_DESCRIPTION+1,strlen(TAG_DESCRIPTION+1))==0){
				memset(buffer, 0, 512);
				pSz+=strlen(TAG_DESCRIPTION+1);
				memcpy(buffer, pSz, strstr(pSz, ETAG_DESCRIPTION)-pSz);
				tmp->class_description=buffer;
				pSz=strstr(pSz, ETAG_DESCRIPTION)+strlen(ETAG_DESCRIPTION);
			}
			if(memcmp(pSz, TAG_TEAM+1,strlen(TAG_TEAM+1))==0){
				memset(buffer, 0, 512);
				pSz+=strlen(TAG_TEAM+1);
				memcpy(buffer, pSz, strstr(pSz, ETAG_TEAM)-pSz);
				tmp->team_name=buffer;
				pSz=strstr(pSz, ETAG_TEAM)+strlen(ETAG_TEAM);
			}
			if(memcmp(pSz, TAG_TYPE+1,strlen(TAG_TYPE+1))==0){
				pSz+=strlen(TAG_TYPE+1);
				sscanf(pSz, "%hu", &tmp->Type);
				pSz=strstr(pSz, ETAG_TYPE)+strlen(ETAG_TYPE);
			}
			if(memcmp(pSz, TAG_STR+1,strlen(TAG_STR+1))==0){
				pSz+=strlen(TAG_STR+1);
				sscanf(pSz, "%hu", &tmp->strength);
				pSz=strstr(pSz, ETAG_STR)+strlen(ETAG_STR);
			}
			if(memcmp(pSz, TAG_DEX+1,strlen(TAG_DEX+1))==0){
				pSz+=strlen(TAG_DEX+1);
				sscanf(pSz, "%hu", &tmp->dexterity);
				pSz=strstr(pSz, ETAG_DEX)+strlen(ETAG_DEX);
			}
			if(memcmp(pSz, TAG_MP+1,strlen(TAG_MP+1))==0){
				pSz+=strlen(TAG_MP+1);
				sscanf(pSz, "%hu", &tmp->mp);
				pSz=strstr(pSz, ETAG_MP)+strlen(ETAG_MP);
			}
			if(memcmp(pSz, TAG_HP+1,strlen(TAG_HP+1))==0){
				pSz+=strlen(TAG_HP+1);
				sscanf(pSz, "%hu", &tmp->hp);
				pSz=strstr(pSz, ETAG_HP);
			}
			if(memcmp(pSz, TAG_1H+1,strlen(TAG_1H+1))==0){
				pSz+=strlen(TAG_1H+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_1HWEP);
				pSz=strstr(pSz, ETAG_1H);
			}
			if(memcmp(pSz, TAG_2H+1,strlen(TAG_2H+1))==0){
				pSz+=strlen(TAG_2H+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_2HWEP);
				pSz=strstr(pSz, ETAG_2H)+strlen(ETAG_2H);
			}
			if(memcmp(pSz, TAG_BOW+1,strlen(TAG_BOW+1))==0){
				pSz+=strlen(TAG_BOW+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_BOW);
				pSz=strstr(pSz, ETAG_BOW);
			}
			if(memcmp(pSz, TAG_XBOW+1,strlen(TAG_XBOW+1))==0){
				pSz+=strlen(TAG_XBOW+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_XBOW);
				pSz=strstr(pSz, ETAG_XBOW);
			}
			if(memcmp(pSz, TAG_MAGIC+1,strlen(TAG_MAGIC+1))==0){
				pSz+=strlen(TAG_MAGIC+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_MAGIC_LVL);
				pSz=strstr(pSz, ETAG_MAGIC);
			}
			if(!memcmp(pSz, TAG_SNEAK+1, strlen(TAG_SNEAK+1))){
				pSz+=strlen(TAG_SNEAK+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_SNEAK);
				pSz=strstr(pSz, ETAG_SNEAK);
			}
			if(!memcmp(pSz, TAG_LOCKPICK+1, strlen(TAG_LOCKPICK+1))){
				pSz+=strlen(TAG_LOCKPICK+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_LOCKPICK);
				pSz=strstr(pSz, ETAG_LOCKPICK);
			}
			if(!memcmp(pSz, TAG_ACROBATICS+1, strlen(TAG_ACROBATICS+1))){
				pSz+=strlen(TAG_ACROBATICS+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_ACROBATICS);
				pSz=strstr(pSz, ETAG_ACROBATICS);
			}
			if(!memcmp(pSz, TAG_PICKPOCKET+1, strlen(TAG_PICKPOCKET+1))){
				pSz+=strlen(TAG_PICKPOCKET+1);
				sscanf(pSz, "%hu", tmp->skill+SHeroClass::AB_PICKPOCKETS);
				pSz=strstr(pSz, ETAG_PICKPOCKET);
			}
			if(memcmp(pSz, TAG_ARMOR+1,strlen(TAG_ARMOR+1))==0){
				memset(buffer, 0, 512);
				pSz+=strlen(TAG_ARMOR+1);
				memcpy(buffer, pSz, strstr(pSz, ETAG_ARMOR)-pSz);
				if(!szTmp.IsEmpty()) szTmp.Clear();
				szTmp=buffer;
				tmp->armor.index=zCParser::GetParser()->GetIndex(szTmp);
				tmp->armor.count=1;
				pSz=strstr(pSz, ETAG_ARMOR)+strlen(ETAG_ARMOR);
			}
			if(memcmp(pSz, TAG_PWEP+1,strlen(TAG_PWEP+1))==0){
				memset(buffer, 0, 512);
				pSz+=strlen(TAG_PWEP+1);
				memcpy(buffer, pSz, strstr(pSz, ETAG_PWEP)-pSz);
				if(!szTmp.IsEmpty()) szTmp.Clear();
				szTmp=buffer;
				szTmp.Upper();
				tmp->prim_wep.index=zCParser::GetParser()->GetIndex(szTmp);
				tmp->prim_wep.count=1;
				pSz=strstr(pSz, ETAG_PWEP)+strlen(ETAG_PWEP);
			}
			if(memcmp(pSz, TAG_SWEP+1,strlen(TAG_SWEP+1))==0){
				memset(buffer, 0, 512);
				pSz+=strlen(TAG_SWEP+1);
				memcpy(buffer, pSz, strstr(pSz, ETAG_SWEP)-pSz);
				if(!szTmp.IsEmpty()) szTmp.Clear();
				szTmp=buffer;
				szTmp.Upper();
				tmp->sec_wep.index=zCParser::GetParser()->GetIndex(szTmp);
				tmp->sec_wep.count=1;
				pSz=strstr(pSz, ETAG_SWEP)+strlen(ETAG_SWEP);
			}
			if(!memcmp(pSz, TAG_ITEM+1,strlen(TAG_ITEM+1))){
				pSz+=strlen(TAG_ITEM+1);
				SItem *tmp_it=new SItem;
				tmp_it->count=1;
				do{
					pSz=strstr(pSz, "<")+1;
					if(!memcmp(pSz, TAG_CODE+1, strlen(TAG_CODE+1))){
						memset(buffer, 0, 512);
						pSz+=strlen(TAG_CODE+1);
						memcpy(buffer, pSz, strstr(pSz, ETAG_CODE)-pSz);
						if(!szTmp.IsEmpty()) szTmp.Clear();
						szTmp=buffer;
						szTmp.Upper();
						tmp_it->index=zCParser::GetParser()->GetIndex(szTmp);
						pSz=strstr(pSz, ETAG_CODE)+strlen(ETAG_CODE+1);
					}
					if(!memcmp(pSz, TAG_COUNT+1, strlen(TAG_COUNT+1))){
						pSz+=strlen(TAG_COUNT+1);
						sscanf(pSz, "%d", &tmp_it->count);
						pSz=strstr(pSz, ETAG_COUNT)+strlen(ETAG_COUNT+1);
					}
				} while(memcmp(pSz, ETAG_ITEM+1, strlen(ETAG_ITEM+1)));
				pSz+=strlen(ETAG_ITEM+1);
				tmp->items.push_back(tmp_it);
			}
		}
		this->data.push_back(tmp);
		if(!szTmp.IsEmpty()) szTmp.Clear();
	}
}

CHeroClass::~CHeroClass(){
	for(size_t i=0; i<data.size(); i++) delete data[i];
	data.clear();
}

DWORD CHeroClass::GetSize(){ return this->data.size(); }

enum{
	ONEHAND_WEAPON_SKILL=1,
	TWOHAND_WEAPON_SKILL,
	BOW_SKILL,
	XBOW_SKILL,
	PICKLOCK_SKILL,
	MAGIC_LEVEL=7,
	SNEAK_SKILL=8,
	ACROBATIC_SKILL=11,
	//ponie¿ej tylko wp³ywanie na wygl¹d w statach
	PICKPOCKET_SKILL=12,
	SMITH_SKILL=13,
	RUNE_SKILL=14,
	ALCHEMY_SKILL=15,
	TROPHY_SKILL=16,
};

void CHeroClass::EquipNPC(size_t offset, CPlayer* Player, bool clear_inventory){
	if(!Player) return;
	if(!Player->npc) return;
	Player->SetNpcType(data[offset]->Type);
	oCNpc* npc = Player->npc;
	if(clear_inventory){
		oCItem *ptr=NULL;
		ptr=npc->GetEquippedArmor();
		if(ptr) npc->UnequipItem(ptr);
		ptr=npc->GetEquippedMeleeWeapon();
		if(ptr) npc->UnequipItem(ptr);
		ptr=npc->GetEquippedRangedWeapon();
		if(ptr) npc->UnequipItem(ptr);
		npc->GetInventory()->ClearInventory();
	}
	if(offset>=this->data.size()) return;
	npc->SetStrength(data[offset]->strength);
	npc->SetDexterity(data[offset]->dexterity);
	npc->SetMaxHealth(data[offset]->hp);
	npc->SetHealth(data[offset]->hp);
	npc->SetMaxMana(data[offset]->mp);
	npc->SetMana(data[offset]->mp);
	npc->SetTalentSkill(ONEHAND_WEAPON_SKILL, data[offset]->skill[SHeroClass::AB_1HWEP]/30);
	npc->SetTalentValue(ONEHAND_WEAPON_SKILL, data[offset]->skill[SHeroClass::AB_1HWEP]/30);
	npc->hitChance[1]=data[offset]->skill[SHeroClass::AB_1HWEP];
	npc->SetTalentSkill(TWOHAND_WEAPON_SKILL, data[offset]->skill[SHeroClass::AB_2HWEP]/30);
	npc->SetTalentValue(TWOHAND_WEAPON_SKILL, data[offset]->skill[SHeroClass::AB_2HWEP]/30);
	npc->hitChance[2]=data[offset]->skill[SHeroClass::AB_2HWEP];
	npc->SetTalentSkill(BOW_SKILL, data[offset]->skill[SHeroClass::AB_BOW]/30);
	npc->SetTalentValue(BOW_SKILL, data[offset]->skill[SHeroClass::AB_BOW]/30);
	npc->hitChance[3]=data[offset]->skill[SHeroClass::AB_BOW];
	npc->SetTalentSkill(XBOW_SKILL, data[offset]->skill[SHeroClass::AB_XBOW]/30);
	npc->SetTalentValue(XBOW_SKILL, data[offset]->skill[SHeroClass::AB_XBOW]/30);
	npc->hitChance[4]=data[offset]->skill[SHeroClass::AB_XBOW];
	npc->SetTalentSkill(MAGIC_LEVEL, data[offset]->skill[SHeroClass::AB_MAGIC_LVL]);
	npc->SetTalentValue(MAGIC_LEVEL, data[offset]->skill[SHeroClass::AB_MAGIC_LVL]);
	npc->SetTalentSkill(SNEAK_SKILL, data[offset]->skill[SHeroClass::AB_SNEAK]);
	npc->SetTalentValue(SNEAK_SKILL, data[offset]->skill[SHeroClass::AB_SNEAK]);
	npc->SetTalentSkill(ACROBATIC_SKILL, data[offset]->skill[SHeroClass::AB_ACROBATICS]);
	npc->SetTalentValue(ACROBATIC_SKILL, data[offset]->skill[SHeroClass::AB_ACROBATICS]);
	npc->SetTalentSkill(PICKLOCK_SKILL, data[offset]->skill[SHeroClass::AB_LOCKPICK]);
	npc->SetTalentValue(PICKLOCK_SKILL, data[offset]->skill[SHeroClass::AB_LOCKPICK]);
	oCObjectFactory *objectfactory=oCObjectFactory::GetFactory();
	if(npc == oCNpc::GetHero()){ if(data[offset]->armor.count) npc->Equip(npc->GetInventory()->Insert(objectfactory->CreateItem(data[offset]->armor.index)));}
	if(npc == oCNpc::GetHero()){
		if(data[offset]->sec_wep.count) npc->EquipWeapon(npc->GetInventory()->Insert(objectfactory->CreateItem(data[offset]->sec_wep.index)));
		if(data[offset]->prim_wep.count) npc->EquipWeapon(npc->GetInventory()->Insert(objectfactory->CreateItem(data[offset]->prim_wep.index)));
	}
	else{
		if(data[offset]->sec_wep.count) npc->GetInventory()->Insert(objectfactory->CreateItem(data[offset]->sec_wep.index));
		if(data[offset]->prim_wep.count) npc->GetInventory()->Insert(objectfactory->CreateItem(data[offset]->prim_wep.index));
	}
	//printf("Number of items: %u\n", data[offset]->items.size());
	npc->DestroySpellBook();
	npc->MakeSpellBook();
	for(size_t i=0; i<data[offset]->items.size(); i++){
		oCItem* Item = objectfactory->CreateItem(data[offset]->items[i]->index);
		if(Item){
			Item->SetAmount(data[offset]->items[i]->count);
			npc->GetInventory()->Insert(Item);
			if(npc == oCNpc::GetHero()){
				if(Item->HasFlag(512)){
					if(Item->GetInstanceName().Search(Sum) < 2){
						if(npc->CanUse(Item)) npc->Equip(Item);
					}
					else {
						npc->GetInventory()->Remove(Item);
						Item->RemoveVobFromWorld();
					}
				}
			}
		}
	}
}