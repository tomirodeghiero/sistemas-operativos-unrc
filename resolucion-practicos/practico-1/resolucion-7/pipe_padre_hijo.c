#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Escribe exactamente len bytes, reintentando si hay interrupciones. */
static int write_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;

    while (sent < len) {
        ssize_t n = write(fd, buf + sent, len - sent);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        sent += (size_t)n;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int p[2];
    pid_t pid;
    const char *msg = "Mensaje enviado por el proceso padre";
    int status;

    /* Permite mensaje custom por argumento. */
    if (argc >= 2) {
        msg = argv[1];
    }

    /* p[0] = lectura, p[1] = escritura. */
    if (pipe(p) < 0) {
        perror("pipe");
        return 1;
    }

    /* Crear hijo. */
    pid = fork();
    if (pid < 0) {
        perror("fork");
        close(p[0]);
        close(p[1]);
        return 1;
    }

    /* Hijo: lee del pipe y lo muestra en stdout. */
    if (pid == 0) {
        char buffer[256];
        ssize_t nread;

        /* El hijo no escribe en el pipe. */
        close(p[1]);

        printf("Hijo recibio: ");
        fflush(stdout);

        /* Lee hasta EOF (cuando el padre cierra escritura). */
        while ((nread = read(p[0], buffer, sizeof(buffer))) > 0) {
            if (write_all(STDOUT_FILENO, buffer, (size_t)nread) < 0) {
                perror("write");
                close(p[0]);
                _exit(1);
            }
        }

        if (nread < 0) {
            perror("read");
            close(p[0]);
            _exit(1);
        }

        write_all(STDOUT_FILENO, "\n", 1);
        close(p[0]);
        _exit(0);
    }

    /* Padre: escribe en el pipe. */
    close(p[0]);

    if (write_all(p[1], msg, strlen(msg)) < 0) {
        perror("write");
        close(p[1]);
        return 1;
    }

    /* Cerrar escritura para que el hijo vea EOF. */
    close(p[1]);

    /* Esperar al hijo y propagar su estado. */
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return 1;
}
