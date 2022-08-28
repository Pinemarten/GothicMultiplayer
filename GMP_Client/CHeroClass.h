
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
#include <g2Api.h>

#include <vector>

#include "CPlayer.h"
using namespace std;

struct SItem
{
  int index;
  int count;
};

struct SHeroClass
{
  enum
  {
    AB_1HWEP = 0,
    AB_2HWEP,
    AB_BOW,
    AB_XBOW,
    AB_MAGIC_LVL,
    AB_SNEAK,
    AB_LOCKPICK,
    AB_ACROBATICS,
    AB_PICKPOCKETS,
    AB_MAX
  };
  CPlayer::NpcType Type;
  zSTRING class_name;
  zSTRING class_description;
  zSTRING team_name;
  USHORT strength, dexterity, mp, hp;
  USHORT skill[AB_MAX];
  SItem armor;
  SItem prim_wep;
  SItem sec_wep;
  vector<SItem*> items;
  ~SHeroClass(void);
};

class CHeroClass
{
public:
  CHeroClass(const char* szData, BYTE size);
  ~CHeroClass(void);
  void EquipNPC(size_t offset, CPlayer* Player, bool clear_inventory);
  DWORD GetSize(void);
  SHeroClass* operator[](unsigned long);

private:
  vector<SHeroClass*> data;
};
