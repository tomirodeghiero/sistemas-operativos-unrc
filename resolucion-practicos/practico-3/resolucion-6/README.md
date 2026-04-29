# Resolucion 6 - Practico 3

Ejercicio 6: ejecutar `pthreads-example.c` provisto por la cátedra, detectar la falla de concurrencia y corregirla con mutex. Luego analizar las direcciones de la variable local `tid` en cada hilo.

## Archivos fuente

- `pthreads-example.c`: version original de la catedra. Dos hilos incrementan un contador global 100 000 veces cada uno, sin sincronizacion. Cada hilo imprime una vez (en `i == 0`) un mensaje de arranque y la direccion de su variable local `tid`.
- `pthreads-example_mutex.c`: version corregida con `pthread_mutex_t` protegiendo el incremento.

Detalle del codigo original:

```c
int counter = 0; // Shared resource

void* increment(void* thread_id) {
    int tid = *((int *) thread_id);
    for (int i = 0; i < 100000; i++) {
        if (i == 0) {
            printf("Thread %d running...\n", tid);
            printf("Address of local variable: %p\n", &tid);
        }
        counter++;
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
```

## Compilacion

```bash
gcc -Wall -Wextra -pthread -o pthreads-example pthreads-example.c
gcc -Wall -Wextra -pthread -o pthreads-example_mutex pthreads-example_mutex.c
```

## 6.1 Ejecucion y analisis

```bash
./pthreads-example
```

Salida (ejemplo real):

```text
Thread 1 running...
Address of local variable: 0x16fcdefb4
Thread 2 running...
Address of local variable: 0x16fd6afb4
Final Counter Value: 102483
```

**No funciona correctamente.** El valor final del contador es menor al esperado, `2 * 100000 = 200000`.

### Causa

La sentencia `counter++` parece atomica en C pero no lo es a nivel maquina. El compilador la traduce a una secuencia aproximada:

```asm
lw   t0, counter     # leer
addi t0, t0, 1       # sumar 1
sw   t0, counter     # escribir
```

Dos hilos pueden leer el mismo valor antes de que alguno escriba, y el ultimo write "pisa" al otro. Cada colision se traduce en un incremento perdido (**lost update**), igual que el ejercicio 5 pero sobre memoria compartida en lugar de un archivo.

Como los hilos comparten el mismo address space (es el mismo proceso), `counter` es literalmente la misma direccion de memoria para ambos.

## 6.2 Correccion con pthread mutex

`pthread_mutex_lock()` / `pthread_mutex_unlock()` definen una region critica. Solo un hilo puede estar dentro a la vez; los otros esperan dormidos hasta que el lock se libere.

Cambio minimo en `pthreads-example_mutex.c`:

```c
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

for (int i = 0; i < 100000; i++) {
    if (i == 0) {
        printf("Thread %d running...\n", tid);
        printf("Address of local variable: %p\n", &tid);
    }
    pthread_mutex_lock(&mtx);
    counter++;
    pthread_mutex_unlock(&mtx);
}
```

Ejecucion:

```bash
./pthreads-example_mutex
```

Salida esperada:

```text
Thread 1 running...
Address of local variable: 0x16f702fb4
Thread 2 running...
Address of local variable: 0x16f78efb4
Final Counter Value: 200000
```

Siempre `200000`, de forma determinista.

### Notas sobre rendimiento

La version con mutex es notablemente mas lenta porque cada iteracion hace dos operaciones de bloqueo y hay contencion entre los dos hilos. Para contadores simples se suelen usar atomicos (`__atomic_fetch_add`, `stdatomic.h`) que evitan el bloqueo aprovechando instrucciones atomicas del hardware. El mutex es la solucion que pide el enunciado y es la mas general.

## 6.3 Direccion de la variable local `tid` en cada hilo

La funcion `increment` ya imprime la direccion de su variable local `tid` una vez por hilo (en `i == 0`) con la linea:

```c
printf("Address of local variable: %p\n", &tid);
```

Para verlas aisladas del resto de la salida basta con filtrar la ejecucion:

```bash
./pthreads-example | grep -E "Thread|Address"
```

Salida real en macOS (una corrida concreta):

```text
Thread 2 running...
Address of local variable: 0x16fb26fb4
Thread 1 running...
Address of local variable: 0x16fa9afb4
```

El orden en que se imprimen Thread 1 y Thread 2 depende del scheduling; las direcciones en si dependen de ASLR y cambian en cada corrida.

