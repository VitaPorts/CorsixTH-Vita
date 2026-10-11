#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *name)
{
  if (!condition) {
    fprintf(stderr, "FAIL %s\n", name);
    ++failures;
  } else {
    printf("PASS %s\n", name);
  }
}

static void check_error(const char *name)
{
  char *message = dlerror();
  check(message && strstr(message, "unsupported") != NULL, name);
  check(dlerror() == NULL, "dlerror consumes pending failure");
}

int main(void)
{
  check(dlerror() == NULL, "initial dlerror clear");
  errno = 0;
  check(dlopen("app0:/missing.suprx", 0) == NULL && errno == ENOSYS,
        "dlopen path denied with ENOSYS");
  check_error("dlopen diagnostic");

  errno = 0;
  check(dlopen(NULL, 0) == NULL && errno == ENOSYS,
        "dlopen default FFI scope denied");
  check_error("default scope diagnostic");

  errno = 0;
  check(dlsym((void *) 1, "foo") == NULL && errno == ENOSYS,
        "dlsym denied with ENOSYS");
  check_error("dlsym diagnostic");

  errno = 0;
  check(dlclose((void *) 1) == -1 && errno == ENOSYS,
        "dlclose never reports success");
  check_error("dlclose diagnostic");

  printf("dynamic_loader_contract failures=%d\n", failures);
  return failures ? 1 : 0;
}
