#include "Script.h"
#include "sol/sol.hpp"
#include <spdlog/spdlog.h>
//Binds
#include "LuaSpdlogFuncs.h"

using namespace std;
const string directory = "scripts";

int script_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception,
                         sol::string_view description) {
  SPDLOG_ERROR("An exception occurred in a function, here's what it says ");
  if (maybe_exception) {
    SPDLOG_ERROR("(straight from the exception): ");
    const std::exception& ex = *maybe_exception;
    SPDLOG_ERROR(ex.what());
  } else {
    SPDLOG_ERROR("(from the description parameter): ");
    SPDLOG_ERROR("{}", description.data());
  }
  return sol::stack::push(L, description);
}

inline void script_panic(sol::optional<std::string> maybe_msg) {
  SPDLOG_CRITICAL("Lua is in a panic state and will now abort() the application");
  if (maybe_msg) {
    const std::string& msg = maybe_msg.value();
    SPDLOG_CRITICAL("error message: {}", msg);
  }
}


Script::Script(vector<string> scripts)
{
  Init();
  LoadScripts(scripts);
}

Script::~Script()
{

}

void Script::Init()
{
  lua.open_libraries();
  lua.set_exception_handler(&script_exception_handler);
  lua.set_panic(sol::c_call<decltype(&script_panic), &script_panic>);
  BindFunctionsAndVariables();
}

void Script::BindFunctionsAndVariables()
{
  LuaSpdlogFuncs::Bind(lua);
}

void Script::LoadScripts(vector<string> scripts) 
{
  for_each(scripts.begin(), scripts.end(), bind(&Script::LoadScript, this, placeholders::_1));
}

void Script::LoadScript(string script) 
{
  try {
    auto result = lua.safe_script_file(directory + "/" + script);
    SPDLOG_INFO("Lua script: {} has been loaded!", script);
  } catch (const sol::error& e) {
    SPDLOG_ERROR("Lua script: {} cannot be loaded!", script);
    SPDLOG_ERROR(e.what());
  }
}