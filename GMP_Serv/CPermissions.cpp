
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

#include "CPermissions.h"

#include <cstring>
#include <fstream>

const char* SAY = "say ";
const char* KICK = "kick ";
const char* BAN = "ban ";
const char* BAN_IP = "banip ";
const char* UNBAN = "unban ";
const char* SAVEBANS = "savebans ";
const char* MUTE = "mute ";
const char* SET_TIME = "settime ";
const char* KILL = "kill ";
const char* LOOP_MSG = "loopmsg ";
const char* HP_REGEN = "hp_regeneration ";
const char* MP_REGEN = "mp_regeneration ";

CPermissions::CPermissions() {
  SetDefaults();
  Load();
}

CPermissions::~CPermissions() {
  if (!mod_list.empty()) {
    std::list<SModerator*>::iterator i;
    for (i = mod_list.begin(); i != mod_list.end(); i++) {
      delete (*i)->nickname;
      delete (*i)->passwd;
      (*i)->nickname = 0;
      (*i)->passwd = 0;
    }
    mod_list.clear();
  }
}

void CPermissions::Load() {
  std::ifstream ifs("moderators");
  if (!ifs.fail()) {
    char* buffer = new char[256];
    char* nickname = new char[64];
    char* password = new char[128];
    do {
      memset(buffer, 0, 256);
      ifs.getline(buffer, 256);
      for (unsigned int i = 0; i <= strlen(buffer); i++)
        if (buffer[i] < 0x20)
          buffer[i] = 0;
      if ((strlen(buffer) >= 3) && (buffer[0] != '#')) {
        memset(nickname, 0, 64);
        memset(password, 0, 128);
        if (sscanf(buffer, "%s%s", nickname, password) == 2) {
          SModerator* mod = new SModerator;
          mod->nickname = new char[strlen(nickname) + 1];
          mod->passwd = new char[strlen(password) + 1];
          memcpy(mod->nickname, nickname, strlen(nickname) + 1);
          memcpy(mod->passwd, password, strlen(password) + 1);
          mod_list.push_back(mod);
          mod = 0;
        }
      }
    } while (!ifs.eof());
    memset(buffer, 0, 256);
    memset(nickname, 0, 64);
    memset(password, 0, 128);
    delete buffer;
    delete nickname;
    delete password;
    nickname = 0;
    password = 0;
    buffer = 0;
  }

  if (ifs.is_open())
    ifs.close();
  ifs.clear();
  ifs.open("permissions");
  if (!ifs.fail()) {
    char* buffer = new char[256];
    do {
      memset(buffer, 0, 256);
      ifs.getline(buffer, 256);
      if ((strlen(buffer) > 3) && (buffer[0] != '#')) {
        for (size_t i = 0; i < strlen(buffer); i++)
          if (buffer[i] < 0x20)
            buffer[i] = 0;
        if (!memcmp(buffer, SAY, strlen(SAY))) {
          sscanf(buffer + strlen(SAY), "%hd", &say);
        } else if (!memcmp(buffer, KICK, strlen(KICK))) {
          sscanf(buffer + strlen(KICK), "%hd", &kick);
        } else if (!memcmp(buffer, BAN, strlen(BAN))) {
          sscanf(buffer + strlen(BAN), "%hd", &ban);
        } else if (!memcmp(buffer, BAN_IP, strlen(BAN_IP))) {
          sscanf(buffer + strlen(BAN_IP), "%hd", &banip);
        } else if (!memcmp(buffer, UNBAN, strlen(UNBAN))) {
          sscanf(buffer + strlen(UNBAN), "%hd", &unban);
        } else if (!memcmp(buffer, SAVEBANS, strlen(SAVEBANS))) {
          sscanf(buffer + strlen(SAVEBANS), "%hd", &savebans);
        } else if (!memcmp(buffer, MUTE, strlen(MUTE))) {
          sscanf(buffer + strlen(MUTE), "%hd", &mute);
        } else if (!memcmp(buffer, SET_TIME, strlen(SET_TIME))) {
          sscanf(buffer + strlen(SET_TIME), "%hd", &settime);
        } else if (!memcmp(buffer, KILL, strlen(KILL))) {
          sscanf(buffer + strlen(KILL), "%hd", &kill);
        } else if (!memcmp(buffer, LOOP_MSG, strlen(LOOP_MSG))) {
          sscanf(buffer + strlen(LOOP_MSG), "%hd", &loopmsg);
        } else if (!memcmp(buffer, HP_REGEN, strlen(HP_REGEN))) {
          sscanf(buffer + strlen(HP_REGEN), "%hd", &hp_regen);
        } else if (!memcmp(buffer, MP_REGEN, strlen(MP_REGEN))) {
          sscanf(buffer + strlen(MP_REGEN), "%hd", &mp_regen);
        }
      }
    } while (!ifs.eof());
    delete buffer;
    buffer = 0;
  }
}

