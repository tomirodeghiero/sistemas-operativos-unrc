#include "edoslib.h"

int main(void)
{
    printf("EDOS init process: pid=%d!\n", getpid());

    // Ejercicio 9: usar el syscall time(). Imprimo el contador de ticks varias
    // veces, durmiendo entre lecturas para que el valor cambie.
    for (int i = 0; i < 5; i++) {
        printf("init: time = %d ticks\n", time());
        sleep(2);
    }

    printf("init: time final = %d ticks. Finishing...\n", time());
    return 0;
}
