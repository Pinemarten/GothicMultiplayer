
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

#include "HooksManager.h"
#include "Mod.h"
#include "common.h"
#include "ocgame.hpp"
#include "patch.h"
#include "Network.h"
#include "zcoption.hpp"

#include <SDL.h>
#include <SDL_syswm.h> 

DWORD IdWatku;
SDL_Window* g_pSdlWindow;

#define hInstApp *(HINSTANCE*)(0x008D4220)
#define VideoH *(int*)(0x008D2BE0)
#define VideoW *(int*)(0x008D2BE4)

void HookwinResizeMainWindow()
{
  SDL_SetWindowSize(g_pSdlWindow, VideoW, VideoH);
}

HWND HookCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y,
                         int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
  zCOption options;
  auto loaded = options.Load("GMP.INI");
  auto windowed = options.ReadBool(zOPT_SEC_VIDEO, "zStartupWindowed", FALSE);
  if (!windowed)
  {
    return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu,
                           hInstance, lpParam);
  }

  uint32_t flags = 0;
  flags |= SDL_WINDOW_HIDDEN;
  flags |= SDL_WINDOW_RESIZABLE;
  g_pSdlWindow = SDL_CreateWindow(lpWindowName, X, Y + 30, 800, 600, flags);
  if (g_pSdlWindow == nullptr)
  {
    SPDLOG_ERROR("Unable to create SDL window. Fallback to CreateWindowExA.");
    return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu,
                           hInstance, lpParam);
  }

  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  if (!SDL_GetWindowWMInfo(g_pSdlWindow, &wmInfo))
  {
    SDL_DestroyWindow(g_pSdlWindow);
    SPDLOG_ERROR("Unable to get window WM info. Fallback to CreateWindowExA.");
    return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu,
                           hInstance, lpParam);
  }

  SDL_SetRelativeMouseMode(SDL_TRUE);
  hInstApp = wmInfo.info.win.hinstance;
  return wmInfo.info.win.window;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  DisableThreadLibraryCalls(hinstDLL);
  if (fdwReason == DLL_PROCESS_ATTACH)
  {
    AllocConsole();
    spdlog::default_logger()->sinks().push_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("GMP_Log.txt", false));
		spdlog::flush_on(spdlog::level::debug);

    Network::LoadNetworkLibrary();

   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
      SPDLOG_ERROR("Couldn't initialize SDL: {}", SDL_GetError());
   }

    // Window hook
    CallPatch(0x0050323F, (DWORD)&HookCreateWindowExA, 1);
    CallPatch(0x004FD794, (DWORD)&HookwinResizeMainWindow, 0);
    CallPatch(0x004FE096, (DWORD)&HookwinResizeMainWindow, 0);

    Patch::SetWndName("Gothic Multiplayer");

    // This needs to be unique, so it's possible to run multiple instances of the game.
    static std::string reg_program_name = "GMP " + std::to_string(GetCurrentProcessId());
    Patch::SetRegProgram(reg_program_name.c_str());
    
    Patch::InitNewSplash();
    Patch::DisablePlayBink();
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