void CPermissions::SetDefaults() {
  memset(this->data, 0, 24);
}

void CPermissions::AddModerator(const char* nickname, const char* password) {
  SModerator* mod = new SModerator;
  mod->nickname = new char[strlen(nickname) + 1];
  mod->passwd = new char[strlen(password) + 1];
  memcpy(mod->nickname, nickname, strlen(nickname) + 1);
  memcpy(mod->passwd, password, strlen(password) + 1);
  mod_list.push_back(mod);
}

bool CPermissions::DeleteModerator(const char* nickname) {
  if (!mod_list.empty()) {
    std::list<SModerator*>::iterator i;
    for (i = mod_list.begin(); i != mod_list.end(); i++) {
      if (!memcmp((*i)->nickname, nickname, strlen(nickname) + 1)) {
        delete (*i)->nickname;
        delete (*i)->passwd;
        (*i)->nickname = 0;
        (*i)->passwd = 0;
        mod_list.erase(i);
        return true;
      }
    }
  }
  return false;
}

int CPermissions::IsPermitted(const char* command) {
  if (!memcmp(command, SAY, strlen(command))) {
    return (int)say;
  } else if (!memcmp(command, KICK, strlen(command))) {
    return (int)kick;
  } else if (!memcmp(command, BAN, strlen(command))) {
    return (int)ban;
  } else if (!memcmp(command, BAN_IP, strlen(command))) {
    return (int)banip;
  } else if (!memcmp(command, UNBAN, strlen(command))) {
    return (int)unban;
  } else if (!memcmp(command, SAVEBANS, strlen(command))) {
    return (int)savebans;
  } else if (!memcmp(command, MUTE, strlen(command))) {
    return (int)mute;
  } else if (!memcmp(command, SET_TIME, strlen(command))) {
    return (int)settime;
  } else if (!memcmp(command, KILL, strlen(command))) {
    return (int)kill;
  } else if (!memcmp(command, LOOP_MSG, strlen(command))) {
    return (int)loopmsg;
  } else if (!memcmp(command, HP_REGEN, strlen(command))) {
    return (int)hp_regen;
  } else if (!memcmp(command, MP_REGEN, strlen(command))) {
    return (int)mp_regen;
  }
  return 0;
}

int CPermissions::IsModerator(const char* nickname, const char* password) {
  if (!mod_list.empty()) {
    std::list<SModerator*>::iterator i;
    for (i = mod_list.begin(); i != mod_list.end(); i++) {
      if (!memcmp((*i)->nickname, nickname, strlen(nickname) + 1)) {
        if (!memcmp((*i)->passwd, password, strlen(password) + 1))
          return 1;
        else
          return -1;
      }
    }
  }
  return 0;
}

SModerator* CPermissions::GetModerator(const char* nickname) {
  if (!mod_list.empty()) {
    std::list<SModerator*>::iterator i;
    for (i = mod_list.begin(); i != mod_list.end(); i++) {
      if (!memcmp((*i)->nickname, nickname, strlen(nickname) + 1))
        return (*i);
    }
  }
  return NULL;
}

bool CPermissions::SetPassword(const char* mod_nick, const char* password) {
  return SetPassword(GetModerator(mod_nick), password);
}

bool CPermissions::SetPassword(SModerator* mod, const char* password) {
  if (!mod)
    return false;
  if (mod->passwd)
    delete mod->passwd;
  mod->passwd = new char[strlen(password) + 1];
  memcpy(mod->passwd, password, strlen(password) + 1);
  return true;
}