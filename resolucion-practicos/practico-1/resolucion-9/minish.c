#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * minish.c
 * ----------
 * Mini shell educativo que soporta:
 *   - ;   secuencial
 *   - &   fondo
 *   - &&  condicional AND
 *   - ||  condicional OR
 *   - |   pipe
 *   - <, >, >> redirecciones en comando simple
 */

#define MAX_ARGS 128

/* Operadores de control soportados por el parser. */
typedef enum {
    OP_NONE = 0,
    OP_SEQ,
    OP_BG,
    OP_AND,
    OP_OR,
    OP_PIPE
} OperatorType;

/* Representación de un comando simple (sin operadores binarios). */
typedef struct {
    char *argv[MAX_ARGS];
    int argc;
    char *infile;
    char *outfile;
    int append;
} SimpleCmd;

/* Elimina espacios en los extremos de un string (in-place). */
static char *trim_inplace(char *s) {
    char *end;

    while (*s && isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return s;
    }

    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return s;
}

/* Crea una copia recortada de line[start..start+len). */
static char *slice_trim(const char *line, size_t start, size_t len) {
    char *tmp = (char *)malloc(len + 1);
    char *trimmed;
    size_t n;

    if (tmp == NULL) {
        return NULL;
    }

    memcpy(tmp, line + start, len);
    tmp[len] = '\0';

    trimmed = trim_inplace(tmp);
    if (trimmed != tmp) {
        n = strlen(trimmed);
        memmove(tmp, trimmed, n + 1);
    }

    return tmp;
}

/* Busca de derecha a izquierda el último ';' o '&' (fondo simple). */
static int find_last_seq_or_bg(const char *s, OperatorType *op, size_t *pos) {
    size_t i;
    int found = 0;
    size_t len = strlen(s);

    for (i = 0; i < len; i++) {
        if (s[i] == ';') {
            *op = OP_SEQ;
            *pos = i;
            found = 1;
            continue;
        }
        if (s[i] == '&') {
            if (i + 1 < len && s[i + 1] == '&') {
                i++;
                continue;
            }
            *op = OP_BG;
            *pos = i;
            found = 1;
        }
    }

    return found;
}

/* Busca el último operador lógico && o ||. */
static int find_last_and_or(const char *s, OperatorType *op, size_t *pos) {
    size_t i;
    int found = 0;
    size_t len = strlen(s);

    for (i = 0; i + 1 < len; i++) {
        if (s[i] == '&' && s[i + 1] == '&') {
            *op = OP_AND;
            *pos = i;
            found = 1;
            i++;
            continue;
        }
        if (s[i] == '|' && s[i + 1] == '|') {
            *op = OP_OR;
            *pos = i;
            found = 1;
            i++;
        }
    }

    return found;
}

/* Busca el último '|' que no forme parte de '||'. */
static int find_last_pipe(const char *s, size_t *pos) {
    size_t i;
    int found = 0;
    size_t len = strlen(s);

    for (i = 0; i < len; i++) {
        if (s[i] != '|') {
            continue;
        }
        if (i + 1 < len && s[i + 1] == '|') {
            i++;
            continue;
        }
        if (i > 0 && s[i - 1] == '|') {
            continue;
        }
        *pos = i;
        found = 1;
    }

    return found;
}

/* Libera memoria asociada a un comando simple parseado. */
static void free_simple(SimpleCmd *cmd) {
    int i;

    for (i = 0; i < cmd->argc; i++) {
        free(cmd->argv[i]);
    }
    free(cmd->infile);
    free(cmd->outfile);
}

/*
 * Parser básico de comando simple:
 * - separa argumentos por espacios
 * - detecta redirecciones <, > y >>
 * - no implementa comillas avanzadas (alcance educativo)
 */
