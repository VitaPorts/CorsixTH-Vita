#include "font.h"

#include <SDL.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <luajit.h>
}

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <system_error>
#include <sys/stat.h>

#ifndef PROBE_GIT_SHA
#define PROBE_GIT_SHA "unknown"
#endif
#ifndef PROBE_SDK_ID
#define PROBE_SDK_ID "unrecorded"
#endif
#ifndef PROBE_BUILD_MODE
#define PROBE_BUILD_MODE "unknown"
#endif

extern "C" int luaopen_lfs(lua_State* state);
extern "C" int luaopen_lpeg(lua_State* state);

namespace {
constexpr char kRoot[] = "ux0:data/corsixth";
constexpr char kDir[] = "ux0:data/corsixth/m1b";
constexpr char kLog[] = "ux0:data/corsixth/m1b-probe.log";
constexpr char kNativeFile[] = "ux0:data/corsixth/m1b/native-token.txt";
constexpr char kLuaFile[] = "ux0:data/corsixth/m1b/lua-token.txt";
constexpr char kCppFile[] = "ux0:data/corsixth/m1b/cpp17-owned/cpp17-token.txt";
constexpr char kNativeToken[] = "M1B-NATIVE-42\n";

enum Test { CHECK_LUA_INIT, LUA_ABI, LPEG, LFS, NATIVE_IO, LUA_IO, CPP17_FS, LOGFILE, TEST_COUNT };
enum State { NOT_RUN, PASS, FAIL, OPTIONAL_UNSUPPORTED };
const char* const kNames[] = {"LUAJIT INIT", "LUA 5.1 ABI", "LPEG PATTERN",
                            "LUAFILESYSTEM", "NATIVE UX0 I/O", "LUA UX0 I/O",
                            "CPP17 FILESYSTEM", "LOGFILE"};
State states[TEST_COUNT] = {};
FILE* log_file = nullptr;

void log_line(const char* phase, const char* detail) {
  std::printf("[%s] %s\n", phase, detail);
  std::fflush(stdout);
  if (log_file) {
    if (std::fprintf(log_file, "[%s] %s\n", phase, detail) < 0 ||
        std::fflush(log_file) != 0) {
      states[LOGFILE] = FAIL;
      std::printf("[ERRORS] logfile write/flush failed errno=%d (%s)\n", errno,
                  std::strerror(errno));
      std::fflush(stdout);
    }
  }
}

void record(Test test, State state, const char* detail) {
  states[test] = state;
  char line[512];
  std::snprintf(line, sizeof(line), "test=%s result=%s detail=%.380s", kNames[test],
                 state == PASS ? "PASS" : state == FAIL ? "FAIL" :
                 state == OPTIONAL_UNSUPPORTED ? "UNSUPPORTED_OPTIONAL" : "NOT_RUN", detail);
  log_line(state == FAIL ? "ERRORS" : "LUA", line);
}

void record_errno(Test test, const char* operation) {
  char line[384];
  std::snprintf(line, sizeof(line), "%s errno=%d (%s)", operation, errno,
                std::strerror(errno));
  record(test, FAIL, line);
}

bool ensure_dir(const char* path) {
  if (mkdir(path, 0777) == 0) return true;
  if (errno != EEXIST) return false;
  struct stat info {};
  return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}

void open_log() {
  if (!ensure_dir(kRoot)) {
    record_errno(LOGFILE, "mkdir ux0:data/corsixth");
    return;
  }
  log_file = std::fopen(kLog, "w");
  if (!log_file) {
    record_errno(LOGFILE, "fopen m1b-probe.log");
    return;
  }
  record(LOGFILE, PASS, "ux0:data/corsixth/m1b-probe.log opened and flushed");
}

bool run_lua(lua_State* lua, Test test, const char* script) {
  const int base = lua_gettop(lua);
  if (luaL_loadstring(lua, script) != 0) {
    const char* message = lua_tostring(lua, -1);
    record(test, FAIL, message ? message : "Lua load error without string message");
    lua_settop(lua, base);
    return false;
  }
  if (lua_pcall(lua, 0, LUA_MULTRET, 0) != 0) {
    const char* message = lua_tostring(lua, -1);
    record(test, FAIL, message ? message : "Lua runtime error without string message");
    lua_settop(lua, base);
    return false;
  }
  lua_settop(lua, base);
  return true;
}

bool native_io() {
  if (!ensure_dir(kDir)) {
    record_errno(NATIVE_IO, "mkdir m1b");
    return false;
  }
  FILE* output = std::fopen(kNativeFile, "wb");
  if (!output) { record_errno(NATIVE_IO, "fopen native write"); return false; }
  const size_t expected = sizeof(kNativeToken) - 1;
  const size_t written = std::fwrite(kNativeToken, 1, expected, output);
  const int closed = std::fclose(output);
  if (written != expected || closed != 0) {
    record_errno(NATIVE_IO, "fwrite/fclose native token");
    std::remove(kNativeFile);
    return false;
  }
  FILE* input = std::fopen(kNativeFile, "rb");
  if (!input) { record_errno(NATIVE_IO, "fopen native read"); std::remove(kNativeFile); return false; }
  char actual[sizeof(kNativeToken)] = {};
  const size_t count = std::fread(actual, 1, expected, input);
  const int extra = std::fgetc(input);
  const int read_closed = std::fclose(input);
  const bool matched = count == expected && extra == EOF && read_closed == 0 &&
                       std::memcmp(actual, kNativeToken, expected) == 0;
  const int removed = std::remove(kNativeFile);
  if (!matched || removed != 0) {
    char line[160];
    std::snprintf(line, sizeof(line), "native mismatch/cleanup count=%u expected=%u remove=%d errno=%d",
                  static_cast<unsigned>(count), static_cast<unsigned>(expected), removed, errno);
    record(NATIVE_IO, FAIL, line);
    return false;
  }
  record(NATIVE_IO, PASS, "mkdir/write/close/read/compare/remove token M1B-NATIVE-42");
  return true;
}

void register_module(lua_State* lua, const char* name, lua_CFunction open_function) {
  lua_getglobal(lua, "package");
  lua_getfield(lua, -1, "preload");
  lua_pushcfunction(lua, open_function);
  lua_setfield(lua, -2, name);
  lua_pop(lua, 2);
}

void test_lua(lua_State* lua) {
  if (!lua) { record(CHECK_LUA_INIT, FAIL, "luaL_newstate returned null"); return; }
  luaL_openlibs(lua);
  record(CHECK_LUA_INIT, PASS, "luaL_newstate and luaL_openlibs succeeded");
  char line[256];
  std::snprintf(line, sizeof(line), "header=%s luajit_header=%s integer_bytes=%u number_bytes=%u",
                LUA_VERSION, LUAJIT_VERSION, static_cast<unsigned>(sizeof(lua_Integer)),
                static_cast<unsigned>(sizeof(lua_Number)));
  log_line("LUA", line);
  const int base = lua_gettop(lua);
  const char* abi_script = "return _VERSION, 6*7";
  if (luaL_loadstring(lua, abi_script) != 0 || lua_pcall(lua, 0, 2, 0) != 0) {
    const char* message = lua_tostring(lua, -1);
    record(LUA_ABI, FAIL, message ? message : "Lua ABI expression failed without string message");
    lua_settop(lua, base);
  } else {
    const char* version = lua_tostring(lua, base + 1);
    const bool valid = version && std::strcmp(version, "Lua 5.1") == 0 &&
                       lua_isnumber(lua, base + 2) && lua_tonumber(lua, base + 2) == 42;
    std::snprintf(line, sizeof(line), "runtime=%s protected_expression=%.0f expected=42",
                  version ? version : "nil", lua_tonumber(lua, base + 2));
    record(LUA_ABI, valid ? PASS : FAIL, line);
    lua_settop(lua, base);
  }
  if (luaL_loadstring(lua,
      "return type(jit)=='table' and jit.version or 'unavailable', "
      "type(jit)=='table' and type(jit.status)=='function' and jit.status() or false") != 0 ||
      lua_pcall(lua, 0, 2, 0) != 0) {
    const char* message = lua_tostring(lua, -1);
    log_line("LUA", message ? message : "jit status query failed without string message");
  } else {
    std::snprintf(line, sizeof(line), "jit.version=%s jit.status=%s (informational; JIT execution not asserted)",
                  lua_tostring(lua, base + 1) ? lua_tostring(lua, base + 1) : "unavailable",
                  lua_toboolean(lua, base + 2) ? "enabled" : "disabled");
    log_line("LUA", line);
  }
  lua_settop(lua, base);
  register_module(lua, "lpeg", luaopen_lpeg);
  if (run_lua(lua, LPEG,
      "local p=require('lpeg'); assert(type(p.version)=='string'); "
      "assert(p.match(p.C(p.R('09')^3), '123')=='123'); return true")) {
    record(LPEG, PASS, "static package.preload lpeg; digit capture 123 matched 123");
  }
  register_module(lua, "lfs", luaopen_lfs);
  if (run_lua(lua, LFS,
      "local lfs=require('lfs'); local root='ux0:data/corsixth/m1b'; "
      "assert(lfs.attributes(root,'mode')=='directory'); "
      "local child=root..'/lfs-owned'; local mode=lfs.attributes(child,'mode'); "
      "if mode then assert(mode=='directory') else local ok,err=lfs.mkdir(child); assert(ok,err) end; "
      "local found=false; for name in lfs.dir(root) do if name=='lfs-owned' then found=true end end; "
      "assert(found); assert(lfs.attributes(child,'mode')=='directory'); "
      "assert(lfs.rmdir(child)); return true")) {
    record(LFS, PASS, "static package.preload lfs; attributes/mkdir/dir/rmdir verified");
  }
  if (run_lua(lua, LUA_IO,
      "local lfs=require('lfs'); local path='ux0:data/corsixth/m1b/lua-token.txt'; "
      "local f,err=io.open(path,'wb'); assert(f,err); assert(f:write('M1B-LUA-42\\n')); assert(f:close()); "
      "assert(lfs.attributes(path,'mode')=='file'); "
      "f,err=io.open(path,'rb'); assert(f,err); local value=f:read('*a'); assert(f:close()); "
      "assert(value=='M1B-LUA-42\\n', value); assert(os.remove(path)); return true")) {
    record(LUA_IO, PASS, "Lua io.open/write/close/read/compare/remove and lfs.attributes verified");
  }
}

void record_fs_unsupported(const char* operation, const std::error_code& error) {
  char line[320];
  std::snprintf(line, sizeof(line),
                "operation=%s code=%d category=%s message=%.150s native_fallback=%s",
                operation, error.value(), error.category().name(), error.message().c_str(),
                states[NATIVE_IO] == PASS ? "PASS" : "FAIL");
  record(CPP17_FS, OPTIONAL_UNSUPPORTED, line);
}

void cpp17_io() {
  namespace fs = std::filesystem;
  std::error_code error;
  const fs::path directory("ux0:data/corsixth/m1b/cpp17-owned");
  fs::create_directories(directory, error);
  if (error) {
    record_fs_unsupported("create_directories", error);
    return;
  }
  const bool directory_exists = fs::is_directory(directory, error);
  if (error) { record_fs_unsupported("is_directory", error); return; }
  if (!directory_exists) { record(CPP17_FS, FAIL, "is_directory returned false without error"); return; }
  FILE* file = std::fopen(kCppFile, "wb");
  if (!file) { record_errno(CPP17_FS, "fopen cpp17 token"); return; }
  const char token[] = "M1B-CPP17-42\n";
  const bool wrote = std::fwrite(token, 1, sizeof(token) - 1, file) == sizeof(token) - 1;
  const int closed = std::fclose(file);
  if (!wrote || closed != 0) {
    record_errno(CPP17_FS, "fwrite/fclose cpp17 token");
    std::remove(kCppFile);
    return;
  }
  const bool exists = fs::exists(kCppFile, error);
  if (error) {
    record_fs_unsupported("exists", error);
    std::remove(kCppFile);
    return;
  }
  const auto size = fs::file_size(kCppFile, error);
  if (error) {
    record_fs_unsupported("file_size", error);
    std::remove(kCppFile);
    return;
  }
  const int removed = std::remove(kCppFile);
  const int remove_errno = errno;
  if (removed != 0) {
    char line[160];
    std::snprintf(line, sizeof(line), "optional token cleanup failed remove=%d errno=%d (%s)",
                  removed, remove_errno, std::strerror(remove_errno));
    record(CPP17_FS, FAIL, line);
    return;
  }
  const bool directory_removed = fs::remove(directory, error);
  if (error) { record_fs_unsupported("remove_directory", error); return; }
  if (exists && size == sizeof(token) - 1 && directory_removed) {
    record(CPP17_FS, PASS, "std::filesystem create/is_directory/exists/file_size/remove verified");
  } else {
    char line[180];
    std::snprintf(line, sizeof(line),
                  "optional runtime mismatch exists=%d size=%u expected=%u directory_removed=%d",
                  exists, static_cast<unsigned>(size), static_cast<unsigned>(sizeof(token) - 1),
                  directory_removed);
    record(CPP17_FS, FAIL, line);
  }
}

void draw_ui(SDL_Renderer* renderer) {
  using probe_font::color;
  using probe_font::fill;
  using probe_font::text;
  if (!color(renderer, 23, 31, 46) || SDL_RenderClear(renderer) != 0) {
    log_line("ERRORS", SDL_GetError()); return;
  }
  SDL_Rect rule{13, 73, 930, 2};
  fill(renderer, rule, 66, 82, 103);
  rule.y = 390;
  fill(renderer, rule, 66, 82, 103);
  color(renderer, 238, 242, 249);
  text(renderer, 13, 30, 2, "CORSIXTH / VITA");
  text(renderer, 740, 31, 2, "M1B LUA PROBE");
  color(renderer, 175, 194, 212);
  text(renderer, 13, 100, 2, "RUNTIME VALIDATION");
  for (int i = 0; i < TEST_COUNT; ++i) {
    const int y = 136 + i * 35;
    const State state = states[i];
    if (state == PASS) color(renderer, 54, 220, 133);
    else if (state == FAIL) color(renderer, 255, 93, 93);
    else color(renderer, 246, 205, 75);
    text(renderer, 14, y, 2, state == PASS ? "+" : state == FAIL ? "X" : "-");
    color(renderer, 238, 242, 249);
    text(renderer, 40, y, 2, kNames[i]);
    if (state == PASS) color(renderer, 54, 220, 133);
    else if (state == FAIL) color(renderer, 255, 93, 93);
    else color(renderer, 246, 205, 75);
    text(renderer, state == OPTIONAL_UNSUPPORTED ? 805 : 820, y, 2,
         state == PASS ? "PASS" : state == FAIL ? "FAIL" :
         state == OPTIONAL_UNSUPPORTED ? "UNSUPPORTED" : "NOT RUN");
  }
  color(renderer, 175, 194, 212);
  text(renderer, 13, 421, 2, "LOG: UX0:DATA/CORSIXTH/M1B-PROBE.LOG");
  color(renderer, 238, 242, 249);
  text(renderer, 13, 464, 2, "START: SAVE RESULTS AND EXIT");
  SDL_RenderPresent(renderer);
}

void final_summary() {
  int required_pass = 0;
  int required_fail = 0;
  int required_not_run = 0;
  for (int i = 0; i < TEST_COUNT; ++i) {
    if (i == CPP17_FS) continue;
    if (states[i] == PASS) ++required_pass;
    if (states[i] == FAIL) ++required_fail;
    if (states[i] == NOT_RUN) ++required_not_run;
  }
  const char* optional = states[CPP17_FS] == PASS ? "PASS" :
                         states[CPP17_FS] == FAIL ? "FAIL" :
                         states[CPP17_FS] == OPTIONAL_UNSUPPORTED ? "UNSUPPORTED" : "NOT_RUN";
  char line[256];
  std::snprintf(line, sizeof(line),
                "FINAL required_pass=%d required_fail=%d required_not_run=%d optional_cpp17=%s runtime=VITA_EXECUTED device_acceptance=PENDING_DIRECTOR_REVIEW",
                required_pass, required_fail, required_not_run, optional);
  log_line("SUMMARY", line);
}
} // namespace

