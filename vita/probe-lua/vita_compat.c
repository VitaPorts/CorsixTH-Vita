/* Probe-only unsupported operations. Static package.preload is the test path.
 * LuaJIT's package loader references libdl and LuaFileSystem references POSIX
 * symlinks even though this diagnostic never invokes those capabilities.
 * Return explicit unsupported errors without requiring Vita kernel plugins.
 */
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

void* dlopen(const char* path, int mode) {
  (void)path; (void)mode;
  errno = ENOSYS;
  return NULL;
}
void* dlsym(void* handle, const char* symbol) {
  (void)handle; (void)symbol;
  errno = ENOSYS;
  return NULL;
}
int dlclose(void* handle) {
  (void)handle;
  errno = ENOSYS;
  return -1;
}
char* dlerror(void) {
  return "dynamic loading unsupported in M1B static probe";
}
int symlink(const char* target, const char* link_path) {
  (void)target; (void)link_path;
  errno = ENOSYS;
  return -1;
}
ssize_t readlink(const char* path, char* buffer, size_t length) {
  (void)path; (void)buffer; (void)length;
  errno = ENOSYS;
  return -1;
}
