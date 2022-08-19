
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

set (SERVER_SOURCES
  CClasses.cpp
  config.cpp        
  CGenericFile.cpp   
  CGmpServ.cpp       
  CLog.cpp
  CPermissions.cpp   
  crypt.cpp
  CSpawnList.cpp     
  CWBFile.cpp        
  GMP_Serv.cpp       
  platform_depend.cpp
  zBitStream.cpp
)

add_executable(GMP_Serv ${SERVER_SOURCES})
target_link_libraries(GMP_Serv PRIVATE RakNet SharedLib spdlog::spdlog fmt::fmt)
target_include_directories(GMP_Serv PRIVATE ${CPP_HTTPLIB_INCLUDE_DIRS})