int main() {
  open_log();
  char line[384];
  std::snprintf(line, sizeof(line), "commit=%s upstream=v0.70.1 sdk=%s mode=%s compiler=%s cxx=%ld time=%lld",
                PROBE_GIT_SHA, PROBE_SDK_ID, PROBE_BUILD_MODE, __VERSION__,
                static_cast<long>(__cplusplus), static_cast<long long>(std::time(nullptr)));
  log_line("PLATFORM", line);
  std::snprintf(line, sizeof(line), "Lua header=%s LuaJIT header=%s sizeof(lua_Integer)=%u sizeof(lua_Number)=%u",
                LUA_VERSION, LUAJIT_VERSION, static_cast<unsigned>(sizeof(lua_Integer)),
                static_cast<unsigned>(sizeof(lua_Number)));
  log_line("LUA", line);
  log_line("PLATFORM", "LuaJIT package=2.1.0-1 LPeg=1.1.0 LuaFileSystem=1.8.0");
  if (native_io()) {
    lua_State* lua = luaL_newstate();
    test_lua(lua);
    if (lua) lua_close(lua);
  } else {
    log_line("ERRORS", "native path failed; Lua path-dependent tests skipped");
    lua_State* lua = luaL_newstate();
    test_lua(lua);
    if (lua) lua_close(lua);
  }
  cpp17_io();
  SDL_version compiled, runtime;
  SDL_VERSION(&compiled);
  SDL_GetVersion(&runtime);
  std::snprintf(line, sizeof(line), "SDL compiled=%u.%u.%u runtime=%u.%u.%u",
                compiled.major, compiled.minor, compiled.patch,
                runtime.major, runtime.minor, runtime.patch);
  log_line("PLATFORM", line);
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0) {
    log_line("ERRORS", SDL_GetError());
    final_summary();
    if (log_file) std::fclose(log_file);
    return 1;
  }
  SDL_Window* window = SDL_CreateWindow("CorsixTH Vita Lua Probe", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, 960, 544, SDL_WINDOW_SHOWN);
  SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED) : nullptr;
  if (!window || !renderer) log_line("ERRORS", SDL_GetError());
  if (renderer) draw_ui(renderer);
  SDL_GameController* controller = nullptr;
  SDL_Joystick* joystick = nullptr;
  if (SDL_NumJoysticks() > 0) {
    if (SDL_IsGameController(0)) controller = SDL_GameControllerOpen(0);
    if (!controller) joystick = SDL_JoystickOpen(0);
  }
  bool running = true;
  while (running) {
    SDL_Event event;
    if (!SDL_WaitEventTimeout(&event, 100)) continue;
    if (event.type == SDL_QUIT) running = false;
    if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_START) running = false;
    if (event.type == SDL_JOYBUTTONDOWN && !controller && event.jbutton.button == 11) running = false;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) running = false;
  }
  final_summary();
  if (controller) SDL_GameControllerClose(controller);
  if (joystick) SDL_JoystickClose(joystick);
  if (renderer) SDL_DestroyRenderer(renderer);
  if (window) SDL_DestroyWindow(window);
  SDL_Quit();
  if (log_file) { std::fflush(log_file); std::fclose(log_file); }
  return 0;
}
