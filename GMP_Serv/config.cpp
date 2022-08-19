
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

#include "Config.h"
#include "TomlWrapper.h"

#include <RakSleep.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <set>
#include <string>

namespace
{
constexpr std::uint32_t kMaxNameLength = 100;

std::unordered_map<std::string, std::variant<std::string, std::int32_t, bool, STime>>
    kDefault_Config_Values = {{"name", std::string("Gothic Multiplayer Server")},
                              {"port", 57005},
                              {"admin_passwd", std::string("")},
                              {"slots", 12},
                              {"public", false},
                              {"log_file", std::string("log.txt")},
                              {"log_mode", 0},
                              {"map", std::string("NEWWORLD\\NEWWORLD.ZEN")},
                              {"respawn_time_seconds", 5},
                              {"message_of_the_day", std::string("Welcome to the server.")},
                              {"message_of_the_day_interval_seconds", 60},
                              {"allow_modification", true},
                              {"game_mode", 0},
                              {"allow_dropitems", true},
                              {"hide_map", false},
                              {"real_chat", false},
                              {"be_unconcious_before_dead", false},
                              {"hp_regeneration", 0},
                              {"mp_regeneration", 0},
                              {"observation", false},
                              {"quick_pots", false},
                              {"map_md5", std::string("")},
                              {"log_to_stdout", true},
                              {"game_time", STime{1u, 8u, 0u}}, // 8:00
#ifndef WIN32
                              {"daemon", true}
#else
                              {"daemon", false}
#endif
};

} // namespace

STime gTime;

STime STime::GetCurrentGothicTime()
{
  return gTime;
}

void STime::SetCurrentGothicTime(STime new_time)
{
  gTime = new_time;
}

RAK_THREAD_DECLARATION(STime::RunClock)
{
  while (1)
  {
    RakSleep(4000);
    if (++gTime.min_ > 59)
    {
      gTime.min_ = 0;
      if (++gTime.hour_ > 23)
      {
        gTime.hour_ = 0;
        gTime.day_++;
      }
    }
  }
  return 0;
}

Config::Config()
{
  Load();
  gTime = Get<STime>("game_time");
}

void Config::Load()
{
  values_ = kDefault_Config_Values;

  TomlWrapper config;
  try
  {
    config = TomlWrapper::CreateFromFile("config.toml");
  }
  catch (const std::exception&)
  {
    return;
  }

  for (auto& value : values_)
  {
    std::visit(
        [&config, &value](auto&& arg)
        {
          using T = std::decay_t<decltype(arg)>;
          auto value_opt = config.GetValue<T>(value.first);
          if (value_opt)
          {
            value.second = *value_opt;
          }
        },
        value.second);
  }
}

void Config::LogConfigValues() const
{
  const std::set<std::string> keys_to_ignore = {"admin_passwd"};

  for (auto& value : values_)
  {
    std::visit(
        [&value, &keys_to_ignore](auto&& arg)
        {
          using T = std::decay_t<decltype(arg)>;
          if (keys_to_ignore.find(value.first) == keys_to_ignore.end())
          {
            if constexpr (std::is_same_v<T, std::string>)
            {
              if (arg.empty())
              {
                return;
              }
            }
            SPDLOG_INFO("{}: {}", value.first, arg);
          }
        },
        value.second);
  }
}