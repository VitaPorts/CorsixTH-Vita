/* LuaJIT's static archive references POSIX dl functions even when CorsixTH
 * loads its C modules through package.preload. Vita M2.1 intentionally does
 * not load external native modules or LuaJIT FFI libraries.
 */
#ifndef __vita__
#error This compatibility boundary is only for Vita targets
#endif

#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>

/* dlerror consumes a pending error. Per-thread state avoids one Lua/SDL
 * thread consuming an error raised by another. No allocation or handles. */
static _Thread_local int pending_error;
static char unsupported_message[] =
    "Vita: dynamic native modules and LuaJIT FFI loading are unsupported";

static void deny_dynamic_load(void)
{
  errno = ENOSYS;
  pending_error = 1;
}

void *dlopen(const char *path, int mode)
{
  (void) path;
  (void) mode;
  deny_dynamic_load();
  return NULL;
}

void *dlsym(void *handle, const char *symbol)
{
  (void) handle;
  (void) symbol;
  deny_dynamic_load();
  return NULL;
}

int dlclose(void *handle)
{
  (void) handle;
  deny_dynamic_load();
  return -1;
}

char *dlerror(void)
{
  if (!pending_error)
    return NULL;
  pending_error = 0;
  return unsupported_message;
}
