#include "LuaSpdlogFuncs.h"
#include <spdlog/spdlog.h>

void spdlog_error(std::string text) {
  SPDLOG_ERROR(text);
}

void spdlog_info(std::string text) {
  SPDLOG_INFO(text);
}

void spdlog_debug(std::string text) {
  SPDLOG_DEBUG(text);
}

void spdlog_critical(std::string text) {
  SPDLOG_CRITICAL(text);
}

void spdlog_warn(std::string text) {
  SPDLOG_WARN(text);
}

void LuaSpdlogFuncs::Bind(sol::state& lua) {
  lua["SPDLOG_ERROR"] = spdlog_error;
  lua["SPDLOG_INFO"] = spdlog_info;
  lua["SPDLOG_DEBUG"] = spdlog_debug;
  lua["SPDLOG_CRITICAL"] = spdlog_critical;
  lua["SPDLOG_WARN"] = spdlog_warn;
}