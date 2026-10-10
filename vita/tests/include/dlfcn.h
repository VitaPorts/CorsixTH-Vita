/* Host test declaration matching the installed VitaSDK dlfcn.h ABI. */
#ifndef CORSIXTH_TEST_DLFCN_H
#define CORSIXTH_TEST_DLFCN_H
void *dlopen(const char *path, int mode);
void *dlsym(void *handle, const char *symbol);
int dlclose(void *handle);
char *dlerror(void);
#endif
