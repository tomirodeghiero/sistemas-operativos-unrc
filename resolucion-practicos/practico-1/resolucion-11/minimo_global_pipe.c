#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Estructura enviada por cada hijo al padre a través del pipe. */
typedef struct {
    int child_id;
    int local_min;
} ChildResult;

/* Escribe exactamente count bytes (evita escrituras parciales). */
static int write_full(int fd, const void *buf, size_t count) {
    const char *p = (const char *)buf;
    size_t done = 0;

    while (done < count) {
        ssize_t n = write(fd, p + done, count - done);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        done += (size_t)n;
    }

    return 0;
}

/* Lee exactamente count bytes (evita lecturas parciales). */
static int read_full(int fd, void *buf, size_t count) {
    char *p = (char *)buf;
    size_t done = 0;

    while (done < count) {
        ssize_t n = read(fd, p + done, count - done);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            return 1; /* EOF antes de completar */
        }
        done += (size_t)n;
    }

    return 0;
}

/* Conversión robusta de string a int con validación de rango. */
static int parse_int(const char *s, int *out) {
    char *end = NULL;
    long v;

    errno = 0;
    v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        return -1;
    }
    if (v < INT_MIN || v > INT_MAX) {
        return -1;
    }
    *out = (int)v;
    return 0;
}

int main(int argc, char *argv[]) {
    int p;
    int n;
    int chunk;
    int *arr;
    int fd[2];
    pid_t *children;
    int i;
    int global_min = INT_MAX;

    /* Formato: <p> seguido del arreglo completo (N valores). */
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <p> <v1> <v2> ... <vN>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s 3 8 4 7 10 -2 5\n", argv[0]);
        return 2;
    }

    /* Validar p. */
    if (parse_int(argv[1], &p) < 0 || p <= 0) {
        fprintf(stderr, "Error: p debe ser entero positivo.\n");
        return 2;
    }

    /* N es la cantidad de enteros recibidos luego de p. */
    n = argc - 2;
    if (n < p) {
        fprintf(stderr, "Error: N debe ser >= p.\n");
        return 2;
    }
    /* El enunciado exige bloques exactos de N/p elementos por hijo. */
    if (n % p != 0) {
        fprintf(stderr, "Error: N debe ser multiplo de p (N %% p == 0).\n");
        return 2;
    }

    arr = (int *)malloc((size_t)n * sizeof(int));
    children = (pid_t *)malloc((size_t)p * sizeof(pid_t));
    if (arr == NULL || children == NULL) {
        perror("malloc");
        free(arr);
        free(children);
        return 1;
    }

    /* Cargar arreglo de entrada. */
    for (i = 0; i < n; i++) {
        if (parse_int(argv[i + 2], &arr[i]) < 0) {
            fprintf(stderr, "Error: valor invalido en posicion %d: %s\n", i, argv[i + 2]);
            free(arr);
            free(children);
            return 2;
        }
    }

    chunk = n / p;

    /* Pipe único compartido: hijos escriben, padre lee. */
    if (pipe(fd) < 0) {
        perror("pipe");
        free(arr);
        free(children);
        return 1;
    }

    /* Crear p hijos; cada uno procesa su segmento. */
    for (i = 0; i < p; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(fd[0]);
            close(fd[1]);
            free(arr);
            free(children);
            return 1;
        }

        if (pid == 0) {
            int start = i * chunk;
            int end = start + chunk;
            int local_min = arr[start];
            ChildResult r;
            int k;

            close(fd[0]);

            /* Mínimo local del bloque [start, end). */
            for (k = start + 1; k < end; k++) {
                if (arr[k] < local_min) {
                    local_min = arr[k];
                }
            }

            r.child_id = i;
            r.local_min = local_min;

            /* Enviar resultado al padre. */
            if (write_full(fd[1], &r, sizeof(r)) < 0) {
                perror("write");
                close(fd[1]);
                _exit(1);
            }

            close(fd[1]);
            _exit(0);
        }

        children[i] = pid;
    }

    /* Padre no escribe en el pipe. */
    close(fd[1]);

    /* Leer p resultados y calcular mínimo global. */
    for (i = 0; i < p; i++) {
        ChildResult r;
        int rr = read_full(fd[0], &r, sizeof(r));
        if (rr != 0) {
            fprintf(stderr, "Error: no se pudo leer resultado de hijo (rr=%d)\n", rr);
            close(fd[0]);
            free(arr);
            free(children);
            return 1;
        }

        printf("Hijo %d -> minimo local = %d\n", r.child_id, r.local_min);
        if (r.local_min < global_min) {
            global_min = r.local_min;
        }
    }

    close(fd[0]);

    /* Esperar terminación de todos los hijos. */
    for (i = 0; i < p; i++) {
        int status;
        while (waitpid(children[i], &status, 0) < 0) {
            if (errno != EINTR) {
                perror("waitpid");
                free(arr);
                free(children);
                return 1;
            }
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Advertencia: hijo %d finalizo con error.\n", i);
        }
    }

    /* Resultado final del problema. */
    printf("Minimo global = %d\n", global_min);

    free(arr);
    free(children);
    return 0;
}
