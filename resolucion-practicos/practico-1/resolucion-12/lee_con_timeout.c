#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Flag global (segura para señales) para indicar timeout. */
static volatile sig_atomic_t timed_out = 0;

/* Handler de SIGALRM: solo marca que venció el tiempo. */
static void alarm_handler(int sig) {
    (void)sig;
    timed_out = 1;
}

/* Valida N de segundos (entero positivo en rango razonable). */
static int parse_positive_int(const char *s, int *out) {
    char *end = NULL;
    long v;

    errno = 0;
    v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        return -1;
    }
    if (v <= 0 || v > 3600) {
        return -1;
    }
    *out = (int)v;
    return 0;
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    int timeout_sec;
    char buf[256];
    ssize_t nread;

    /* Se espera un único argumento: timeout en segundos. */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <N_segundos>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s 5\n", argv[0]);
        return 2;
    }

    if (parse_positive_int(argv[1], &timeout_sec) < 0) {
        fprintf(stderr, "Error: N debe ser un entero positivo (1..3600).\n");
        return 2;
    }

    /*
     * Importante: sin SA_RESTART para que read() se interrumpa por SIGALRM
     * y podamos detectar timeout.
     */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; /* sin SA_RESTART para que read sea interrumpida por SIGALRM */

    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    printf("Esperando un dato por stdin (timeout: %d s)...\n", timeout_sec);
    fflush(stdout);

    /* Arranca reloj y queda bloqueado esperando datos. */
    alarm((unsigned int)timeout_sec);
    nread = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    /* Si llegó antes, cancelar alarma pendiente. */
    alarm(0);

    /* Caso 1: llegó dato antes del timeout. */
    if (nread > 0) {
        buf[nread] = '\0';
        printf("Dato recibido: %s", buf);
        if (buf[nread - 1] != '\n') {
            printf("\n");
        }
        return 0;
    }

    /* Caso 2: stdin se cerró (EOF). */
    if (nread == 0) {
        printf("EOF recibido (stdin cerrado).\n");
        return 0;
    }

    /* Caso 3: read interrumpida por SIGALRM => timeout. */
    if (errno == EINTR && timed_out) {
        printf("Timeout: no se recibieron datos en %d segundos.\n", timeout_sec);
        return 124;
    }

    /* Otro error de lectura. */
    perror("read");
    return 1;
}
