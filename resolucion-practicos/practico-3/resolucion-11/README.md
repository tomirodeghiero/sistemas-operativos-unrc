# Resolucion 11 - Practico 3

Ejercicio 11: implementar el problema del **productor-consumidor** usando la API de semaforos POSIX (`sem_init`, `sem_open`, `sem_wait`, `sem_post`, `sem_post`, ...).

1. Usando POSIX threads.
2. Usando procesos y semaforos con nombre. El buffer compartido puede ser un archivo o una shared memory area.

## Arquitectura comun a las dos versiones

Es la solucion clasica de Dijkstra con buffer acotado y tres semaforos:

| Semaforo | Significado                | Init    | Lo decrementa                | Lo incrementa             |
|----------|----------------------------|---------|------------------------------|---------------------------|
| `empty`  | cantidad de slots libres   | `BUF_SIZE` | Productor antes de escribir | Consumidor tras leer      |
| `full`   | cantidad de slots ocupados | `0`     | Consumidor antes de leer    | Productor tras escribir   |
| `mtx`    | mutex sobre el buffer      | `1`     | Quien va a tocar el buffer  | Quien lo libera            |

El esquema de cada parte es identico en threads y en procesos:

```text
Productor                          Consumidor
---------                          ----------
sem_wait(empty)                    sem_wait(full)
sem_wait(mtx)                      sem_wait(mtx)
  buffer[in] = item                  item = buffer[out]
  in = (in+1) % BUF_SIZE             out = (out+1) % BUF_SIZE
sem_post(mtx)                      sem_post(mtx)
sem_post(full)                     sem_post(empty)
```

Algunas observaciones que justifican este orden:

- **`sem_wait(empty)` antes que `sem_wait(mtx)`**: si fuera al reves, el productor podria tomar el mutex teniendo el buffer lleno y dejaria al consumidor sin chance de avanzar -> deadlock clasico.
- El mutex se mantiene **solo** para la actualizacion del indice y la escritura/lectura del slot. La produccion y el consumo "reales" (los `usleep` que simulan trabajo en los programas) quedan **fuera** de la region critica para no serializar todo el pipeline.
- Los semaforos contadores `empty` y `full` actuan como variables de condicion: bloquean al productor cuando el buffer esta lleno y al consumidor cuando esta vacio.

## 11.1 Version con POSIX threads

Archivo: [`prod_cons_threads.c`](./prod_cons_threads.c).

Como los hilos comparten memoria, alcanza con declarar el buffer y los semaforos como variables globales del proceso. Los semaforos son **sin nombre** (`sem_t` declarado directo, inicializado con `sem_init`).

Compilacion y ejecucion (Linux):

```bash
cc -O2 -Wall -Wextra -pthread -o prod_cons_threads prod_cons_threads.c
./prod_cons_threads
```

Salida tipica (recortada):

```text
[P] produjo 0 (in=1)
[P] produjo 1 (in=2)
    [C] consumio 0 (out=1)
[P] produjo 4 (in=3)
    [C] consumio 1 (out=2)
[P] produjo 9 (in=4)
...
```

El productor adelanta al consumidor hasta que se llena el buffer (`empty` se vuelve 0). Ahi se bloquea en `sem_wait(empty)` hasta que el consumidor libere un slot.

> **Nota**: en macOS/Darwin `sem_init()` esta deprecado y devuelve `ENOSYS` en runtime. Se mantiene la advertencia del compilador (`-Wdeprecated-declarations`) intacta y este programa esta pensado para correrse en GNU-Linux. Para macOS, la version 11.2 con semaforos con nombre funciona.

## 11.2 Version con procesos y named semaphores

Archivo: [`prod_cons_processes.c`](./prod_cons_processes.c).

Los procesos no comparten memoria por defecto, asi que hace falta:

1. Un area de **shared memory POSIX** para el buffer y los indices, creada con `shm_open` + `ftruncate` + `mmap`.
2. **Semaforos con nombre** (`sem_open`) que vivan en el namespace global del kernel. Cualquier proceso que los abra por el mismo nombre comparte el mismo objeto.

El programa hace:

1. `shm_unlink` y `sem_unlink` al inicio para limpiar restos de corridas previas.
2. `shm_open(NAME_SHM, O_CREAT|O_RDWR, 0600)` + `ftruncate` + `mmap` con `MAP_SHARED` -> obtiene un puntero al buffer compartido.
3. `sem_open(NAME_*, O_CREAT|O_EXCL, 0600, valor_inicial)` para `empty`, `full`, `mtx`.
4. `fork()`. El **padre** ejecuta el productor; el **hijo** ejecuta el consumidor.
5. Al terminar, `sem_close`, `sem_unlink`, `munmap`, `close`, `shm_unlink` para no dejar basura en `/dev/shm`.

