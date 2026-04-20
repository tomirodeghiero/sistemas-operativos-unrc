/*
 * pthreads-example_mutex.c
 *
 * Version corregida de pthreads-example.c con pthread_mutex_t protegiendo
 * el incremento del contador global. El resto del programa se mantiene igual.
 */

#include <stdio.h>
#include <pthread.h>

int counter = 0; // Shared resource
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* increment(void* thread_id) {
    int tid = *((int *) thread_id);
    for (int i = 0; i < 100000; i++) {
        if (i == 0) {
            printf("Thread %d running...\n", tid);
            printf("Address of local variable: %p\n", &tid);
        }
        pthread_mutex_lock(&mtx);
        counter++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    int tn1 = 1, tn2 = 2;

    pthread_create(&t1, NULL, increment, &tn1);
    pthread_create(&t2, NULL, increment, &tn2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final Counter Value: %d\n", counter);
    return 0;
}
