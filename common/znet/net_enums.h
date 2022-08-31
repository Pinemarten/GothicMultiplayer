
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

namespace Net
{
enum PacketReliability
{
	UNRELIABLE,
	UNRELIABLE_SEQUENCED,
	RELIABLE,
	RELIABLE_ORDERED,
	RELIABLE_SEQUENCED
};

enum PacketPriority
{
  IMMEDIATE_PRIORITY,
  HIGH_PRIORITY,
  MEDIUM_PRIORITY,
  LOW_PRIORITY,
};

enum PacketID
{
  WL_PREPARE_TO_JOIN,
  WL_JOIN_TO_GAME,
  WL_INGAME,
  ID_CONNECTION_ATTEMPT_FAILED = 17,
  ID_ALREADY_CONNECTED,
  ID_NEW_INCOMING_CONNECTION,
  ID_NO_FREE_INCOMING_CONNECTIONS,
  ID_DISCONNECTION_NOTIFICATION,
	ID_CONNECTION_LOST,
  ID_CONNECTION_BANNED,
  ID_INVALID_PASSWORD,
  ID_INCOMPATIBLE_PROTOCOL_VERSION,
  PT_MSG = 135,
  PT_NEWCONN,  // <- u�ywa kto� tego?
  PT_REQUEST_FILE_LENGTH,
  PT_REQUEST_FILE_PART,
  PT_WHOAMI,
  PT_JOIN_GAME,
  PT_ACTUAL_STATISTICS,  // <- chyba tutaj będe musiał dodac optymalizacje gdyż nie potrzeba nam wszystkich informacji o
                         // graczu który jest od nas dalej niż 5000.0f
  PT_ALL_OTHERS,  // pakiety wysy�any tylko z serwera do klienta, informacje o wszystkich graj�cych wysy�ane jednorazowo
  PT_HP_DIFF,
  PT_MAP_ONLY,  // wysy�a tylko informacje o x,y graczy �eby mapka dzia�a�a
  PT_CHECKSUM,  // pakiet sprawdzaj�cy czy crc32 serwera i klienta s� zgodne
  PT_RCON,      // administrowanie
  PT_WHISPER,
  PT_EXTENDED_4_SCRIPTS,  // jak ju� kiedy� wdro�ymy skrypty
  PT_SRVMSG,
  PT_YOUR_NAME,
  PT_LEFT_GAME,
  PT_GAME_INFO,
  PT_MAP_NAME,
  PT_DODIE,
  PT_RESPAWN,
  PT_DROPITEM,
  PT_TAKEITEM,
  PT_CASTSPELL,
  PT_CASTSPELLONTARGET,
  PT_VOICE,
};
}