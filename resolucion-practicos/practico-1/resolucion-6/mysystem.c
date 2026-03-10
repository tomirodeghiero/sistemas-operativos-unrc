#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Versión simplificada de system():
 * - crea un hijo con fork()
 * - ejecuta "/bin/sh -c <command>" en el hijo
 * - espera al hijo y devuelve su status crudo (como waitpid)
 */
int mysystem(const char *command) {
    pid_t pid;
    int status;

    /* Comportamiento estándar: consultar si hay shell disponible. */
    if (command == NULL) {
        return access("/bin/sh", X_OK) == 0;
    }

    /* Crear proceso hijo. */
    pid = fork();
    if (pid < 0) {
        return -1;
    }

    /* Hijo: reemplaza su imagen con /bin/sh -c command. */
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        /* Si exec falla, convención: salir con 127. */
        _exit(127);
    }

    /* Padre: espera al hijo (reintenta si la espera fue interrumpida). */
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            return -1;
        }
    }

    return status;
}

/* Une argv[1..] en un único string con espacios para pasarlo a mysystem(). */
static char *build_command(int argc, char *argv[]) {
    size_t total = 0;
    size_t i;
    char *command;

    for (i = 1; i < (size_t)argc; i++) {
        total += strlen(argv[i]) + 1;
    }

    command = malloc(total + 1);
    if (command == NULL) {
        return NULL;
    }

    command[0] = '\0';
    for (i = 1; i < (size_t)argc; i++) {
        strcat(command, argv[i]);
        if (i + 1 < (size_t)argc) {
            strcat(command, " ");
        }
    }

    return command;
}

int main(int argc, char *argv[]) {
    char *command;
    int status;

    /* Se requiere al menos un comando para ejecutar. */
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <comando>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s ls -l\n", argv[0]);
        return 2;
    }

    command = build_command(argc, argv);
    if (command == NULL) {
        perror("malloc");
        return 1;
    }

    status = mysystem(command);
    free(command);

    if (status < 0) {
        perror("mysystem");
        return 1;
    }

    /* Decodificar estado de terminación del hijo. */
    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        printf("Comando finalizo con exit status %d\n", code);
        return code;
    }

    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("Comando finalizo por senal %d\n", sig);
        return 128 + sig;
    }

    fprintf(stderr, "Comando finalizo en estado no esperado\n");
    return 1;
}
