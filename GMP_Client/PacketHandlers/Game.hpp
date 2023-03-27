
/*
MIT License

Copyright (c) 2022 Gothic Multiplayer Team.

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

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/ext/std_optional.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
#include <spdlog/spdlog.h>

#include "../CChat.h"
#include "../game_client.h"
#include "packets.h"

extern zCOLOR RED;
namespace Game {
void OnMapName(GameClient* client, Packet p) {
  MapNamePacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  std::string currentMap = oCGame::GetGame()->GetGameWorld()->GetWorldName().ToChar();

  if (packet.map_name != currentMap) {
    client->map = packet.map_name.c_str();
  } else if (!client->map.IsEmpty()) {
    client->map.Clear();
  }
}

void OnInGame(GameClient* client, Packet packet) {
  client->IsInGame = true;
}

void OnActualStatistics(GameClient* client, Packet p) {
  static const zSTRING Door = "DOOR";
  static const zSTRING BowSound = "BOWSHOOT";
  static const zSTRING CrossbowSound = "CROSSBOWSHOOT";
  static const zSTRING Lever = "LEVER";
  static const zSTRING Touchplate = "TOUCHPLATE";
  static const zSTRING VWheel = "VWHEEL";
  static const zSTRING STUMBLE = "T_STUMBLE";
  static const zSTRING GOTHIT = "T_GOTHIT";
  static const zSTRING HitSound = "SVM_15_AARGH_4";
  static const zSTRING Arrows = "ITRW_ARROW";
  static const zSTRING Bolt = "ITRW_BOLT";

  PlayerStateUpdatePacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  if (client->game_mode == 1) {
    for (size_t i = 1; i < client->player.size(); i++) {
      if (!memcmp((*client->classmgr)[client->player[i]->char_class]->team_name.ToChar(),
                  (*client->classmgr)[client->player[0]->char_class]->team_name.ToChar(),
                  (*client->classmgr)[client->player[0]->char_class]->team_name.Length())) {
        client->player[i]->npc->variousFlags = 1 << 1;
        client->player[i]->SameTeamAsLocalPlayer = true;
      }
    }
  }

  CPlayer* player = nullptr;
  for (size_t i = 0; i < client->player.size(); i++) {
    if (client->player[i]->id == *packet.player_id) {
      player = client->player[i];
      break;
    };
  }

  if (player) {
    zVEC3 pos(packet.state.position.x, packet.state.position.y, packet.state.position.z);
    if (!player->enable) {
      player->npc->Enable(pos);
      player->enable = TRUE;
    } else {
      player->AnalyzePosition(pos);
    }

    if (!player->npc->IsDead()) {
      player->npc->SetAngle(-packet.state.nrot[0], packet.state.nrot[1], 0.0f, packet.state.nrot[0], packet.state.nrot[1], packet.state.nrot[2]);
    }
    if (packet.state.left_hand_item_instance == 0) {
      if (player->npc->GetLeftHand()) {
        oCItem* lptr = player->npc->GetLeftHand();
        oCItem* ptr = player->npc->GetRightHand();
        player->npc->DropAllInHand();
        lptr->RemoveVobFromWorld();
        if (ptr)
          player->npc->SetRightHand(ptr);
      }
    } else if (packet.state.left_hand_item_instance > 5892 && packet.state.left_hand_item_instance < 7850) {
      if (!player->npc->GetLeftHand()) {
        oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.left_hand_item_instance));
        player->npc->SetLeftHand(New);
        client->CheckForSpecialEffects(player->npc->GetLeftHand(), player->npc);
      } else {
        if (player->npc->GetLeftHand()->GetInstance() != static_cast<int>(packet.state.left_hand_item_instance)) {
          oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.left_hand_item_instance));
          oCItem* Old = player->npc->GetLeftHand();
          player->npc->SetLeftHand(New);
          Old->RemoveVobFromWorld();
          client->CheckForSpecialEffects(player->npc->GetLeftHand(), player->npc);
        }
      }
    }
    if (packet.state.right_hand_item_instance == 0) {
      if (player->npc->GetRightHand()) {
        oCItem* rptr = player->npc->GetRightHand();
        oCItem* ptr = player->npc->GetLeftHand();
        player->npc->DropAllInHand();
        if (ptr)
          player->npc->SetLeftHand(ptr);
        client->PlayDrawSound(rptr, player->npc, false);
        rptr->RemoveVobFromWorld();
      }
    } else if (packet.state.right_hand_item_instance > 5892 && packet.state.right_hand_item_instance < 7850) {
      if (!player->npc->GetRightHand()) {
        oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.right_hand_item_instance));
        player->npc->SetRightHand(New);
        client->PlayDrawSound(player->npc->GetRightHand(), player->npc, true);
      } else {
        if (player->npc->GetRightHand()->GetInstance() != static_cast<int>(packet.state.right_hand_item_instance)) {
          oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.right_hand_item_instance));
          oCItem* Old = player->npc->GetRightHand();
          player->npc->SetRightHand(New);
          Old->RemoveVobFromWorld();
          client->PlayDrawSound(player->npc->GetRightHand(), player->npc, true);
        }
      }
    }

    if (packet.state.equipped_armor_instance == 0) {
      if (player->npc->GetEquippedArmor()) {
        player->npc->UnequipItem(player->npc->GetEquippedArmor());
      }
    } else if (packet.state.equipped_armor_instance > 5892 && packet.state.equipped_armor_instance < 7850) {
      if (!player->npc->GetEquippedArmor()) {
        if (!player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.equipped_armor_instance), 1)) {
          oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.equipped_armor_instance));
          player->npc->GetInventory()->Insert(New);
          player->npc->Equip(New);
        } else
          player->npc->Equip(player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.equipped_armor_instance), 1));
      } else {
        if (player->npc->GetEquippedArmor()->GetInstance() != static_cast<int>(packet.state.equipped_armor_instance)) {
          if (!player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.equipped_armor_instance), 1)) {
            oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.equipped_armor_instance));
            player->npc->GetInventory()->Insert(New);
            player->npc->Equip(New);
          } else
            player->npc->Equip(player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.equipped_armor_instance), 1));
        }
      }
    }
    if (player->npc && packet.state.animation > 0 && packet.state.animation < 1400) {
      zCModelAni* Ani = player->npc->GetModel()->GetAniFromAniID(static_cast<int>(packet.state.animation));
      if (Ani && !player->npc->IsDead()) {
        if (!IsBadCodePtr((FARPROC)&Ani->GetAniName())) {
          if (!Ani->GetAniName().IsEmpty()) {
            if (Ani->GetAniName().Search(Door) == 2) {
              if (!player->npc->GetInteractMob()) {
                oCMobLockable* ALocked = static_cast<oCMobLockable*>(player->npc->FindMobInter(Door));
                if (ALocked) {
                  ALocked->SetMobBodyState(player->npc);
                  ALocked->AI_UseMobToState(player->npc, !ALocked->GetState());
                }
              } else {
                player->npc->GetInteractMob()->SetMobBodyState(player->npc);
                player->npc->GetInteractMob()->AI_UseMobToState(player->npc, !player->npc->GetInteractMob()->GetState());
              }
            } else if (player->npc->GetInteractMob()) {
              if (player->npc->GetInteractMob()->GetState() == 0)
                player->npc->GetInteractMob()->SendEndInteraction(player->npc, 0, 1);
              else
                player->npc->GetInteractMob()->SendEndInteraction(player->npc, 1, 0);
            }
            if (!player->npc->GetModel()->IsAnimationActive(Ani->GetAniName())) {
              player->npc->GetModel()->StartAnimation(Ani->GetAniName());
              if (!memcmp("T_BOWRELOAD", Ani->GetAniName().ToChar(), 11)) {
                oCItem* Bullet = oCObjectFactory::GetFactory()->CreateItem(zCParser::GetParser()->GetIndex(Arrows));
                zCSoundSystem::GetSoundSystem()->PlaySound3D(BowSound, client->player[0]->npc, 2);
                player->npc->SetRightHand(Bullet);
                oCItem* Arrowe = player->npc->GetInventory()->IsIn(7083, 1);
                if (Arrowe)
                  player->npc->GetInventory()->Remove(7083, 1);
                player->npc->DoShootArrow(1);
              }
              if (!memcmp("T_CBOWRELOAD", Ani->GetAniName().ToChar(), 12)) {
                oCItem* Bullet = oCObjectFactory::GetFactory()->CreateItem(zCParser::GetParser()->GetIndex(Bolt));
                zCSoundSystem::GetSoundSystem()->PlaySound3D(CrossbowSound, client->player[0]->npc, 2);
                player->npc->SetLeftHand(Bullet);
                oCItem* Bolte = player->npc->GetInventory()->IsIn(7084, 1);
                if (Bolte)
                  player->npc->GetInventory()->Remove(7084, 1);
                player->npc->DoShootArrow(1);
              }
              if (!memcmp("T_LEVER_S0_2_S1", Ani->GetAniName().ToChar(), 15)) {
                oCMobInter* LeverSwitch = player->npc->FindMobInter(Lever);
                if (LeverSwitch) {
                  if (!LeverSwitch->GetTriggerName().IsEmpty()) {
                    zCMover* Mover = static_cast<zCMover*>(oCGame::GetGame()->GetGameWorld()->SearchVobByName(LeverSwitch->GetTriggerName()));
                    if (Mover)
                      Mover->TriggerMover(Mover);
                  }
                }
              }
              if (!memcmp("T_TOUCHPLATE_S0_2_S1", Ani->GetAniName().ToChar(), 20) ||
                  !memcmp("T_TOUCHPLATE_S1_2_S0", Ani->GetAniName().ToChar(), 20)) {
                oCMobInter* LeverSwitch = player->npc->FindMobInter(Touchplate);
                if (LeverSwitch) {
                  if (!LeverSwitch->GetTriggerName().IsEmpty()) {
                    zCMover* Mover = static_cast<zCMover*>(oCGame::GetGame()->GetGameWorld()->SearchVobByName(LeverSwitch->GetTriggerName()));
                    if (Mover)
                      Mover->TriggerMover(Mover);
                  }
                }
              }
              if (!memcmp("T_VWHEEL_S0_2_S1", Ani->GetAniName().ToChar(), 16)) {
                oCMobInter* LeverSwitch = player->npc->FindMobInter(VWheel);
                if (LeverSwitch) {
                  if (!LeverSwitch->GetTriggerName().IsEmpty()) {
                    zCMover* Mover = static_cast<zCMover*>(oCGame::GetGame()->GetGameWorld()->SearchVobByName(LeverSwitch->GetTriggerName()));
                    if (Mover)
                      Mover->TriggerMover(Mover);
                  }
                }
              }
            }
          }
        }
      }
    }
    if (packet.state.health_points > player->npc->GetHealth()) {
      if (player->npc->GetHealth() == 1) {
        player->npc->RefreshNpc();
        player->npc->SetHealth(1);
      }
    }
    if ((!player->hp) && (packet.state.health_points == player->npc->GetMaxHealth())) {
      player->hp = packet.state.health_points;
      player->npc->ResetPos(player->npc->GetPosition());
    } else if ((player->npc->GetHealth() > 0) && (packet.state.health_points == 0)) {
      player->hp = 0;
    } else {
      if (player->update_hp_packet >= 5) {
        player->hp = packet.state.health_points;
        player->npc->SetHealth(static_cast<int>(packet.state.health_points));
        player->update_hp_packet = 0;
      } else
        player->update_hp_packet++;
    }
    player->npc->SetMana(static_cast<int>(packet.state.mana_points));

    BYTE SpellNr = static_cast<BYTE>(packet.state.active_spell_nr);
    if (SpellNr != player->npc->GetActiveSpellNr() && SpellNr > 0 && SpellNr < 100) {
      for (int s = 0; s < player->npc->GetSpellBook()->GetNoOfSpells(); s++) {
        player->npc->Equip(player->npc->GetSpellBook()->GetSpellItem(s));
      }
      oCItem* SpellItem = player->npc->GetSpellItem((int)SpellNr);
      if (SpellItem) {
        player->npc->Equip(SpellItem);
        player->npc->GetSpellBook()->Open(0);
      }
    } else if (SpellNr == 0 && player->npc->GetActiveSpellNr() > 0) {
      player->npc->GetSpellBook()->Close(1);
    } else if (player->npc->IsDead()) {
      player->npc->GetSpellBook()->Close(1);
    }
    if ((BYTE)player->npc->GetWeaponMode() != packet.state.weapon_mode) {
      player->npc->SetWeaponMode((oCNpc_WeaponMode)packet.state.weapon_mode);
    }
    switch ((CPlayer::HeadState)packet.state.head_direction) {
      case CPlayer::HEAD_NONE:
        player->npc->GetAnictrl()->SetLookAtTarget(0.5f, 0.5f);
        break;
      case CPlayer::HEAD_LEFT:
        player->npc->GetAnictrl()->SetLookAtTarget(0.0f, 0.5f);
        break;
      case CPlayer::HEAD_RIGHT:
        player->npc->GetAnictrl()->SetLookAtTarget(1.0f, 0.5f);
        break;
      case CPlayer::HEAD_UP:
        player->npc->GetAnictrl()->SetLookAtTarget(0.5f, 0.0f);
        break;
      case CPlayer::HEAD_DOWN:
        player->npc->GetAnictrl()->SetLookAtTarget(0.5f, 1.0f);
        break;
    }
    if (packet.state.ranged_weapon_instance == 0) {
      if (player->npc->GetEquippedRangedWeapon()) {
        player->npc->UnequipItem(player->npc->GetEquippedRangedWeapon());
      }
    } else if (packet.state.ranged_weapon_instance > 5892 && packet.state.ranged_weapon_instance < 7850) {
      if (!player->npc->GetEquippedRangedWeapon()) {
        if (!player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.ranged_weapon_instance), 1)) {
          oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.ranged_weapon_instance));
          player->npc->GetInventory()->Insert(New);
          player->npc->Equip(New);
        } else
          player->npc->Equip(player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.ranged_weapon_instance), 1));
      } else {
        if (player->npc->GetEquippedRangedWeapon()->GetInstance() != static_cast<int>(packet.state.ranged_weapon_instance)) {
          if (!player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.ranged_weapon_instance), 1)) {
            oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.ranged_weapon_instance));
            player->npc->GetInventory()->Insert(New);
            player->npc->Equip(New);
          } else
            player->npc->Equip(player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.ranged_weapon_instance), 1));
        }
      }
    }
    if (packet.state.melee_weapon_instance == 0) {
      if (player->npc->GetEquippedMeleeWeapon()) {
        player->npc->UnequipItem(player->npc->GetEquippedMeleeWeapon());
      }
    } else if (packet.state.melee_weapon_instance > 5892 && packet.state.melee_weapon_instance < 7850) {
      if (!player->npc->GetEquippedMeleeWeapon()) {
        if (!player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.melee_weapon_instance), 1)) {
          oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.melee_weapon_instance));
          player->npc->GetInventory()->Insert(New);
          player->npc->Equip(New);
        } else
          player->npc->Equip(player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.melee_weapon_instance), 1));
      } else {
        if (player->npc->GetEquippedMeleeWeapon()->GetInstance() != static_cast<int>(packet.state.melee_weapon_instance)) {
          if (!player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.melee_weapon_instance), 1)) {
            oCItem* New = oCObjectFactory::GetFactory()->CreateItem(static_cast<int>(packet.state.melee_weapon_instance));
            player->npc->GetInventory()->Insert(New);
            player->npc->Equip(New);
          } else
            player->npc->Equip(player->npc->GetInventory()->IsIn(static_cast<int>(packet.state.melee_weapon_instance), 1));
        }
      }
    }
  }
  // This looks unrelated to packet handling... 
  // else {
  //   if (client->player[0]->update_hp_packet >= 5) {
  //     short last_hp = client->player[0]->hp;
  //     memcpy(&client->player[0]->hp, packet.data + pIt, 2);
  //     pIt += 2;
  //     if ((!last_hp) && (client->player[0]->hp == client->player[0]->npc->GetMaxHealth())) {
  //       client->player[0]->npc->RefreshNpc();
  //       client->player[0]->npc->SetMovLock(0);
  //       if (client->game_mode == 0) {
  //         client->player[0]->npc->ResetPos(*(*client->spawnpoint)[rand() % client->spawnpoint->GetSize()]);
  //       }
  //       if (client->game_mode == 1) {
  //         std::vector<const char*> team_list;
  //         team_list.push_back((*client->classmgr)[0]->team_name.ToChar());
  //         for (size_t x = 1; x < client->classmgr->GetSize(); x++) {
  //           bool match = false;
  //           for (size_t y = 0; y < team_list.size(); y++) {
  //             if (!memcmp((*client->classmgr)[x]->team_name.ToChar(), team_list[y], strlen(team_list[y]) + 1)) {
  //               match = true;
  //               break;
  //             }
  //           }
  //           if (!match) {
  //             team_list.push_back((*client->classmgr)[x]->team_name.ToChar());
  //           }
  //         }
  //         size_t z;
  //         for (z = 0; z < team_list.size(); z++) {
  //           if (!memcmp((*client->classmgr)[client->player[0]->char_class]->team_name.ToChar(), team_list[z], strlen(team_list[z]) + 1)) {
  //             break;
  //           }
  //         }
  //         client->player[0]->npc->ResetPos(
  //             *(*client->spawnpoint)[(rand() % (client->spawnpoint->GetSize() / team_list.size())) * team_list.size() + z]);
  //         team_list.clear();
  //       }
  //     } else
  //       client->player[0]->SetHealth(static_cast<int>(client->player[0]->hp));
  //     client->player[0]->update_hp_packet = 0;
  //     if (static_cast<int>(client->player[0]->hp) < client->HeroLastHp) {
  //       if (!client->player[0]->npc->IsDead() && client->player[0]->npc->GetBodyState() != 10 && client->player[0]->npc->GetBodyState() != 12 &&
  //           client->player[0]->npc->GetBodyState() != 22 && !client->player[0]->npc->GetAnictrl()->IsInWater()) {
  //         if (client->player[0]->npc->GetAnictrl()->IsRunning()) {
  //           client->player[0]->npc->GetModel()->StartAnimation(GOTHIT);
  //         } else {
  //           client->player[0]->npc->GetModel()->StartAnimation(STUMBLE);
  //         }
  //         int RandomSound = rand() % 2 + 1;
  //         if (RandomSound == 2) {
  //           zCSoundSystem::GetSoundSystem()->PlaySound3D(HitSound, client->player[0]->GetNpc(), 2);
  //         }
  //       }
  //       client->HeroLastHp = static_cast<int>(client->player[0]->hp);
  //     } else if (static_cast<int>(client->player[0]->hp) != client->HeroLastHp)
  //       client->HeroLastHp = static_cast<int>(client->player[0]->hp);
  //   } else {
  //     client->player[0]->update_hp_packet++;
  //     pIt += 2;
  //   }
  // }
}

void OnMapOnly(GameClient* client, Packet p) {
  PlayerPositionUpdatePacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  if (!packet.player_id) {
    SPDLOG_ERROR("PlayerPositionUpdatePacket: Player id is null");
    return;
  }

  for (size_t i = 0; i < client->player.size(); i++) {
    if (client->player[i]->id == *packet.player_id) {
      const auto& pos = packet.position;
      client->player[i]->npc->SetPosition(pos[0], client->player[i]->npc->GetPosition().y, pos[1]);
      client->player[i]->DisablePlayer();
    }
  }
}

void OnDoDie(GameClient* client, Packet p) {
  PlayerDeathInfoPacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  for (size_t i = 0; i < client->player.size(); i++) {
    if (client->player[i]->id == packet.player_id) {
      client->player[i]->hp = 0;
      client->player[i]->update_hp_packet = 0;
      client->player[i]->SetHealth(0);
    }
  }
}

void OnRespawn(GameClient* client, Packet p) {
  PlayerRespawnInfoPacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  for (size_t i = 0; i < client->player.size(); i++) {
    if (client->player[i]->id == packet.player_id) {
      client->player[i]->RespawnPlayer();
    }
  }
}

void OnCastSpell(GameClient* client, Packet p) {
  CastSpellPacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  if (!packet.caster_id) {
    SPDLOG_ERROR("CastSpellPacket Caster ID is null");
    return;
  }

  if (packet.spell_id >= 0 && packet.spell_id < 100) {
    for (size_t i = 0; i < client->player.size(); i++) {
      if (client->player[i]->id == *packet.caster_id) {
        oCSpell* Spell = oCSpell::_CreateNewInstance();
        Spell->InitValues(packet.spell_id);
        Spell->Setup(client->player[i]->GetNpc(), 0, 0);
        client->RunSpellLogic(packet.spell_id, client->player[i]->GetNpc(), 0);
        client->RunSpellScript(Spell->GetSpellInstanceName(packet.spell_id).ToChar(), client->player[i]->GetNpc());
        Spell->Cast();
      }
    }
  }
}

void OnCastSpellOnTarget(GameClient* client, Packet p) {
  CastSpellPacket packet;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({p.data, p.length}, packet);

  if (!packet.caster_id || !packet.target_id) {
    SPDLOG_ERROR("Invalid CastSpellOnTarget packet. No caster or target id.");
  }

  CPlayer* caster = nullptr;
  CPlayer* target = nullptr;

  if (packet.spell_id >= 0 && packet.spell_id < 100) {
    for (size_t i = 0; i < client->player.size(); i++) {
      if (client->player[i]->id == *packet.caster_id) {
        caster = client->player[i];
        if (target)
          break;
      } else if (client->player[i]->id == *packet.target_id) {
        target = client->player[i];
        if (caster)
          break;
      }
    }
  }

  if (!caster || !target) {
    SPDLOG_WARN("Invalid CastSpellOnTarget packet. No caster or target found.");
  }

  oCSpell* Spell = oCSpell::_CreateNewInstance();
  Spell->InitValues(packet.spell_id);
  Spell->Setup(caster->GetNpc(), target->GetNpc(), 0);
  client->RunSpellLogic(packet.spell_id, caster->GetNpc(), target->GetNpc());
  client->RunSpellScript(Spell->GetSpellInstanceName(packet.spell_id).ToChar(), caster->GetNpc());
  Spell->Cast();
}

void OnDropItem(GameClient* client, Packet packet) {
  DropItemPacket dropItemPacket;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({packet.data, packet.length}, dropItemPacket);

  if (!dropItemPacket.player_id) {
    SPDLOG_ERROR("Invalid DropItem packet. No player id.");
  }

  if (dropItemPacket.item_instance > 5892 && dropItemPacket.item_instance < 7850) {
    for (size_t i = 0; i < client->player.size(); i++) {
      if (!client->player[i]->id == *dropItemPacket.player_id) {
        oCWorld* world = oCGame::GetGame()->GetGameWorld();
        oCItem* NpcDrop = oCObjectFactory::GetFactory()->CreateItem(dropItemPacket.item_instance);
        NpcDrop->SetAmount(dropItemPacket.item_amount);
        zVEC3 startPos = client->player[i]->npc->GetTrafoModelNodeToWorld("ZS_RIGHTHAND").GetTranslation();
        NpcDrop->SetPosition(startPos);
        world->AddVob(NpcDrop);
        NpcDrop->SetSleeping(false);
        NpcDrop->SetStaticVob(false);
        NpcDrop->SetPhysicsEnabled(true);
      }
    }
  }
}

void OnTakeItem(GameClient* client, Packet packet) {
  TakeItemPacket takeItemPacket;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({packet.data, packet.length}, takeItemPacket);

  if (!takeItemPacket.player_id) {
    SPDLOG_ERROR("Invalid TakeItem packet. No player id.");
    return;
  }

  if (takeItemPacket.item_instance > 5892 && takeItemPacket.item_instance < 7850) {
    zCListSort<oCItem>* ItemList = oCGame::GetGame()->GetWorld()->GetItemList();
    int size = ItemList->GetSize();
    for (size_t x = 0; x < client->player.size(); x++) {
      if (client->player[x]->id == *takeItemPacket.player_id) {
        for (int i = 0; i < size; i++) {
          ItemList = ItemList->GetNext();
          oCItem* ItemInList = ItemList->GetData();
          if (ItemInList->GetInstance() == takeItemPacket.item_instance) {
            if (client->player[x]->npc->GetDistanceToVob(ItemInList) < 250.0f) {
              client->player[x]->npc->DoTakeVob(ItemInList);
              break;
            }
          }
        }
      }
    }
  }
}

void OnWhisper(GameClient* client, Packet packet) {
  MessagePacket messagePacket;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({packet.data, packet.length}, messagePacket);

  if (!messagePacket.sender) {
    SPDLOG_ERROR("Invalid Message packet. No sender id.");
    return;
  }

  for (size_t i = 0; i < client->player.size(); i++) {
    if (*messagePacket.sender == client->player[i]->id) {
      CChat::GetInstance()->WriteMessage(WHISPER, true, zCOLOR(0, 255, 255, 255), "%s-> %s", client->player[i]->npc->GetName().ToChar(),
                                         messagePacket.message.c_str());
      return;
    }
  }
}

void OnMessage(GameClient* client, Packet packet) {
  MessagePacket messagePacket;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({packet.data, packet.length}, messagePacket);

  if (!messagePacket.sender) {
    SPDLOG_ERROR("Invalid Message packet. No sender id.");
    return;
  }

  for (size_t i = 0; i < client->player.size(); i++) {
    if (*messagePacket.sender == client->player[i]->id) {
      SPDLOG_INFO("Message from player: {} ({}): {}", client->player[i]->npc->GetName().ToChar(), client->player[i]->GetName(), messagePacket);
      CChat::GetInstance()->WriteMessage(NORMAL, false, "%s: %s", client->player[i]->npc->GetName().ToChar(), messagePacket.message.c_str());
      return;
    }
  }
}

void OnServerMessage(GameClient* client, Packet packet) {
  MessagePacket messagePacket;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({packet.data, packet.length}, messagePacket);
  CChat::GetInstance()->WriteMessage(NORMAL, false, zCOLOR(255, 128, 0, 255), "(SERVER): %s", messagePacket.message.c_str());
}

void OnRcon(GameClient* client, Packet packet) {
  if (packet.data[1] == 0x41) {
    client->IsAdminOrModerator = true;
  }
  CChat::GetInstance()->WriteMessage(ADMIN, false, RED, "%s", (char*)packet.data + 1);
}

void OnAllOthers(GameClient* client, Packet packet) {
  std::vector<ExistingPlayerPacket> existing_players;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  InputAdapter adapter(packet.data, packet.length);
  auto deserializer = bitsery::Deserializer<InputAdapter>(std::move(adapter));
  deserializer.container(existing_players, 400);

  for (const auto& existing_player : existing_players) {
    SPDLOG_INFO("ExistingPlayerPacket packet: {}", existing_player);

    CPlayer* newhero = new CPlayer();
    newhero->enable = FALSE;
    newhero->id = existing_player.player_id;
    oCNpc* npc = oCObjectFactory::GetFactory()->CreateNpc(oCNpc::GetHero()->GetInstance());
    newhero->SetNpc(npc);
    newhero->char_class = existing_player.selected_class;
    client->classmgr->EquipNPC(existing_player.selected_class, newhero, true);
    newhero->npc->SetGuild(9);
    newhero->hp = static_cast<short>(newhero->GetHealth());
    newhero->SetPosition(zVEC3(existing_player.position.x, existing_player.position.y, existing_player.position.z));
    if (newhero->Type == CPlayer::NPC_HUMAN) {
      newhero->SetAppearance(existing_player.head_model, existing_player.skin_texture, existing_player.face_texture);
    }
    if (newhero->Type > CPlayer::NPC_DRACONIAN || newhero->Type == CPlayer::NPC_HUMAN) {
      newhero->npc->ApplyOverlay(CPlayer::GetWalkStyleFromByte(existing_player.walk_style));
    }
    newhero->SetName(existing_player.player_name.c_str());
    newhero->update_hp_packet = 0;
    client->player.push_back(newhero);
  }
}

void OnJoinGame(GameClient* client, Packet packet) {
  JoinGamePacket joinGamePacket;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({packet.data, packet.length}, joinGamePacket);

  if (!joinGamePacket.player_id) {
    SPDLOG_ERROR("Invalid JoinGame packet. No player id.");
    return;
  }

  SPDLOG_INFO("JoinGame packet: {}", joinGamePacket);

  CPlayer* newhero = new CPlayer();
  newhero->id = *joinGamePacket.player_id;
  zVEC3 pos(joinGamePacket.position.x, joinGamePacket.position.y, joinGamePacket.position.z);
  oCNpc* npc = oCObjectFactory::GetFactory()->CreateNpc(oCNpc::GetHero()->GetInstance());
  newhero->SetNpc(npc);
  client->classmgr->EquipNPC(joinGamePacket.selected_class, newhero, true);
  newhero->char_class = joinGamePacket.selected_class;
  newhero->npc->SetGuild(9);
  newhero->hp = static_cast<short>(newhero->GetHealth());
  newhero->SetPosition(pos);
  newhero->SetName(joinGamePacket.player_name.c_str());
  npc->SetName(joinGamePacket.player_name.c_str());
  if (newhero->Type == CPlayer::NPC_HUMAN)
    newhero->SetAppearance(joinGamePacket.head_model, joinGamePacket.skin_texture, joinGamePacket.face_texture);
  if (newhero->Type > CPlayer::NPC_DRACONIAN || newhero->Type == CPlayer::NPC_HUMAN)
    newhero->npc->ApplyOverlay(CPlayer::GetWalkStyleFromByte(joinGamePacket.walk_style));
  CChat::GetInstance()->WriteMessage(NORMAL, false, zCOLOR(0, 255, 0, 255), "%s%s", joinGamePacket.player_name.c_str(),
                                     (*client->lang)[CLanguage::SOMEONE_JOIN_GAME].ToChar());
  newhero->enable = FALSE;
  newhero->update_hp_packet = 0;
  // kod
  client->player.push_back(newhero);
}

void OnGameInfo(GameClient* client, Packet packet) {
  GameInfoPacket gameInfoPacket;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({packet.data, packet.length}, gameInfoPacket);

  STime t;
  t.time = static_cast<int>(gameInfoPacket.raw_game_time);
  oCGame::GetGame()->SetTime(static_cast<int>(t.day), static_cast<int>(t.hour), static_cast<int>(t.min));

  if (!client->IgnoreFirstTimeMessage) {
    CChat::GetInstance()->WriteMessage(NORMAL, false, "Time set to: %d:%.2d", t.hour, t.min);
  }
  client->IgnoreFirstTimeMessage = false;
  client->game_mode = gameInfoPacket.game_mode;
  UsePotionKeys = gameInfoPacket.flags & 0x01;
  client->DropItemsAllowed = gameInfoPacket.flags & 0x02;
  client->ForceHideMap = gameInfoPacket.flags & 0x04;
}

void OnLeftGame(GameClient* client, Packet packet) {
  DisconnectionInfoPacket disconnectionInfoPacket;
  using InputAdapter = bitsery::InputBufferAdapter<unsigned char*>;
  auto state = bitsery::quickDeserialization<InputAdapter>({packet.data, packet.length}, disconnectionInfoPacket);

  for (size_t i = 1; i < client->player.size(); i++) {
    if (client->player[i]->id == disconnectionInfoPacket.disconnected_id) {
      CChat::GetInstance()->WriteMessage(NORMAL, false, zCOLOR(255, 0, 0, 255), "%s%s", client->player[i]->GetName(),
                                         (*client->lang)[CLanguage::SOMEONEDISCONNECT_FROM_SERVER].ToChar());
      client->player[i]->LeaveGame();
      delete client->player[i];
      client->player.erase(client->player.begin() + i);
      break;
    }
  }
}

void OnVoice(GameClient* client, Packet packet) {
  int size;
  memcpy(&size, packet.data + 1, 4);
  char* buffer = new char[size];
  memcpy(buffer, packet.data + 5, size);
  client->voicePlayback->PlayVoice(buffer, size);
}
}  // namespace Game