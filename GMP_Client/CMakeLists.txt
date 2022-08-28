
# MIT License

# Copyright (c) 2022 Gothic Multiplayer Team (pampi, skejt23, mecio)

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# For git metadata in C++.
include(FetchContent)
FetchContent_Declare(cmake_git_version_tracking                   
  GIT_REPOSITORY https://github.com/andrew-hardin/cmake-git-version-tracking.git
  GIT_TAG 904dbda1336ba4b9a1415a68d5f203f576b696bb
)
FetchContent_MakeAvailable(cmake_git_version_tracking)

set (CLIENT_SOURCES
  Button.cpp
  CActiveAniID.cpp   
  CAnimMenu.cpp      
  CChat.cpp
  CConfig.cpp        
  CFollowNpc.cpp     
  CGmpClient.cpp     
  CHeroClass.cpp     
  CIngame.cpp        
  CInterpolatePos.cpp
  CInventory.cpp     
  CLanguage.cpp      
  CLocalPlayer.cpp   
  CMainMenu.cpp      
  CMap.cpp
  CMenu.cpp
  CObjectMenu.cpp    
  CObservation.cpp   
  CPlayer.cpp        
  CPlayerList.cpp    
  CSanta.cpp
  CSelectClass.cpp   
  CServerList.cpp    
  CShrinker.cpp      
  CSpawnPoint.cpp
  CSyncFuncs.cpp
  CUpdate.cpp
  CWatch.cpp
  ExceptionHandler.cpp
  ExtendedServerList.cpp
  Font.cpp
  GMPSplash.cpp
  GMP_Client.cpp
  HooksManager.cpp
  Interface.cpp
  keyboard.cpp
  Mod.cpp
  Network.cpp
  HTTPDownloader.cpp
  patch.cpp
  SharedUtil.cpp
  Table.cpp
  WorldBuilder/CBuilder.cpp
  WorldBuilder/load.cpp
  WorldBuilder/save.cpp
)

add_library(GMP SHARED ${CLIENT_SOURCES})
target_link_libraries(GMP PRIVATE common g2api ${InjectMage} RakNet ${BugTrap} SharedLib wsock32 ws2_32 Iphlpapi httplib spdlog::spdlog cmake_git_version_tracking SDL2::SDL2-static pugixml)
target_include_directories(GMP PRIVATE ${InjectMage_INCLUDE_DIR} ${BugTrap_INCLUDE_DIR} ${CPP_HTTPLIB_INCLUDE_DIRS})

add_subdirectory(g2api)