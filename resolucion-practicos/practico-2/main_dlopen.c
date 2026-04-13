/* main_dlopen.c */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef char *(*hello_fn_t)(void);

int main(void)
{
    const char *lib_path = "./libhello.dylib";
    void *handle = dlopen(lib_path, RTLD_NOW);

    if (handle == NULL) {
        fprintf(stderr, "Error en dlopen(%s): %s\n", lib_path, dlerror());
        return 1;
    }

    dlerror();

    hello_fn_t hello_fn = NULL;
    *(void **)(&hello_fn) = dlsym(handle, "hello");

    {
        const char *err = dlerror();
        if (err != NULL) {
            fprintf(stderr, "Error en dlsym(hello): %s\n", err);
            dlclose(handle);
            return 1;
        }
    }

    printf("%s\n", hello_fn());

    if (dlclose(handle) != 0) {
        fprintf(stderr, "Error en dlclose: %s\n", dlerror());
        return 1;
    }

    return 0;
}
