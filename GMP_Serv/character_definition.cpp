
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

#include "character_definition.h"

#include <spdlog/spdlog.h>
#include <string.h>

#include <pugixml.hpp>

CharacterDefinitionManager::CharacterDefinitionManager() {
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file("class.xml");
  if (!result) {
    SPDLOG_WARN("class.xml file not found. No character definitions loaded.");
    return;
  }

  for (auto classNode : doc.child("character").children("class")) {
    CharacterDefinition new_def;
    new_def.id = id_counter_;
    new_def.name = classNode.child("name").text().get();
    new_def.team_name = classNode.child("team").text().get();
    new_def.description = classNode.child("description").text().get();
    new_def.armor = classNode.child("armor").text().get();
    new_def.prim_wep = classNode.child("prim_wep").text().get();
    new_def.sec_wep = classNode.child("sec_wep").text().get();

    new_def.abilities[STR] = classNode.child("strength").text().as_int();
    new_def.abilities[DEX] = classNode.child("dexterity").text().as_int();
    new_def.abilities[MP] = classNode.child("mana").text().as_int();
    new_def.abilities[HP] = classNode.child("health").text().as_int();
    new_def.abilities[WEP1H] = classNode.child("onehweapon").text().as_int();
    new_def.abilities[WEP2H] = classNode.child("twohweapon").text().as_int();
    new_def.abilities[BOW] = classNode.child("bow").text().as_int();
    new_def.abilities[XBOW] = classNode.child("crossbow").text().as_int();
    new_def.abilities[MLVL] = classNode.child("magic").text().as_int();
    new_def.abilities[SNEAK] = classNode.child("sneaking").text().as_int();
    new_def.abilities[LOCKPICKING] = classNode.child("lockpicking").text().as_int();
    new_def.abilities[ACROBATICS] = classNode.child("acrobatics").text().as_int();
    new_def.abilities[PICKPOCKET] = classNode.child("pickpocket").text().as_int();
    character_definitions_[id_counter_] = std::move(new_def);
    id_counter_++;
  }
}