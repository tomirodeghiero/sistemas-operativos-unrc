#include <stdint.h>
#include <stddef.h>

/* Semaforos + demo productor-consumidor para un mini-kernel */

/*
 * Para evitar error de tipo incompleto en chequeos locales, uso placeholders.
 * Cuando EDOS_USE_REAL_SYNC_TYPES esta definido, se usan los headers reales.
 */
#ifdef EDOS_USE_REAL_SYNC_TYPES
#include "spinlock.h"
#include "wait_queue.h"
#else
struct spinlock {
    int _placeholder;
};
struct wait_queue {
    int _placeholder;
};
#endif

/* Primitivas del kernel que usa esta implementacion */

extern void spin_init(struct spinlock *lk);
extern void spin_lock(struct spinlock *lk);
extern void spin_unlock(struct spinlock *lk);

extern void wait_queue_init(struct wait_queue *q);
extern void wakeup_one(struct wait_queue *q);

/* Duerme en q liberando lk de forma atomica; al volver, lk queda tomado. */
extern void sleep_on(struct wait_queue *q, struct spinlock *lk);

extern void task_yield(void);
extern int task_create(const char *name, void (*fn)(void *), void *arg);
extern void kprintf(const char *fmt, ...);

struct semaphore {
    /* value > 0: hay recursos; value == 0: hay que bloquearse */
    int value;

    /* protege value y la cola de espera */
    struct spinlock lock;

    /* tareas bloqueadas en sem_wait */
    struct wait_queue waiters;
};

void sem_init(struct semaphore *s, int initial_value)
{
    /* inicializacion basica */
    s->value = initial_value;
    spin_init(&s->lock);
    wait_queue_init(&s->waiters);
}

void sem_wait(struct semaphore *s)
{
    /* P/down: espera recurso y lo consume */
    spin_lock(&s->lock);
    while (s->value == 0) {
        /* al despertar, revalida condicion */
        sleep_on(&s->waiters, &s->lock);
    }
    /* consume recurso */
    s->value--;
    spin_unlock(&s->lock);
}

void sem_signal(struct semaphore *s)
{
    /* V/up: libera recurso y despierta a uno */
    spin_lock(&s->lock);
    s->value++;
    wakeup_one(&s->waiters);
    spin_unlock(&s->lock);
}

/* ------------------------- Productor-consumidor --------------------------- */

#define BUF_CAP 8
#define PRODUCERS 2
#define CONSUMERS 2
#define ITEMS_PER_PRODUCER 40

/* Buffer circular compartido */
static int buffer[BUF_CAP];
static int in_idx = 0;
static int out_idx = 0;

/* Semaforos clasicos: empty/full/mutex */
static struct semaphore sem_empty;
static struct semaphore sem_full;
static struct semaphore sem_mutex;

static void put_item(int v)
{
    /* inserta y avanza */
    buffer[in_idx] = v;
    in_idx = (in_idx + 1) % BUF_CAP;
}

static int get_item(void)
{
    /* extrae y avanza */
    int v = buffer[out_idx];
    out_idx = (out_idx + 1) % BUF_CAP;
    return v;
}

static void producer_task(void *arg)
{
    /* cada productor genera su lote */
    intptr_t id = (intptr_t)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = (int)(id * 1000 + i);

        /* espera lugar libre */
        sem_wait(&sem_empty);

        /* region critica del buffer */
        sem_wait(&sem_mutex);
        put_item(item);
        kprintf("[PROD %ld] produce %d\n", (long)id, item);

        /* sale de region critica */
        sem_signal(&sem_mutex);

        /* hay un item nuevo */
        sem_signal(&sem_full);

        /* cede CPU */
        task_yield();
    }
}

static void consumer_task(void *arg)
{
    /* reparte items entre consumidores */
    intptr_t id = (intptr_t)arg;
    int items_to_consume = (PRODUCERS * ITEMS_PER_PRODUCER) / CONSUMERS;

    for (int i = 0; i < items_to_consume; i++) {
        int item;

        /* espera item */
        sem_wait(&sem_full);

        /* region critica del buffer */
        sem_wait(&sem_mutex);
        item = get_item();
        kprintf("[CONS %ld] consume %d\n", (long)id, item);

        /* sale de region critica */
        sem_signal(&sem_mutex);

        /* libera un lugar */
        sem_signal(&sem_empty);

        /* cede CPU */
        task_yield();
    }
}

/* Punto de arranque de la demo */
void sem_demo_start(void)
{
    /* estado inicial de semaforos */
    sem_init(&sem_empty, BUF_CAP);
    sem_init(&sem_full, 0);
    sem_init(&sem_mutex, 1);

    /* crea productores */
    for (intptr_t i = 0; i < PRODUCERS; i++) {
        task_create("producer", producer_task, (void *)i);
    }

    /* crea consumidores */
    for (intptr_t i = 0; i < CONSUMERS; i++) {
        task_create("consumer", consumer_task, (void *)i);
    }
}
