#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *endptr = NULL;
    long value;

    /* Se espera exactamente 1 argumento: el código de salida deseado. */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <0-255>\n", argv[0]);
        return 2;
    }

    /* strtol permite validar errores de conversión numérica. */
    errno = 0;
    value = strtol(argv[1], &endptr, 10);

    /* Error si no es entero puro (ej: "12x", "abc", etc.). */
    if (errno != 0 || endptr == argv[1] || *endptr != '\0') {
        fprintf(stderr, "Error: el argumento debe ser un entero.\n");
        return 2;
    }

    /* El shell maneja exit status en rango 0..255. */
    if (value < 0 || value > 255) {
        fprintf(stderr, "Error: el exit status valido esta entre 0 y 255.\n");
        return 2;
    }

    /* Se devuelve exactamente el código pedido. */
    return (int)value;
}
