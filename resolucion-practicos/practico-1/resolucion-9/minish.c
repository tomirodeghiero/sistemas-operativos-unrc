/*
 * minish.c — Mini shell educativo (Práctico 1, ejercicio 9).
 *
 * Lee comandos en un bucle REPL y los ejecuta hasta que el usuario presione
 * Ctrl-D (EOF en stdin). Reconoce los siguientes operadores binarios:
 *
 *     cmd1 ;  cmd2     ejecución secuencial
 *     cmd1 && cmd2     ejecución condicional (si cmd1 termina con éxito)
 *     cmd1 || cmd2     ejecución condicional (si cmd1 termina con error)
 *     cmd1 |  cmd2     pipe (stdout de cmd1 -> stdin de cmd2)
 *     cmd1 &  cmd2     cmd1 en background, cmd2 en foreground
 *     cmd  <  archivo  redirección de entrada estándar
 *     cmd  >  archivo  redirección de salida estándar (sobrescribe)
 *     cmd  >> archivo  redirección de salida estándar (append)
 *
 * El parsing es deliberadamente simple: tokeniza por espacios y busca el
 * primer operador encontrado de izquierda a derecha. No soporta comillas
 * ni expansión de variables: el objetivo es ilustrar el modelo
 * fork()/exec()/wait()/pipe()/dup2() de las Notas del curso.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_TOKENS 64

/* ----- Utilidades de tokens ----- */

/* Divide la línea por espacios. Retorna la cantidad de tokens y los deja en
 * out (que es un arreglo de punteros al buffer original, modificado in-situ
 * con \0 como separadores). */
static int tokenize(char *line, char *out[], int max) {
    int n = 0;
    char *p = line;

    while (*p != '\0' && n < max) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n') break;

        out[n++] = p;
        while (*p != '\0' && *p != '\n' && *p != ' ' && *p != '\t') p++;
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    return n;
}

/* Busca el primer token igual a op en tokens[0..n-1]. -1 si no aparece. */
static int find_op(char *tokens[], int n, const char *op) {
    for (int i = 0; i < n; i++) {
        if (strcmp(tokens[i], op) == 0) return i;
    }
    return -1;
}

/* ----- Ejecución de un comando simple ----- */

/* Ejecuta argv (NULL-terminated) con fork/execvp/waitpid.
 * Si stdin_fd / stdout_fd ≠ -1, los reasigna con dup2 antes de exec.
 * Si background = 1, no espera al hijo y retorna 0.
 * Devuelve el exit status del hijo (o 127 si exec falló). */
static int exec_simple(char *argv[], int stdin_fd, int stdout_fd, int background) {
    pid_t pid;
    int status;

    if (argv == NULL || argv[0] == NULL) return 0;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Hijo: aplicar redirecciones y reemplazar la imagen. */
        if (stdin_fd != -1) {
            if (dup2(stdin_fd, STDIN_FILENO) < 0) { perror("dup2"); _exit(1); }
            close(stdin_fd);
        }
        if (stdout_fd != -1) {
            if (dup2(stdout_fd, STDOUT_FILENO) < 0) { perror("dup2"); _exit(1); }
            close(stdout_fd);
        }
        execvp(argv[0], argv);
        /* Solo se llega aquí si exec falló. */
        fprintf(stderr, "minish: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    /* Padre. */
    if (background) {
        printf("[bg] pid=%d\n", (int)pid);
        return 0;
    }

    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) { perror("waitpid"); return 1; }
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return 1;
}

/* ----- Operadores ----- */

/* cmd1 | cmd2 : crea un pipe, redirige stdout(cmd1) y stdin(cmd2) y los
 * lanza en paralelo. */
static int op_pipe(char *left[], char *right[]) {
    int fd[2];
    pid_t pid1, pid2;
    int status1 = 0, status2 = 0;

    if (pipe(fd) < 0) { perror("pipe"); return 1; }

    pid1 = fork();
    if (pid1 < 0) { perror("fork"); return 1; }
    if (pid1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execvp(left[0], left);
        fprintf(stderr, "minish: %s: %s\n", left[0], strerror(errno));
        _exit(127);
    }

    pid2 = fork();
    if (pid2 < 0) { perror("fork"); return 1; }
    if (pid2 == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        execvp(right[0], right);
        fprintf(stderr, "minish: %s: %s\n", right[0], strerror(errno));
        _exit(127);
    }

    /* El padre debe cerrar AMBOS extremos para que los hijos vean EOF. */
    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    /* El exit status de un pipe en sh es el del último comando. */
    return WIFEXITED(status2) ? WEXITSTATUS(status2) : 1;
}

/* cmd < input_file */
static int op_redirect_in(char *cmd[], const char *path) {
    int fd = open(path, O_RDONLY);
    int rc;
    if (fd < 0) { perror(path); return 1; }
    rc = exec_simple(cmd, fd, -1, 0);
    close(fd);
    return rc;
}

