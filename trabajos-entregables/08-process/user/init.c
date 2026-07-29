#include "edoslib.h"

int main(void)
{
    printf("EDOS init process: pid=%d!\n", getpid());

    printf("ticks desde el boot: %d\n", time());

    console_puts("Init going to sleep for 4 ticks...\n");
    sleep(4);
    console_puts("init awake! Finishing...\n");

    printf("ticks despues del sleep: %d\n", time());

    // Ejercicio 7: para provocar un page fault descomentar esta linea.
    // Escribe en una direccion no mapeada del espacio del proceso.
    // int *p = (int *) 0x40000000; *p = 42;

    return 0;
}
