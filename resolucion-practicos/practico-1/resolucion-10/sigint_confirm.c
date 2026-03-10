#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Devuelve el primer carácter no blanco leído en la respuesta del usuario. */
static int first_non_space_char(const char *buf, ssize_t n) {
    ssize_t i;

    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)buf[i];
        if (!isspace(c)) {
            return c;
        }
    }
    return -1;
}

/*
 * Handler de SIGINT:
 * pregunta al usuario si quiere finalizar y termina con:
 * - 0  -> respuesta afirmativa
 * - -1 -> cualquier otra respuesta (visible como 255 en shell)
 *
 * Nota: dentro de un handler usamos llamadas async-signal-safe (write/read/_exit).
 */
static void sigint_handler(int sig) {
    const char prompt[] = "\nSIGINT recibida. Desea finalizar el proceso? [s/N]: ";
    char answer[64];
    ssize_t nread;
    int ch;

    (void)sig;

    if (write(STDOUT_FILENO, prompt, sizeof(prompt) - 1) < 0) {
        _exit(255);
    }

    do {
        nread = read(STDIN_FILENO, answer, sizeof(answer));
    } while (nread < 0 && errno == EINTR);

    if (nread < 0) {
        _exit(255);
    }

    ch = first_non_space_char(answer, nread);

    if (ch == 's' || ch == 'S' || ch == 'y' || ch == 'Y') {
        _exit(0);
    }

    _exit(-1);
}

int main(void) {
    struct sigaction sa;

    /* Registrar handler para SIGINT. */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    printf("Proceso en ejecucion. PID=%d\n", (int)getpid());
    printf("Envie SIGINT con Ctrl-C o: kill -SIGINT %d\n", (int)getpid());
    fflush(stdout);

    /* Espera pasiva de señales. */
    for (;;) {
        pause();
    }
}