static int parse_simple(const char *line, SimpleCmd *cmd) {
    const char *p = line;
    int pending = 0; /* 1: < , 2: > , 3: >> */

    memset(cmd, 0, sizeof(*cmd));

    while (*p) {
        const char *start;
        size_t len;
        char *tok;

        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        if (*p == '<') {
            pending = 1;
            p++;
            continue;
        }
        if (*p == '>') {
            if (*(p + 1) == '>') {
                pending = 3;
                p += 2;
            } else {
                pending = 2;
                p++;
            }
            continue;
        }

        start = p;
        while (*p && !isspace((unsigned char)*p) && *p != '<' && *p != '>') {
            p++;
        }
        len = (size_t)(p - start);

        tok = (char *)malloc(len + 1);
        if (tok == NULL) {
            perror("malloc");
            free_simple(cmd);
            return -1;
        }
        memcpy(tok, start, len);
        tok[len] = '\0';

        if (pending == 1) {
            free(cmd->infile);
            cmd->infile = tok;
            pending = 0;
            continue;
        }
        if (pending == 2 || pending == 3) {
            free(cmd->outfile);
            cmd->outfile = tok;
            cmd->append = (pending == 3);
            pending = 0;
            continue;
        }

        if (cmd->argc >= MAX_ARGS - 1) {
            fprintf(stderr, "Demasiados argumentos\n");
            free(tok);
            free_simple(cmd);
            return -1;
        }
        cmd->argv[cmd->argc++] = tok;
        cmd->argv[cmd->argc] = NULL;
    }

    if (pending != 0) {
        fprintf(stderr, "Redireccion incompleta\n");
        free_simple(cmd);
        return -1;
    }

    return 0;
}

/*
 * Ejecuta un comando simple en el proceso actual:
 * - aplica redirecciones
 * - llama a execvp
 * Este código se invoca desde un hijo.
 */
static int run_simple_in_current_process(const char *line) {
    SimpleCmd cmd;
    int fd;

    if (parse_simple(line, &cmd) < 0) {
        return 2;
    }

    if (cmd.argc == 0) {
        free_simple(&cmd);
        return 0;
    }

    if (cmd.infile != NULL) {
        fd = open(cmd.infile, O_RDONLY);
        if (fd < 0) {
            perror("open <");
            free_simple(&cmd);
            return 1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 <");
            close(fd);
            free_simple(&cmd);
            return 1;
        }
        close(fd);
    }

    if (cmd.outfile != NULL) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd.append ? O_APPEND : O_TRUNC;
        fd = open(cmd.outfile, flags, 0644);
        if (fd < 0) {
            perror("open >");
            free_simple(&cmd);
            return 1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 >");
            close(fd);
            free_simple(&cmd);
            return 1;
        }
        close(fd);
    }

    execvp(cmd.argv[0], cmd.argv);
    perror("execvp");
    free_simple(&cmd);
    return 127;
}

/* Espera un pid y traduce status a código de salida "shell-like". */
static int wait_exit_code(pid_t pid) {
    int status;

    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            perror("waitpid");
            return 1;
        }
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return 1;
}

/* Declaración adelantada porque execute_pipe llama recursivamente a execute_line. */
static int execute_line(char *line);

/* Ejecuta "left | right" usando dos hijos y un pipe del kernel. */
static int execute_pipe(char *left, char *right) {
    int fd[2];
    pid_t p1;
    pid_t p2;
    int st1, st2;

    if (pipe(fd) < 0) {
        perror("pipe");
        return 1;
    }

    p1 = fork();
    if (p1 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (p1 == 0) {
        int code;
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) < 0) {
            perror("dup2 pipe left");
            _exit(1);
        }
        close(fd[1]);
        code = execute_line(left);
        _exit(code & 0xFF);
    }

    p2 = fork();
    if (p2 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (p2 == 0) {
        int code;
        close(fd[1]);
        if (dup2(fd[0], STDIN_FILENO) < 0) {
            perror("dup2 pipe right");
            _exit(1);
        }
        close(fd[0]);
        code = execute_line(right);
        _exit(code & 0xFF);
    }

    close(fd[0]);
    close(fd[1]);

    st1 = wait_exit_code(p1);
    st2 = wait_exit_code(p2);
    (void)st1;
    /* Convención típica: retornar estado del comando de la derecha. */
    return st2;
}

