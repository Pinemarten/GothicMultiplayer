
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

#include <atomic>
#include <cstdint>
#include <fmt/ostream.h>
#include <memory>
#include <mutex>
#include <thread>
#include <toml.hpp>

// Class responsible for counting Gothic time on the server side.
class GothicClock
{
public:
  struct Time
  {
    std::uint16_t day_ = 1;
    std::uint8_t hour_ = 8;
    std::uint8_t min_ = 0;

    void from_toml(const toml::value& v);
  };

  GothicClock(Time initial_time);
  ~GothicClock();

  void UpdateTime(Time new_time);
  Time GetTime() const;

private:
  void RunClockLoop();

  Time time_;
  mutable std::mutex time_mutex_;
  std::atomic_bool run_clock_thread_{true};
  std::thread thread_;
};

std::ostream& operator<<(std::ostream& os, const GothicClock::Time& d);
template <> struct fmt::formatter<GothicClock::Time> : ostream_formatter
{
};