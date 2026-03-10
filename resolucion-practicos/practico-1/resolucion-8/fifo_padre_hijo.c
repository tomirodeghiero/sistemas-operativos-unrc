#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Named pipe usado para comunicar padre e hijo. */
#define FIFO_PATH "/tmp/practico1_fifo_pipe"

/* Escribe todo el buffer, manejando interrupciones por señal. */
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

/* Crea el FIFO si no existe; si existe, valida que realmente sea FIFO. */
static int ensure_fifo(const char *path) {
    struct stat st;

    if (lstat(path, &st) == 0) {
        if (S_ISFIFO(st.st_mode)) {
            return 0;
        }
        fprintf(stderr, "Error: %s existe y no es un FIFO.\n", path);
        return -1;
    }

    if (mkfifo(path, 0600) < 0) {
        perror("mkfifo");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    pid_t pid;
    const char *msg = "Mensaje enviado por el proceso padre mediante FIFO";
    int status;

    /* Mensaje opcional pasado por argumento. */
    if (argc >= 2) {
        msg = argv[1];
    }

    /* Crear/validar FIFO antes del fork. */
    if (ensure_fifo(FIFO_PATH) < 0) {
        return 1;
    }

    /* Crear hijo. */
    pid = fork();
    if (pid < 0) {
        perror("fork");
        unlink(FIFO_PATH);
        return 1;
    }

    /* Hijo: abre FIFO para lectura y muestra lo recibido. */
    if (pid == 0) {
        char buffer[256];
        int fd;
        ssize_t nread;

        fd = open(FIFO_PATH, O_RDONLY);
        if (fd < 0) {
            perror("open read");
            _exit(1);
        }

        printf("Hijo recibio: ");
        fflush(stdout);

        while ((nread = read(fd, buffer, sizeof(buffer))) > 0) {
            if (write_all(STDOUT_FILENO, buffer, (size_t)nread) < 0) {
                perror("write");
                close(fd);
                _exit(1);
            }
        }

        if (nread < 0) {
            perror("read");
            close(fd);
            _exit(1);
        }

        write_all(STDOUT_FILENO, "\n", 1);
        close(fd);
        _exit(0);
    }

    /* Padre: abre FIFO para escritura y envía el mensaje. */
    {
        int fd = open(FIFO_PATH, O_WRONLY);
        if (fd < 0) {
            perror("open write");
            unlink(FIFO_PATH);
            return 1;
        }

        if (write_all(fd, msg, strlen(msg)) < 0) {
            perror("write");
            close(fd);
            unlink(FIFO_PATH);
            return 1;
        }

        close(fd);
    }

    /* Esperar fin del hijo. */
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        unlink(FIFO_PATH);
        return 1;
    }

    /* Limpieza del recurso FIFO. */
    if (unlink(FIFO_PATH) < 0) {
        perror("unlink");
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return 1;
}