/* Ejecuta una línea que no tiene operadores compuestos. */
static int execute_simple(char *line) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        int code = run_simple_in_current_process(line);
        _exit(code & 0xFF);
    }

    return wait_exit_code(pid);
}

/*
 * Evaluador recursivo de la línea:
 * - respeta precedencia básica al buscar operadores por etapas
 * - divide en subexpresiones y ejecuta cada parte
 */
static int execute_line(char *line) {
    char *work = trim_inplace(line);
    OperatorType op = OP_NONE;
    size_t pos = 0;

    if (*work == '\0') {
        return 0;
    }

    /* Menor precedencia: ';' y '&'. */
    if (find_last_seq_or_bg(work, &op, &pos)) {
        char *left = slice_trim(work, 0, pos);
        char *right = slice_trim(work, pos + 1, strlen(work) - pos - 1);
        int result = 0;

        if (left == NULL || right == NULL) {
            free(left);
            free(right);
            perror("malloc");
            return 1;
        }

        if (op == OP_SEQ) {
            /* cmd1 ; cmd2: cmd2 siempre se ejecuta */
            (void)execute_line(left);
            result = execute_line(right);
            free(left);
            free(right);
            return result;
        }

        if (*left != '\0') {
            /* cmd1 & ... : cmd1 corre en background */
            pid_t bg = fork();
            if (bg < 0) {
                perror("fork");
                free(left);
                free(right);
                return 1;
            }
            if (bg == 0) {
                int code = execute_line(left);
                _exit(code & 0xFF);
            }
            printf("[bg pid %d]\n", (int)bg);
        }

        /* Luego se ejecuta lo que esté a la derecha en foreground. */
        if (*right != '\0') {
            result = execute_line(right);
        } else {
            result = 0;
        }

        free(left);
        free(right);
        return result;
    }

    /* Precedencia media: && y ||. */
    if (find_last_and_or(work, &op, &pos)) {
        char *left = slice_trim(work, 0, pos);
        char *right = slice_trim(work, pos + 2, strlen(work) - pos - 2);
        int left_status;
        int result;

        if (left == NULL || right == NULL) {
            free(left);
            free(right);
            perror("malloc");
            return 1;
        }

        left_status = execute_line(left);
        if (op == OP_AND) {
            /* cmd1 && cmd2: cmd2 solo si cmd1 terminó en 0 */
            result = (left_status == 0) ? execute_line(right) : left_status;
        } else {
            /* cmd1 || cmd2: cmd2 solo si cmd1 falló (!=0) */
            result = (left_status != 0) ? execute_line(right) : left_status;
        }

        free(left);
        free(right);
        return result;
    }

    /* Precedencia más alta entre compuestos: pipe. */
    if (find_last_pipe(work, &pos)) {
        char *left = slice_trim(work, 0, pos);
        char *right = slice_trim(work, pos + 1, strlen(work) - pos - 1);
        int result;

        if (left == NULL || right == NULL) {
            free(left);
            free(right);
            perror("malloc");
            return 1;
        }

        result = execute_pipe(left, right);
        free(left);
        free(right);
        return result;
    }

    /* Caso base: comando simple. */
    return execute_simple(work);
}

int main(void) {
    char *line = NULL;
    size_t cap = 0;

    while (1) {
        ssize_t nread;

        /* Prompt interactivo. */
        printf("minish$ ");
        fflush(stdout);

        nread = getline(&line, &cap, stdin);
        if (nread < 0) {
            if (feof(stdin)) {
                /* Ctrl-D: salir del shell. */
                printf("\n");
                break;
            }
            perror("getline");
            continue;
        }

        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        (void)execute_line(line);

        /* Recolectar hijos de fondo que hayan terminado. */
        while (waitpid(-1, NULL, WNOHANG) > 0) {
            /* Reap background children. */
        }
    }

    free(line);
    return 0;
}
