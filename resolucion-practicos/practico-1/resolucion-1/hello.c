#include <stdio.h>

int main(int argc, char *argv[]) {
    /* argc indica cuántos argumentos recibió el programa. */
    printf("argc = %d\n", argc);

    /* argv es un arreglo de strings: argv[0] es el nombre del programa. */
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    /* Retorno 0 = ejecución exitosa. */
    return 0;
}
