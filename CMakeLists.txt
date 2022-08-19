
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

cmake_minimum_required(VERSION 3.18)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_SOURCE_DIR}/triplets")

project(GothicMultiplayer VERSION 0.2 DESCRIPTION "Gothic Multiplayer" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_path(CPP_HTTPLIB_INCLUDE_DIRS "httplib.h")
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory(common)
add_subdirectory(Shared)
add_subdirectory(RakNet)
add_subdirectory(GMP_Serv)

if (WIN32)
    find_library(InjectMage InjectMage.lib PATHS ./InjectMage)
    find_path(
        InjectMage_INCLUDE_DIR
        NAMES "InjectMage.h"
        PATHS "./InjectMage"
    )

    find_path(
        DX7SDK_INCLUDE_DIR
        NAMES "d3d.h" "ddraw.h" "dinput.h"
        PATHS "./GMP_Client/dx7sdk/inc"
        REQUIRED
    )

    find_library(BugTrap BugTrapU.lib PATHS ./BugTrap)
    find_path(
        BugTrap_INCLUDE_DIR
        NAMES "BugTrap.h"
        PATHS "./BugTrap"
    )

    add_subdirectory(g2api)
    add_subdirectory(GMP_Client)
endif()