#include <dlfcn.h>
int main(void) {
  void *handle = dlopen("app0:/missing.suprx", RTLD_NOW);
  if (handle) { (void)dlsym(handle, "missing"); (void)dlclose(handle); }
  return dlerror() != 0;
}
