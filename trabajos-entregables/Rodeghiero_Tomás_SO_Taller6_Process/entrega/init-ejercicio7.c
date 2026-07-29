#include "edoslib.h"

int main(void)
{
    printf("EDOS init process: pid=%d!\n", getpid());

    // Ejercicio 7: provocar un page fault desde modo usuario.
    // PROC_MAX_VA = 0x80000000. Solo estan mapeadas las paginas de codigo/datos
    // (desde va=0) y la pagina del stack en PROC_MAX_VA - PAGE_SIZE. Escribir
    // en 0xDEADBEEF cae fuera de toda pagina mapeada del proceso, por lo que
    // debe disparar un STORE_PAGE_FAULT que el kernel mata el proceso.
    printf("Intentando escribir en una direccion no mapeada (0xdeadbeef)...\n");
    volatile int *bad = (int *) 0xdeadbeef;
    *bad = 0x42;

    console_puts("Esta linea no deberia ejecutarse.\n");
    return 0;
}
