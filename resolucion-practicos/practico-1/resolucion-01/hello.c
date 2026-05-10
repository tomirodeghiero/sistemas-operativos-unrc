/*
 * hello.c — Imprime los argumentos recibidos por la línea de comandos.
 *
 * El shell, al ejecutar `./hello a b c`, hace fork() y luego execve() en el
 * hijo. La llamada execve carga la imagen ELF y transfiere el control a la
 * función main, pasándole argc, argv y envv tal como se reciben aquí.
 */

#include <stdio.h>

int main(int argc, char *argv[]) {
    /* argc = cantidad total de argumentos (incluido argv[0] = nombre del programa). */
    printf("argc = %d\n", argc);

    /* argv es un arreglo NULL-terminated de strings. */
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    return 0; /* exit status 0 = ejecución exitosa, según convención UNIX. */
}
