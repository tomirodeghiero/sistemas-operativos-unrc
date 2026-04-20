# Resolucion 5 - Practico 3

Ejercicio 5: programa `counter.c` provisto por la catedra que incrementa un contador persistente en `counter.dat`. Primero se observa la race condition al correr dos instancias en paralelo y luego se la corrige con `flock()`.

## Archivos fuente

- `counter.c`: 1000 iteraciones de lectura-modificacion-escritura sobre `counter.dat` usando `open`, `lseek`, `read`, `write` y `fsync`. Escribe el valor con formato `"%d"` (sin padding, longitud variable segun el numero).
- `counter_locked.c`: version corregida que envuelve la zona critica de cada iteracion con `flock(fd, LOCK_EX)` / `flock(fd, LOCK_UN)`.

Detalle del codigo original:

```c
int fd = open("counter.dat", O_RDWR);

for (i=0; i<1000; i++) {
    char s[N];
    int n;

    lseek(fd, 0, SEEK_SET);
    read(fd, s, N);
    n = atoi(s);
    sprintf(s, "%d", n+1);
    lseek(fd, 0, SEEK_SET);   // rewind
    write(fd, s, strlen(s));
    fsync(fd);
}
```

## Compilacion

```bash
gcc -Wall -Wextra -o counter counter.c
```

Nota: el compilador emite una advertencia por la variable `pid` declarada pero no usada en el codigo oficial. Es inofensiva.

## Preparacion

Crear el archivo de datos con valor inicial `0`:

```bash
echo 0 > counter.dat
```

## 5.1 Ejecucion secuencial

Una sola invocacion (para ver el `1000` del enunciado):

```bash
echo 0 > counter.dat
./counter
cat counter.dat    # 1000
```

Dos invocaciones encadenadas:

```bash
echo 0 > counter.dat
./counter ; ./counter
cat counter.dat    # 2000
```

Cada invocacion incrementa 1000 veces el valor. El enunciado dice "deberia quedar en 1000" porque considera una sola invocacion; con dos ejecuciones secuenciales partiendo de `0` se obtienen `1000 + 1000 = 2000`. Lo que importa es que en secuencial **no hay perdida de incrementos**.

## 5.2 Ejecucion concurrente (sin lock)

```bash
echo 0 > counter.dat
./counter & ./counter
wait
cat counter.dat
```

Salida observada: un valor **menor a 2000** (en mis corridas: `1826`, `1740`, `1766`, varia entre ejecuciones).

## 5.3 Explicacion del resultado

El ciclo hace, por iteracion, la secuencia:

1. `lseek(fd, 0)` + `read` -> leer el valor actual.
2. `atoi` + `+1` -> calcular nuevo valor.
3. `lseek(fd, 0)` + `write` + `fsync` -> grabarlo.

Con dos procesos A y B ejecutando en paralelo sobre el mismo archivo, esta secuencia no es atomica. Una intercalacion posible:

```text
A lseek(0); read -> "100"    (v=100)
                            B lseek(0); read -> "100"    (v=100)
A sprintf("101"); write 3 bytes
                            B sprintf("101"); write 3 bytes   <-- pisa igual
```

Ambos procesos terminan escribiendo el mismo valor: un incremento se pierde. Es un **lost update** clasico. El resultado final depende del intercalado del scheduler y suele ubicarse bastante por debajo de 2000.

Detalles tecnicos adicionales que refuerzan la race:

- El descriptor `fd` es independiente en cada proceso (cada uno hizo su propio `open`), por lo que tienen offsets separados aunque apunten al mismo inode.
- `fsync()` solo garantiza que la escritura llegue a disco, no excluye mutuamente a otro proceso.
- Con `"%d"` el tamano del archivo crece monotonamente (1, 2, 3, 4 digitos). Como los numeros nunca achican su longitud, no quedan bytes residuales problematicos en el caso secuencial; en el concurrente la race es la misma por lost update, independiente del formato.

## 5.4 Solucion con flock()

`flock(fd, LOCK_EX)` toma un **lock exclusivo advisory** sobre el descriptor. Mientras un proceso lo tiene, cualquier otro que intente tomarlo exclusivamente se bloquea hasta que se libere con `LOCK_UN` (o hasta que se cierre el descriptor).

En `counter_locked.c` la modificacion es minima: se agrega `flock(fd, LOCK_EX)` al inicio del cuerpo del for y `flock(fd, LOCK_UN)` al final, englobando el read-modify-write:

```c
for (i=0; i<1000; i++) {
    char s[N];
    int n;

    flock(fd, LOCK_EX);

    lseek(fd, 0, SEEK_SET);
    read(fd, s, N);
    n = atoi(s);
    sprintf(s, "%d", n+1);
    lseek(fd, 0, SEEK_SET);
    write(fd, s, strlen(s));
    fsync(fd);

    flock(fd, LOCK_UN);
}
```

Con el lock, las regiones criticas de los dos procesos quedan serializadas y el resultado es determinista.

Verificacion:

```bash
echo 0 > counter.dat
./counter_locked & ./counter_locked
wait
cat counter.dat    # 2000
```

### Notas

- `flock()` es **advisory**: solo funciona si todos los procesos cooperan usando el mismo mecanismo. No impide que un proceso que ignore el lock pise el archivo.
- Alternativa POSIX: `fcntl(F_SETLKW, ...)` con rangos de bytes, mas flexible pero mas verboso.
- Con NFS antiguo `flock()` puede tener problemas. En filesystems locales (ext4, apfs, tmpfs) funciona correctamente.
