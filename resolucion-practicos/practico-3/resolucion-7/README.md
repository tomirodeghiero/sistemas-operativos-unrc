# Resolucion 7 - Practico 3

Ejercicio 7: Java soporta multithreading (implementando `Runnable` o heredando de `Thread`) y mecanismos de sincronizacion como `synchronized` y los metodos `wait()`, `notify()`, `notifyAll()`. Se pide:

1. Analizar los mecanismos de creacion y control de threads.
2. Analizar los mecanismos de sincronizacion provistos y clasificarlos.
3. Programar el equivalente del ejercicio 6 sin race conditions.

## 7.1 Creacion y control de threads en Java

Java tiene un soporte de hilos *de primera clase* en la JVM. Cada hilo Java se corresponde uno-a-uno con un hilo nativo del sistema operativo (un *kernel thread* en GNU-Linux), por lo que aplica el mismo modelo conceptual visto en la teoria: comparten el espacio de direcciones del proceso, tienen su propio stack y son planificados por el kernel.

### Creacion: dos formas equivalentes

**Forma 1: heredar de `Thread`** (acopla la logica al mecanismo de hilos, poco flexible).

```java
class MiHilo extends Thread {
    @Override
    public void run() {
        // codigo a ejecutar en el hilo
    }
}

new MiHilo().start();
```

**Forma 2: implementar `Runnable`** (preferida; separa "que hace el hilo" de "como se lo lanza", y permite que la misma clase ya extienda otra).

```java
class Tarea implements Runnable {
    @Override
    public void run() { /* ... */ }
}

new Thread(new Tarea()).start();
// equivalente con lambda:
new Thread(() -> { /* ... */ }).start();
```

Variante con valor de retorno: `Callable<V>` + `ExecutorService.submit()` devuelve un `Future<V>`. El hilo no se administra a mano; lo lanza el pool.

### Llamadas clave para gestionar hilos

| Metodo                        | Que hace                                                               |
|-------------------------------|------------------------------------------------------------------------|
| `start()`                     | Pide a la JVM que cree el hilo y ejecute `run()` en paralelo           |
| `run()`                       | Cuerpo del hilo. Llamarlo directo NO crea hilo, ejecuta secuencial     |
| `join()`                      | Bloquea al hilo llamador hasta que el hilo termine                     |
| `Thread.sleep(ms)`            | Pone al hilo actual en estado `TIMED_WAITING`                          |
| `Thread.yield()`              | Sugiere al scheduler ceder la CPU (no garantizado)                     |
| `interrupt()`                 | Marca el hilo como interrumpido; lo despierta de `sleep`/`wait`        |
| `isInterrupted()`             | Consulta el flag de interrupcion sin limpiarlo                         |
| `setDaemon(true)`             | Marca el hilo como demonio (no impide la terminacion del proceso)      |
| `setPriority(int)`            | Sugiere prioridad (1..10). En la practica el SO la respeta poco        |

El ciclo de vida de un hilo Java tiene los estados `NEW`, `RUNNABLE`, `BLOCKED`, `WAITING`, `TIMED_WAITING` y `TERMINATED`. Es identico al esquema general visto en la teoria, con la diferencia de que `RUNNABLE` agrupa "ejecutando" y "lista para ejecutar" (la JVM no distingue cual de las dos).

## 7.2 Mecanismos de sincronizacion

Java provee dos primitivas integradas al lenguaje, sostenidas por una abstraccion comun: el **monitor**. Cada objeto Java lleva implicitamente un monitor (un mutex + una variable de condicion) que se activa con la palabra clave `synchronized`.

### `synchronized`: exclusion mutua

Garantiza que solo un hilo a la vez ejecute la region protegida sobre un objeto dado. Tres formas:

```java
// Bloque sincronizado: el lock es el objeto pasado entre parentesis.
synchronized (obj) { /* region critica */ }

// Metodo de instancia sincronizado: el lock implicito es 'this'.
public synchronized void m() { /* region critica */ }

// Metodo estatico sincronizado: el lock implicito es la Class del tipo.
public static synchronized void m() { /* region critica */ }
```

Los locks son **reentrantes**: si el mismo hilo entra dos veces al mismo monitor, no se autodbloquea (incrementa un contador interno y libera el lock recien al ultimo `unlock`). Ademas, el monitor se libera automaticamente al salir del bloque, incluso por excepcion, lo que evita el clasico problema de olvidarse de un `unlock`.

### `wait()` / `notify()` / `notifyAll()`: variables de condicion

Permiten que un hilo se suspenda esperando que otro hilo le avise que algo cambio. Las tres son metodos de `Object` y **solo pueden invocarse mientras se posee el monitor del objeto** (es decir, dentro de un `synchronized` sobre ese mismo objeto). Funcionan asi:

