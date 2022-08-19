
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

#pragma once

#include <cassert>
#include <cstdint>
#include <fmt/ostream.h>
#include <ostream>
#include <string>
#include <unordered_map>
#include <variant>

#include <RakThread.h>
#include <toml.hpp>

struct STime
{
  std::uint16_t day_ = 1;
  std::uint8_t hour_ = 8;
  std::uint8_t min_ = 0;

  void from_toml(const toml::value& v)
  {
    day_ = toml::find<std::uint16_t>(v, "day");
    hour_ = toml::find<std::uint8_t>(v, "hour");
    min_ = toml::find<std::uint8_t>(v, "min");
    return;
  }

  static RAK_THREAD_DECLARATION(RunClock);

  static STime GetCurrentGothicTime();
  static void SetCurrentGothicTime(STime new_time);

  friend std::ostream& operator<<(std::ostream& os, const STime& d)
  {
    return os << d.day_ << '-' << d.hour_ << ':' << d.min_;
  }
};

template <> struct fmt::formatter<STime> : ostream_formatter
{
};

class Config
{
public:
  Config();

  template <typename T> const T& Get(const std::string& key) const
  {
    auto it = values_.find(key);
    assert(it != values_.end());
    assert(std::holds_alternative<T>(it->second));

    return std::get<T>(it->second);
  }

  void LogConfigValues() const;

protected:
  void Load();

  std::unordered_map<std::string, std::variant<std::string, std::int32_t, bool, STime>> values_;
};