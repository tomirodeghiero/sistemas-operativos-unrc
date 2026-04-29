/*
 * prod_cons_processes.c
 *
 * Productor-consumidor con dos procesos (fork) usando:
 *   - shared memory POSIX (shm_open + mmap) para el buffer compartido,
 *   - semaforos con nombre POSIX (sem_open) para sincronizar.
 *
 * Esquema:
 *   - /so_pc_empty: slots libres del buffer  (init = BUF_SIZE)
 *   - /so_pc_full:  slots ocupados          (init = 0)
 *   - /so_pc_mtx:   mutex binario           (init = 1)
 *   - /so_pc_shm:   memoria compartida con el buffer y los indices
 *
 * Compilacion:
 *   cc -O2 -Wall -Wextra -o prod_cons_processes prod_cons_processes.c -lrt   (Linux)
 *   cc -O2 -Wall -Wextra -o prod_cons_processes prod_cons_processes.c        (macOS)
 *
 * Notas:
 *   - sem_open crea el semaforo de manera persistente en el sistema; por eso
 *     se hace sem_unlink al inicio para evitar arrastrar estado de corridas
 *     anteriores, y de nuevo al final para limpiar.
 *   - Se usa shm_open + ftruncate + mmap para crear una region compartida
 *     que ambos procesos heredan despues del fork.
 */

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 8
#define N_ITEMS  32

#define NAME_EMPTY "/so_pc_empty"
#define NAME_FULL  "/so_pc_full"
#define NAME_MTX   "/so_pc_mtx"
#define NAME_SHM   "/so_pc_shm"

typedef struct {
    int  buffer[BUF_SIZE];
    int  in_idx;
    int  out_idx;
} shared_t;

static void die(const char *msg) { perror(msg); exit(1); }

int main(void) {
    /* Limpieza de cualquier residuo de corridas anteriores */
    sem_unlink(NAME_EMPTY);
    sem_unlink(NAME_FULL);
    sem_unlink(NAME_MTX);
    shm_unlink(NAME_SHM);

    /* 1. Memoria compartida para el buffer */
    int fd = shm_open(NAME_SHM, O_CREAT | O_RDWR, 0600);
    if (fd < 0) die("shm_open");
    if (ftruncate(fd, sizeof(shared_t)) != 0) die("ftruncate");

    shared_t *sh = mmap(NULL, sizeof(shared_t),
                       PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sh == MAP_FAILED) die("mmap");
    memset(sh, 0, sizeof(*sh));

    /* 2. Semaforos con nombre */
    sem_t *empty = sem_open(NAME_EMPTY, O_CREAT | O_EXCL, 0600, BUF_SIZE);
    if (empty == SEM_FAILED) die("sem_open empty");
    sem_t *full  = sem_open(NAME_FULL,  O_CREAT | O_EXCL, 0600, 0);
    if (full  == SEM_FAILED) die("sem_open full");
    sem_t *mtx   = sem_open(NAME_MTX,   O_CREAT | O_EXCL, 0600, 1);
    if (mtx   == SEM_FAILED) die("sem_open mtx");

    /* 3. Fork: padre = productor, hijo = consumidor */
    pid_t pid = fork();
    if (pid < 0) die("fork");

    if (pid == 0) {
        /* Hijo: consumidor */
        for (int i = 0; i < N_ITEMS; i++) {
            sem_wait(full);
            sem_wait(mtx);
            int item = sh->buffer[sh->out_idx];
            sh->out_idx = (sh->out_idx + 1) % BUF_SIZE;
            printf("    [C pid=%d] consumio %d (out=%d)\n",
                   (int)getpid(), item, sh->out_idx);
            sem_post(mtx);
            sem_post(empty);
            usleep(25 * 1000);
        }
        sem_close(empty); sem_close(full); sem_close(mtx);
        munmap(sh, sizeof(*sh));
        close(fd);
        exit(0);
    }

    /* Padre: productor */
    for (int i = 0; i < N_ITEMS; i++) {
        int item = i * i;
        sem_wait(empty);
        sem_wait(mtx);
        sh->buffer[sh->in_idx] = item;
        sh->in_idx = (sh->in_idx + 1) % BUF_SIZE;
        printf("[P pid=%d] produjo %d (in=%d)\n",
               (int)getpid(), item, sh->in_idx);
        sem_post(mtx);
        sem_post(full);
        usleep(10 * 1000);
    }

    /* Esperar al hijo y limpiar */
    waitpid(pid, NULL, 0);

    sem_close(empty); sem_close(full); sem_close(mtx);
    sem_unlink(NAME_EMPTY);
    sem_unlink(NAME_FULL);
    sem_unlink(NAME_MTX);

    munmap(sh, sizeof(*sh));
    close(fd);
    shm_unlink(NAME_SHM);
    return 0;
}
