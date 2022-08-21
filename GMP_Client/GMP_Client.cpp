
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

#include <io.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <iostream>

#include "CLog.h"
#include "HooksManager.h"
#include "Mod.h"
#include "common.h"
#include "ocgame.hpp"
#include "patch.h"
#include "version.h"

DWORD IdWatku;
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  DisableThreadLibraryCalls(hinstDLL);
  if (fdwReason == DLL_PROCESS_ATTACH)
  {
    spdlog::default_logger()->sinks().push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("GMP_Log.txt", false));
		spdlog::flush_on(spdlog::level::debug);
    AllocConsole();
    CLog::GetInstance()->Write(LV_INFORMATION, "Initialize GMP.");
    Patch::SetWndName("Gothic Multiplayer");
    Patch::SetVersion(GMP_VERSION);
    Patch::InitNewSplash();
    Patch::DisableStartupScript();
    Patch::DisableAbnormalExit();
    Patch::AlwaysNoMenu();
    oCGame::InstallPatches();
    HooksManager* hm = HooksManager::GetInstance();
    hm->AddHook(HT_AIMOVING, (DWORD)Initialize, false);
    CreateThread(NULL, 0, CheckForBadApps, 0, 0, &IdWatku);
    Patch::ChangeDefaultIni();
  }
  return TRUE;
}