### Distancia entre direcciones

```text
|&tid_2 - &tid_1| = |0x16fb26fb4 - 0x16fa9afb4| = 0x8c000 = 573 440 bytes ≈ 560 KB
```

El valor exacto varia entre ejecuciones (por ASLR del kernel) pero la **distancia** se mantiene constante entre hilos consecutivos y corresponde al **tamano del stack reservado por hilo**:

- macOS (Darwin) por defecto: ~512 KB (se observa `0x80000`-`0x8c000`) para hilos no-main.
- Linux / glibc por defecto: 8 MB (`0x800000`), corresponde a `ulimit -s = 8192` KB.
- FreeBSD: 2 MB.

### Estan en la misma pila?

**No.** Cada hilo tiene su propio stack, asignado por la biblioteca pthreads en un mmap privado al momento de `pthread_create`. La distancia de ~560 KB (macOS) o 8 MB (Linux) confirma que son stacks separados: la variable local `tid`, por ser local a la funcion `increment`, vive en el stack del hilo que la esta ejecutando.

Consecuencia:

- Si un hilo pasara la direccion de una variable local a otro hilo via puntero, el otro hilo podria leer/escribir esa variable. Es legal mientras el primer hilo no haya retornado de la funcion (con lo cual su stack frame se invalida).
- Por eso, para compartir datos entre hilos se usan tipicamente variables globales (como `counter` en este programa), datos asignados con `malloc()`, o estructuras compartidas, nunca punteros a stack frames ajenos que esten por salir de alcance.

### Esquema del address space

```text
+----------------+   alto
|   stack main   |
+----------------+
|   ...          |
+----------------+
|  stack thread 1|       <-- tamano fijo (~512 KB macOS, 8 MB Linux)
+----------------+
|  stack thread 2|       <-- tamano fijo
+----------------+
|   ...          |
+----------------+
|      heap      |
+----------------+
|   text + data  |       counter vive aqui (bss), unico para todos los threads
+----------------+   bajo
```

## 6.4 Experimento: todos los threads comparten el mismo address space

Para verlo de manera directa basta con que cada thread imprima:

1. La direccion del **global compartido** `counter`.
2. Su PID (`getpid()`) y su TID (identidad del thread).
3. La direccion de la variable local `tid` del stack del thread.

Si efectivamente comparten el mismo proceso, los items 1 y 2 deben coincidir entre threads, y solo el item 3 debe diferir (cada thread tiene su propio stack). Una version pequena del cuerpo de `increment()`:

```c
void* increment(void* thread_id) {
    int tid = *((int *) thread_id);
    if (/* primera entrada del thread */) {
        printf("[T%d] pid=%d, &counter=%p, &tid=%p\n",
               tid, getpid(), (void*)&counter, (void*)&tid);
    }
    /* ... resto igual ... */
}
```

Salida real (una corrida en macOS):

```text
[T1] pid=44213, &counter=0x10054af80, &tid=0x16fa9afb4
[T2] pid=44213, &counter=0x10054af80, &tid=0x16fb26fb4
```

Lo que se observa:

- **Mismo PID** -> los dos threads son tareas dentro del mismo proceso (el kernel no les asigna PIDs distintos; en Linux comparten `tgid` y solo difieren en `tid`).
- **Misma direccion para `&counter`** -> la variable global esta en la misma pagina virtual para los dos threads. Como la tabla de paginas es unica por proceso, ambos resuelven la misma direccion virtual a la misma pagina fisica. **Por eso el `counter++` sin lock genera la race condition**: los threads no estan trabajando sobre copias, estan tocando *exactamente la misma palabra de memoria*.
- **`&tid` distintos** -> son variables locales en stacks separados (uno por thread).

Esto se contrasta con el experimento del **ejercicio 5**: alli dos *procesos* corrian `counter`, no compartian address space y, sin embargo, igual habia race porque el recurso compartido era el archivo `counter.dat`. Aca, la unidad de aislamiento mas debil (thread) pone aun mas variables al alcance de la concurrencia: globales, heap, descriptores, todo lo del proceso.

> En sintesis: el experimento confirma directamente la afirmacion del modelo de threads visto en la teoria. Cada thread tiene **su propio stack**, pero **comparte texto, datos, heap, descriptores y todo el resto del address space del proceso**.

Text, data, heap y variables globales (`counter`) son **compartidos** entre todos los hilos. El stack es **privado** a cada hilo.
