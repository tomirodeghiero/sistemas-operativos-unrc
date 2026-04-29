/*
 * prod_cons_threads.c
 *
 * Productor-consumidor con POSIX threads y semaforos POSIX sin nombre
 * (sem_init/sem_destroy). Buffer circular en memoria del proceso.
 *
 * Esquema clasico (Dijkstra):
 *   - empty: cuenta slots libres del buffer  (init = BUF_SIZE)
 *   - full:  cuenta slots ocupados           (init = 0)
 *   - mtx:   protege la actualizacion del buffer (mutex binario, init = 1)
 *
 * Compilacion (Linux):
 *   cc -O2 -Wall -Wextra -pthread -o prod_cons_threads prod_cons_threads.c
 *
 * En macOS/Darwin sem_init() esta deprecado y devuelve ENOSYS, por lo que
 * este programa esta pensado para correrse en GNU/Linux. Para macOS, ver
 * prod_cons_processes.c que usa sem_open con semaforos con nombre.
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE 8
#define N_ITEMS  32

static int buffer[BUF_SIZE];
static int in_idx  = 0;   // proxima posicion a escribir
static int out_idx = 0;   // proxima posicion a leer

static sem_t empty;       // slots libres
static sem_t full;        // slots ocupados
static sem_t mtx;         // mutex sobre el buffer

static void *producer(void *arg) {
    (void)arg;
    for (int i = 0; i < N_ITEMS; i++) {
        int item = i * i;            // "produccion" simulada

        sem_wait(&empty);            // espero slot libre
        sem_wait(&mtx);              // entro a la region critica
        buffer[in_idx] = item;
        in_idx = (in_idx + 1) % BUF_SIZE;
        printf("[P] produjo %d (in=%d)\n", item, in_idx);
        sem_post(&mtx);              // salgo de la region critica
        sem_post(&full);             // hay un slot mas ocupado

        usleep(10 * 1000);           // 10 ms para mezclar la salida
    }
    return NULL;
}

static void *consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < N_ITEMS; i++) {
        sem_wait(&full);
        sem_wait(&mtx);
        int item = buffer[out_idx];
        out_idx = (out_idx + 1) % BUF_SIZE;
        printf("    [C] consumio %d (out=%d)\n", item, out_idx);
        sem_post(&mtx);
        sem_post(&empty);

        usleep(25 * 1000);           // consumidor mas lento que productor
    }
    return NULL;
}

int main(void) {
    if (sem_init(&empty, 0, BUF_SIZE) != 0) { perror("sem_init empty"); exit(1); }
    if (sem_init(&full,  0, 0)        != 0) { perror("sem_init full");  exit(1); }
    if (sem_init(&mtx,   0, 1)        != 0) { perror("sem_init mtx");   exit(1); }

    pthread_t tp, tc;
    pthread_create(&tp, NULL, producer, NULL);
    pthread_create(&tc, NULL, consumer, NULL);
    pthread_join(tp, NULL);
    pthread_join(tc, NULL);

    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mtx);
    return 0;
}
