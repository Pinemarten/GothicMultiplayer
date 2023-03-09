
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

#include "CClasses.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pugixml.hpp>

CClassManager::CClassManager() {
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file("class.xml");
  if (!result) {
    SPDLOG_ERROR("Critical error! Could not open file class.xml!");
    return;
  }
  size_t n =
      std::distance(doc.child("character").children("class").begin(), doc.child("character").children("class").end());
  this->num_of_classes = n;
  this->class_array = new SClass[this->num_of_classes];

  SPDLOG_INFO("Numer of classes: {}", this->num_of_classes);

  for (short i = 0; i < this->num_of_classes; i++) {
    this->class_array[i].class_id = (unsigned char)i;
    this->class_array[i].name = NULL;
    this->class_array[i].team_name = NULL;
    this->class_array[i].armor = NULL;
    this->class_array[i].prim_wep = NULL;
    this->class_array[i].sec_wep = NULL;
    for (short j = 0; j < 13; j++) this->class_array[i].abilities[j] = 0;
  }

  int i = 0;
  for (auto classNode : doc.child("character").children("class")) {
    this->class_array[i].name = (char *)classNode.child("name").text().get();
    this->class_array[i].team_name = (char *)classNode.child("team").text().get();
    this->class_array[i].description = (char *)classNode.child("description").text().get();
    this->class_array[i].armor = (char *)classNode.child("armor").text().get();
    this->class_array[i].prim_wep = (char *)classNode.child("prim_wep").text().get();
    this->class_array[i].sec_wep = (char *)classNode.child("sec_wep").text().get();

    this->class_array[i].abilities[STR] = classNode.child("strength").text().as_int();
    this->class_array[i].abilities[DEX] = classNode.child("dexterity").text().as_int();
    this->class_array[i].abilities[MP] = classNode.child("mana").text().as_int();
    this->class_array[i].abilities[HP] = classNode.child("health").text().as_int();
    this->class_array[i].abilities[WEP1H] = classNode.child("onehweapon").text().as_int();
    this->class_array[i].abilities[WEP2H] = classNode.child("twohweapon").text().as_int();
    this->class_array[i].abilities[BOW] = classNode.child("bow").text().as_int();
    this->class_array[i].abilities[XBOW] = classNode.child("crossbow").text().as_int();
    this->class_array[i].abilities[MLVL] = classNode.child("magic_lvl").text().as_int();
    this->class_array[i].abilities[SNEAK] = classNode.child("sneaking").text().as_int();
    this->class_array[i].abilities[LOCKPICKING] = classNode.child("lockpicking").text().as_int();
    this->class_array[i].abilities[ACROBATICS] = classNode.child("acrobatics").text().as_int();
    this->class_array[i].abilities[PICKPOCKET] = classNode.child("pickpocket").text().as_int();

    i++;
  }
}

CClassManager::~CClassManager() {
  if (this->class_array) {
    for (int i = 0; i < this->num_of_classes; i++) {
      this->class_array[i].class_id = 0;
      if (this->class_array[i].name)
        delete this->class_array[i].name;
      if (this->class_array[i].team_name)
        delete this->class_array[i].team_name;
      if (this->class_array[i].armor)
        delete this->class_array[i].armor;
      if (this->class_array[i].prim_wep)
        delete this->class_array[i].prim_wep;
      if (this->class_array[i].sec_wep)
        delete this->class_array[i].sec_wep;
    }
    delete this->class_array;
    this->class_array = NULL;
  }
}