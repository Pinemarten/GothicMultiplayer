
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

#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <sstream>

#include "game_server.h"
#include "chillout.h"

namespace {
void SetupCrashHandler() {
  static std::stringstream ss;
}
}  // namespace

int main(int argc, char **argv) {
  SetupCrashHandler();
  srand(static_cast<unsigned int>(time(NULL)));

  GameServer serv;
  if (!serv.Init()) {
    SPDLOG_ERROR("Server initialization failed!");
    exit(0);
  }
  while (1) {
    using namespace std::chrono_literals;
    serv.Run();
    serv.SendSpamMessage();
    std::this_thread::sleep_for(1ms);
  }
  return 0;
}