- `obj.wait()`: libera atomicamente el monitor de `obj` y duerme al hilo. Cuando es despertado, vuelve a tomar el monitor antes de retornar.
- `obj.notify()`: despierta a uno (cualquiera) de los hilos esperando en `obj`.
- `obj.notifyAll()`: despierta a todos los hilos esperando en `obj`.

Por la posibilidad de *spurious wakeups* y por el patron de uso multi-productor, la regla canonica es esperar **siempre dentro de un `while`**, no de un `if`:

```java
synchronized (obj) {
    while (!condicion) obj.wait();
    // ahora la condicion es cierta y tengo el monitor
}
```

### Clasificacion segun los tipos de mecanismos vistos en teoria

Los mecanismos de sincronizacion estudiados se agrupan en dos familias clasicas: **exclusion mutua** y **sincronizacion por condicion**. Java cubre ambas:

| Mecanismo Java                  | Categoria                                | Equivalente en POSIX/UNIX                  |
|---------------------------------|------------------------------------------|--------------------------------------------|
| `synchronized` (clase, metodo, bloque) | Exclusion mutua (mutex / monitor) | `pthread_mutex_t` / `flock`                |
| `wait()` / `notify()` / `notifyAll()` | Variable de condicion             | `pthread_cond_wait` / `cond_signal`        |
| `Thread.join()`                 | Sincronizacion por finalizacion          | `pthread_join` / `wait()` syscall          |
| `volatile`                      | Visibilidad entre threads (no exclusion) | `__atomic_*` / barreras de memoria         |

Ademas en `java.util.concurrent` hay implementaciones explicitas: `ReentrantLock`, `Semaphore`, `CountDownLatch`, `CyclicBarrier`, `BlockingQueue`, etc. Para este practico alcanza con las primitivas del nucleo del lenguaje.

## 7.3 Programa Java equivalente al ejercicio 6

Se replica la logica de `pthreads-example.c`: dos hilos incrementan un contador compartido 100 000 veces cada uno. Para evitar la race condition se protege el incremento con `synchronized` sobre un objeto-cerrojo dedicado.

```java
public class CounterExample {
    private static int counter = 0;
    private static final Object lock = new Object();
    private static final int N = 100_000;

    public static void main(String[] args) throws InterruptedException {
        Thread t1 = new Thread(new Incrementer(1));
        Thread t2 = new Thread(new Incrementer(2));

        t1.start();
        t2.start();

        t1.join();
        t2.join();

        System.out.println("Final Counter Value: " + counter);
    }

    static class Incrementer implements Runnable {
        private final int tid;

        Incrementer(int tid) { this.tid = tid; }

        @Override
        public void run() {
            System.out.println("Thread " + tid + " running...");
            for (int i = 0; i < N; i++) {
                synchronized (lock) {
                    counter++;
                }
            }
        }
    }
}
```

Compilacion y ejecucion:

```bash
javac CounterExample.java
java CounterExample
```

Salida esperada (siempre):

```text
Thread 1 running...
Thread 2 running...
Final Counter Value: 200000
```

### Por que esto funciona

`counter++` se traduce, igual que en C, a una secuencia *load-modify-store* no atomica. Sin sincronizacion, dos hilos pueden leer el mismo valor antes de que cualquiera escriba, y un incremento se pierde (mismo *lost update* del ejercicio 6). Al envolver el incremento con `synchronized (lock)`, la JVM garantiza:

1. **Exclusion mutua**: solo un hilo a la vez puede estar dentro del bloque, gracias al monitor de `lock`.
2. **Visibilidad**: las escrituras hechas dentro del bloque se ven garantizadamente desde otros hilos al re-entrar al monitor (el modelo de memoria de Java define esta relacion *happens-before* sobre lock/unlock).

Como el incremento ahora es atomico respecto de los otros hilos, el resultado es deterministicamente `2 * 100000 = 200000`.

### Variantes equivalentes

- Marcar el metodo como `synchronized`: si fuera de instancia, todos los hilos compartirian el mismo `this` como lock; aca como `Incrementer` se construye una instancia por hilo, el lock tiene que ser **un objeto compartido** (por eso el `static final Object lock`).
- Usar `AtomicInteger`:

  ```java
  private static final AtomicInteger counter = new AtomicInteger(0);
  // ...
  counter.incrementAndGet();
  ```

  Es mas eficiente porque usa instrucciones atomicas del hardware (`LOCK XADD` en x86, `LR/SC` en RISC-V) en vez de bloquear, pero conceptualmente el practico pide mostrar el uso de `synchronized`.