Compilacion:

```bash
# Linux: hace falta -lrt para shm_open
cc -O2 -Wall -Wextra -o prod_cons_processes prod_cons_processes.c -lrt

# macOS: shm_open/sem_open viven en libc, no se necesita -lrt
cc -O2 -Wall -Wextra -o prod_cons_processes prod_cons_processes.c
```

Ejecucion:

```bash
./prod_cons_processes
```

Salida tipica (recortada):

```text
[P pid=12345] produjo 0 (in=1)
    [C pid=12346] consumio 0 (out=1)
[P pid=12345] produjo 1 (in=2)
[P pid=12345] produjo 4 (in=3)
    [C pid=12346] consumio 1 (out=2)
...
```

`pid` distinto en cada linea: confirma que productor y consumidor son procesos separados que sin embargo comparten el mismo buffer y los mismos semaforos.

### Por que `O_EXCL` y `sem_unlink`

`sem_open` con `O_CREAT` solo crea el semaforo si no existe; con `O_CREAT | O_EXCL` falla si ya existe. Esto obliga a una creacion limpia y avisa si una corrida anterior dejo basura en el sistema. La rutina inicial `sem_unlink` borra cualquier semaforo previo con esos nombres antes de crearlos. La rutina final hace lo mismo para no contaminar a la siguiente corrida.

### Por que shared memory y no un archivo

El enunciado permite usar un archivo plano como buffer (la cinta usa `read`/`write`/`fsync` o `mmap` sobre un archivo regular). Se eligio shared memory POSIX porque:

- Es mas rapido (no toca el filesystem).
- `mmap` con `MAP_SHARED` sobre el `fd` que devolvio `shm_open` pone una sola estructura en RAM con el buffer y los dos indices, accesible de ambos procesos sin sincronizacion adicional mas alla del mutex POSIX.
- En sistemas modernos las regiones de `shm_open` se ven en `/dev/shm/<nombre>` y se pueden inspeccionar facilmente durante el desarrollo.

Si en cambio se usa un archivo regular, hay que recordar `fsync` para forzar persistencia y se paga el costo de I/O. La logica de sincronizacion es identica.

## Por que esta solucion no tiene problemas tipicos

- **Sin lost wakeup**: los semaforos contadores recuerdan los `post` aunque no haya nadie esperando. Si el consumidor llega antes que el productor, `sem_wait(full)` lo deja dormido; cuando el productor hace `sem_post(full)` el consumidor se despierta.
- **Sin deadlock**: ambos hilos/procesos toman primero los semaforos contadores y despues el mutex. Como el mutex se libera en cada iteracion, no hay ciclos de espera.
- **Sin starvation**: `sem_wait` en POSIX es FIFO en la mayoria de las implementaciones (no es estrictamente requerido por el estandar pero es lo habitual en glibc/Linux). En la practica los hilos despertados se atienden en orden de llegada.
- **Sin race condition sobre `in_idx`/`out_idx`**: ambos vivien dentro de la region critica protegida por `mtx`.

## Resumen de las APIs usadas

| Llamada                                  | Que hace                                                              |
|------------------------------------------|-----------------------------------------------------------------------|
| `sem_init(sem, pshared, value)`          | Inicializa un semaforo sin nombre (memoria del proceso o shm)         |
| `sem_open(name, flags, mode, value)`     | Crea o abre un semaforo con nombre (vive en `/dev/shm` en Linux)      |
| `sem_wait(sem)` / `sem_trywait(sem)`     | P(sem). Decrementa o bloquea hasta poder hacerlo                      |
| `sem_post(sem)`                          | V(sem). Incrementa y eventualmente despierta a un waiter              |
| `sem_close(sem)` / `sem_destroy(sem)`    | Cierra el descriptor (con nombre) o destruye (sin nombre)             |
| `sem_unlink(name)`                       | Quita el nombre del namespace; los procesos que lo tienen abierto siguen usandolo |
| `shm_open(name, flags, mode)`            | Crea/abre una region de memoria compartida POSIX                      |
| `mmap(NULL, size, ..., MAP_SHARED, fd, 0)` | Mapea esa region en el espacio del proceso                          |
