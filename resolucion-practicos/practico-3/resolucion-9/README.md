# Resolucion 9 - `shmwriter.c` y `shmreader.c` (IPC SysV Shared Memory)

## 0) Que se pide en el ejercicio

El ejercicio pide observar como funciona la memoria compartida SysV (UNIX IPC):

1. Compilar y ejecutar `shmwriter.c` y `shmreader.c` (en ese orden).
2. Volver a correr `shmreader` y explicar que pasa.
3. Comentar en `shmreader.c` la linea que destruye la shm (`IPC_RMID`), correr una vez writer y varias veces reader, y explicar el comportamiento.

## 1) Archivos usados (movidos desde resolucion-8)

Se movieron solamente los solicitados:

- `shmwriter.c`
- `shmreader.c`

## 2) Compatibilidad Linux/macOS

Aunque el enunciado nombra Linux, estos programas usan API SysV (`ftok`, `shmget`, `shmat`, `shmdt`, `shmctl`), que tambien existe en macOS.

Se hicieron dos mejoras minimas para poder ejecutar de forma segura:

- chequeo de errores en llamadas del sistema
- reemplazo de `gets` por `fgets` (porque `gets` es insegura y obsoleta)

La logica de IPC del ejercicio no cambia.

## 3) Recordatorio conceptual corto (muy importante)

Una shm SysV tiene este ciclo de vida:

1. `ftok(path, proj_id)` genera una key.
2. `shmget(key, size, flags)` crea o recupera el segmento (retorna `shmid`).
3. `shmat(shmid, ...)` lo mapea al espacio virtual del proceso (retorna puntero local).
4. Proceso escribe/lee bytes compartidos.
5. `shmdt(ptr)` desadjunta en ese proceso.
6. `shmctl(shmid, IPC_RMID, NULL)` marca el segmento para eliminacion global.

Idea clave:

- `shmdt` solo desconecta al proceso actual.
- `IPC_RMID` afecta al objeto compartido del kernel (el segmento).

## 4) Preparacion y compilacion

`ftok("shmfile", 65)` requiere que exista `shmfile` en el directorio actual:

```bash
: > shmfile
```

Compilacion:

```bash
clang -std=c17 -Wall -Wextra -O0 -g shmwriter.c -o shmwriter
clang -std=c17 -Wall -Wextra -O0 -g shmreader.c -o shmreader
```

## 5) Ejecucion base (writer -> reader)

Comando:

```bash
printf 'mensaje inicial\n' | ./shmwriter
./shmreader
```

Salida observada:

```text
shared memory logical address: 0x104548000
Data to write to shmem: Data written in memory: mensaje inicial
shared memory logical address: 0x1021cc000
Data read from memory: mensaje inicial
```

Interpretacion detallada:

1. `shmwriter` y `shmreader` usan la misma key (`ftok("shmfile", 65)`), entonces apuntan al mismo `shmid`.
2. `shmwriter` escribe `mensaje inicial` en el segmento.
3. `shmreader` lee exactamente ese contenido desde otro proceso.
4. Las direcciones impresas (`0x104548000`, `0x1021cc000`) son distintas porque cada proceso puede mapear el mismo objeto fisico en direcciones virtuales diferentes. Esto es normal.

## 6) Inciso (a): correr nuevamente reader y observar

### Escenario con `IPC_RMID` activo (version original del reader)

En la version original, reader termina con:

```c
shmctl(shmid, IPC_RMID, NULL);
```

Si despues del primer reader se corre otra vez:

```bash
./shmreader
```

Salida observada:

```text
shared memory logical address: 0x1023d0000
Data read from memory:
```

Que significa esto:

1. El reader anterior elimino (marco para eliminar) el segmento compartido.
2. En la nueva corrida, `shmget(..., IPC_CREAT)` crea otro segmento nuevo.
3. Ese segmento nuevo no trae el string anterior, por eso aparece vacio.

## 7) Inciso (b): comentar la destruccion y repetir varias lecturas

Se comento en `shmreader.c`:

```c
// shmctl(shmid, IPC_RMID, NULL);
```

Luego se ejecuto:

```bash
printf 'persistente en shm\n' | ./shmwriter
./shmreader
./shmreader
./shmreader
```

Salida observada:

```text
shared memory logical address: 0x1022b0000
Data to write to shmem: Data written in memory: persistente en shm
shared memory logical address: 0x102970000
Data read from memory: persistente en shm
shared memory logical address: 0x102f4c000
Data read from memory: persistente en shm
shared memory logical address: 0x102620000
Data read from memory: persistente en shm
```

Interpretacion detallada:

1. Writer escribe una sola vez.
2. Reader puede ejecutarse varias veces y seguir leyendo el mismo contenido.
3. El dato persiste porque el segmento no se elimina al final de cada reader.
4. De nuevo, cambia el puntero virtual en cada proceso, pero el backing segment es el mismo.

## 8) Evidencia del estado de shm en el sistema

Consulta real:

```bash
ipcs -m
```

Salida observada (fragmento):

```text
T     ID     KEY        MODE       OWNER    GROUP
Shared Memory:
m  65536 0x000dfe8b --rw------- tomasrodeghiero    staff
m 196609 0x410a8c13 --rw-rw-rw- tomasrodeghiero    staff
```

Eso muestra segmentos SysV activos en el kernel.

## 9) Diagrama mental rapido

```text
Proceso A (writer)         Kernel (segmento shm)           Proceso B (reader)
------------------         ----------------------          ------------------
shmat -> ptr A   ----->    [ bytes compartidos ]    <-----  ptr B <- shmat
escribe texto             (mismo shmid / misma key)        lee texto

ptr A != ptr B (VA distintas), pero ambos apuntan al mismo objeto IPC.
```

## 10) Conclusiones de aprendizaje

1. Shared memory SysV permite compartir datos sin copiar por kernel en cada read/write.
2. El estado de persistencia depende de si se llama o no `IPC_RMID`.
3. `shmdt` no destruye; solo desadjunta localmente.
4. `IPC_RMID` cambia totalmente el comportamiento entre ejecuciones:
   con `IPC_RMID` el siguiente reader ve un segmento nuevo, sin `IPC_RMID` ve el contenido previo.