/* cmd > output_file (sobreescribe) */
static int op_redirect_out(char *cmd[], const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int rc;
    if (fd < 0) { perror(path); return 1; }
    rc = exec_simple(cmd, -1, fd, 0);
    close(fd);
    return rc;
}

/* cmd >> output_file (append) */
static int op_redirect_append(char *cmd[], const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    int rc;
    if (fd < 0) { perror(path); return 1; }
    rc = exec_simple(cmd, -1, fd, 0);
    close(fd);
    return rc;
}

/* ----- Despacho según operador ----- */

static int dispatch(char *tokens[], int n) {
    int idx;

    if (n == 0) return 0;

    /* Operador "&" (background): cmd1 & cmd2 -> lanzar cmd1 en bg, cmd2 fg.
     * Si '&' es el último token, solo cmd1 en background. */
    idx = find_op(tokens, n, "&");
    if (idx >= 0) {
        char *left[MAX_TOKENS];
        for (int i = 0; i < idx; i++) left[i] = tokens[i];
        left[idx] = NULL;
        exec_simple(left, -1, -1, 1);

        if (idx + 1 < n) {
            return dispatch(&tokens[idx + 1], n - idx - 1);
        }
        return 0;
    }

    /* Secuencia: cmd1 ; cmd2 */
    idx = find_op(tokens, n, ";");
    if (idx >= 0) {
        char *left[MAX_TOKENS];
        for (int i = 0; i < idx; i++) left[i] = tokens[i];
        left[idx] = NULL;
        dispatch(left, idx);
        return dispatch(&tokens[idx + 1], n - idx - 1);
    }

    /* Condicionales: cmd1 && cmd2 / cmd1 || cmd2 */
    idx = find_op(tokens, n, "&&");
    if (idx >= 0) {
        char *left[MAX_TOKENS];
        int rc;
        for (int i = 0; i < idx; i++) left[i] = tokens[i];
        left[idx] = NULL;
        rc = dispatch(left, idx);
        if (rc == 0) return dispatch(&tokens[idx + 1], n - idx - 1);
        return rc;
    }

    idx = find_op(tokens, n, "||");
    if (idx >= 0) {
        char *left[MAX_TOKENS];
        int rc;
        for (int i = 0; i < idx; i++) left[i] = tokens[i];
        left[idx] = NULL;
        rc = dispatch(left, idx);
        if (rc != 0) return dispatch(&tokens[idx + 1], n - idx - 1);
        return rc;
    }

    /* Pipe: cmd1 | cmd2 */
    idx = find_op(tokens, n, "|");
    if (idx >= 0) {
        char *left[MAX_TOKENS], *right[MAX_TOKENS];
        for (int i = 0; i < idx; i++) left[i] = tokens[i];
        left[idx] = NULL;
        for (int i = idx + 1; i < n; i++) right[i - idx - 1] = tokens[i];
        right[n - idx - 1] = NULL;
        return op_pipe(left, right);
    }

    /* Redirecciones */
    idx = find_op(tokens, n, ">>");
    if (idx >= 0 && idx + 1 < n) {
        char *cmd[MAX_TOKENS];
        for (int i = 0; i < idx; i++) cmd[i] = tokens[i];
        cmd[idx] = NULL;
        return op_redirect_append(cmd, tokens[idx + 1]);
    }

    idx = find_op(tokens, n, ">");
    if (idx >= 0 && idx + 1 < n) {
        char *cmd[MAX_TOKENS];
        for (int i = 0; i < idx; i++) cmd[i] = tokens[i];
        cmd[idx] = NULL;
        return op_redirect_out(cmd, tokens[idx + 1]);
    }

    idx = find_op(tokens, n, "<");
    if (idx >= 0 && idx + 1 < n) {
        char *cmd[MAX_TOKENS];
        for (int i = 0; i < idx; i++) cmd[i] = tokens[i];
        cmd[idx] = NULL;
        return op_redirect_in(cmd, tokens[idx + 1]);
    }

    /* Comando simple. tokens debe estar NULL-terminated antes de execvp. */
    {
        char *argv[MAX_TOKENS + 1];
        for (int i = 0; i < n; i++) argv[i] = tokens[i];
        argv[n] = NULL;
        return exec_simple(argv, -1, -1, 0);
    }
}

/* ----- Loop principal ----- */

int main(void) {
    char *line = NULL;
    size_t cap = 0;

    for (;;) {
        fputs("minish$ ", stdout);
        fflush(stdout);

        ssize_t nread = getline(&line, &cap, stdin);
        if (nread < 0) {
            /* EOF (Ctrl-D) o error: salir limpiamente. */
            putchar('\n');
            break;
        }

        char *tokens[MAX_TOKENS];
        int n = tokenize(line, tokens, MAX_TOKENS);
        if (n == 0) continue;

        /* Builtin opcional: exit. */
        if (strcmp(tokens[0], "exit") == 0) break;

        dispatch(tokens, n);
    }

    free(line);
    return 0;
}
