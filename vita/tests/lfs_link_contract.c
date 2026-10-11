/* Host contract test of the actual vendored LFS functions with a minimal
 * Lua stack recorder. The Vita feature guard is forced on for this test. */
#include <lua.h>
#include <lauxlib.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define CORSIXTH_LFS_HOST_CONTRACT 1

struct lua_State {
  int symbolic;
  int has_target_arg;
  int nil_count;
  lua_Integer error_code;
  char message[256];
};

#include "../../third_party/luafilesystem-1.8.0/src/lfs.c"

const char *luaL_checklstring(lua_State *L, int argument, size_t *length)
{
  (void) L;
  (void) argument;
  if (length)
    *length = 5;
  return "dummy";
}

int lua_toboolean(lua_State *L, int argument)
{
  return argument == 3 ? L->symbolic : 0;
}

void lua_pushnil(lua_State *L)
{
  ++L->nil_count;
}

void lua_pushstring(lua_State *L, const char *value)
{
  (void) L;
  (void) value;
}

const char *lua_pushfstring(lua_State *L, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf(L->message, sizeof L->message, format, args);
  va_end(args);
  return L->message;
}

void lua_pushinteger(lua_State *L, lua_Integer value)
{
  L->error_code = value;
}

static int failures;

static void check_result(const char *name, int (*operation)(lua_State *),
                         int symbolic, int has_target_arg,
                         const char *message_part)
{
  lua_State state = {0};
  state.symbolic = symbolic;
  state.has_target_arg = has_target_arg;
  errno = 0;
  int result_count = operation(&state);
  if (result_count == 3 && state.nil_count == 1 &&
      state.error_code == ENOSYS && errno == ENOSYS &&
      strstr(state.message, message_part)) {
    printf("PASS %s\n", name);
  } else {
    fprintf(stderr, "FAIL %s returns=%d nil=%d code=%lld errno=%d message=%s\n",
            name, result_count, state.nil_count,
            (long long) state.error_code, errno, state.message);
    ++failures;
  }
}

int main(void)
{
  check_result("lfs.link hard", make_link, 0, 0, "hard links unsupported");
  check_result("lfs.link symbolic", make_link, 1, 0, "symbolic links unsupported");
  check_result("lfs.lock_dir", lfs_lock_dir, 0, 0, "lock_dir unsupported");
  check_result("lfs.symlinkattributes table", link_info, 0, 0,
               "symlinkattributes unsupported");
  check_result("lfs.symlinkattributes target", link_info, 0, 1,
               "symlinkattributes unsupported");
  printf("lfs_link_contract failures=%d\n", failures);
  return failures ? 1 : 0;
}
