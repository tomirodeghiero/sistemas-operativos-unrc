# Resolucion 5 - Practico 3

Ejercicio 5: dado el programa `counter.c` provisto por la catedra, observar la race condition al correr dos instancias en paralelo y despues solucionarla con `flock()`.

## Los archivos del ejercicio

- `counter.c`: version original. Hace 1000 iteraciones de un read-modify-write sobre `counter.dat` usando `open`, `lseek`, `read`, `atoi`, `sprintf`, `write` y `fsync`. Escribe el numero con formato `"%d"` (longitud variable segun el valor).
- `counter_locked.c`: version corregida. Es la misma pero envolviendo el read-modify-write con `flock(fd, LOCK_EX)` / `flock(fd, LOCK_UN)`.

El cuerpo del bucle del original se ve asi:

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

Compilacion:

```bash
gcc -Wall -Wextra -o counter counter.c
```

> El compilador advierte por una variable `pid` declarada pero no usada en el codigo oficial. Es inofensiva.

Antes de cada experimento, dejo el contador en cero:

```bash
echo 0 > counter.dat
```

## 5.1 Ejecucion secuencial

Una sola invocacion incrementa 1000 veces y deja el archivo con `1000`. Encadenando dos con `;`, la segunda parte de `1000` y termina en `2000`:

```bash
echo 0 > counter.dat
./counter ; ./counter
cat counter.dat    # 2000
```

El enunciado dice "deberia quedar en 1000" porque considera una sola invocacion. Lo importante es que en secuencial **no se pierde ningun incremento**: cada ejecucion empieza despues de que la anterior cierra el archivo, asi que no hay nada que se pisen entre si.

## 5.2 Ejecucion concurrente sin lock

Ahora las dos instancias en paralelo:

```bash
echo 0 > counter.dat
./counter & ./counter
wait
cat counter.dat
```

El resultado es **menor a 2000** y, ademas, **no determinista**: en mis corridas obtuve `1826`, `1740`, `1766`, valores que cambian de ejecucion en ejecucion.

## 5.3 Por que se pierden incrementos

El problema es que la secuencia "leer, sumar uno, escribir" no es atomica. Para cada iteracion el codigo hace:

1. `lseek(fd, 0)` y `read()` -> trae el valor actual.
2. `atoi` y `+1` -> calcula el nuevo valor.
3. `lseek(fd, 0)`, `write()` y `fsync()` -> graba.

Con dos procesos `A` y `B` corriendo en paralelo sobre el mismo archivo, se puede dar facilmente un intercalado como este:

```text
A lseek(0); read -> "100"    (v=100)
                            B lseek(0); read -> "100"    (v=100)
A sprintf("101"); write 3 bytes
                            B sprintf("101"); write 3 bytes   <-- pisa con el mismo valor
```

Los dos procesos terminan escribiendo `101` cuando deberian haber producido `101` y `102`. Un incremento se pierde silenciosamente. Esto es exactamente el patron clasico de **lost update** y el resultado final depende de como el scheduler haya intercalado los pasos. Por eso el numero final queda muy por debajo de 2000 y varia entre corridas.

Algunos detalles que vale la pena mencionar:

- Cada proceso tiene su propio `fd` porque cada uno hizo su `open` (el descriptor es local a cada proceso aunque apunte al mismo inodo). Esto significa que tienen offsets independientes.
- `fsync()` solo garantiza que el dato llegue a disco; **no excluye** a otro proceso que este queriendo escribir en paralelo.
- Como el formato es `"%d"`, el archivo crece monotonamente en tamano (de 1 a 2 a 3 a 4 digitos). En el caso secuencial eso no es un problema. En el concurrente la race es la misma de todas formas; el formato no es la causa.

## 5.4 Solucion con `flock()`

`flock(fd, LOCK_EX)` toma un lock exclusivo *advisory* sobre el descriptor. Mientras un proceso lo tiene, cualquier otro que pida el mismo lock se duerme hasta que el primero llame a `LOCK_UN` (o cierre el fd). El cambio en el codigo es minimo: encierro el read-modify-write con un par `LOCK_EX`/`LOCK_UN`, dejando lo mas chica posible la region critica:

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

Verificacion:

```bash
echo 0 > counter.dat
./counter_locked & ./counter_locked
wait
cat counter.dat    # 2000
```

Con el lock, las regiones criticas de los dos procesos quedan serializadas: si entra `A` primero, `B` espera dormido hasta que `A` libere; despues le toca a `B`. Los 2000 incrementos se aplican y el resultado es deterministico.

### Detalles de `flock`

- Es **advisory**: solo funciona si todos los procesos cooperan usando el mismo mecanismo. Si alguno ignora el lock y abre el archivo crudo, lo va a poder pisar igual. En el contexto del ejercicio esto no es un problema porque controlo a las dos instancias.
- Como alternativa POSIX existe `fcntl(F_SETLKW, ...)`, que permite locks por rangos de bytes en lugar de sobre el descriptor entero. Es mas flexible pero mas verboso. Para este caso `flock` alcanza.
- Sobre NFS antiguo `flock` puede comportarse de manera rara, pero en filesystems locales (ext4, apfs, tmpfs) anda bien.

## 5.5 Es `fd` una variable compartida entre las distintas instancias?

**No**. Cuando dos procesos hacen `./counter` por separado, cada uno corre en su propio espacio de direcciones y cada uno hace su propio `open("counter.dat", O_RDWR)`. El kernel les devuelve un descriptor en la tabla de descriptores **de cada proceso**, que casualmente probablemente sea el numero `3` en ambos (porque `0`, `1`, `2` ya estan ocupados por stdin/stdout/stderr), pero son entradas en tablas distintas. Apuntan al mismo inodo del filesystem, no a la misma `struct file` del kernel; cada uno tiene su propio offset interno.

Esto es justamente lo que hace que la race se pueda dar: como los offsets son independientes, ambos procesos pueden hacer `lseek(0)` y leer simultaneamente el mismo valor sin que el kernel los serialice. El recurso compartido entre las dos instancias **no es `fd`**, sino el **archivo en disco** (`counter.dat`), y la manera de coordinar el acceso a ese recurso compartido es el `flock` introducido en 5.4.

> Caso distinto: si las dos instancias no fueran procesos separados sino threads del mismo proceso, ahi `fd` si seria una variable compartida (todos los threads ven la misma tabla de descriptores). Pero ese es el escenario del ejercicio 6, no este.
