# Resolucion 1 - Practico 3

Ejercicio 1: ejecutar `ps aux` para ver los procesos del sistema y entender su salida con ayuda de `man ps`.

## Como leer la salida de `ps aux`

Lo primero que conviene hacer es correr el comando y mirar las primeras lineas:

```bash
ps aux | head -n 5
```

Salida (ejemplo):

```text
USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
root           1  0.0  0.2 170364 12552 ?        Ss   09:00   0:02 /sbin/init
tomas       1234  1.5  0.8 812004 67240 ?        Sl   09:10   0:08 /usr/lib/firefox
...
```

Las columnas que mas se usan son las siguientes. `USER` y `PID` identifican al proceso y a su dueno. `%CPU` y `%MEM` reportan el porcentaje de uso instantaneo de cpu y de memoria fisica. `VSZ` y `RSS` muestran, respectivamente, la memoria virtual reservada y la residente en RAM (ambas en KB). `TTY` indica la terminal controladora (`?` si no tiene). `STAT` es la columna interesante para este ejercicio: codifica el estado del proceso. Por ultimo, `START` y `TIME` reportan el momento en que arranco y el tiempo total de cpu consumido, y `COMMAND` la linea de comando con la que se lanzo.

## 1.1 Estados de los procesos

La columna `STAT` codifica el estado actual del proceso. En macOS (que es BSD), los codigos principales son:

| Codigo | Estado                                                                 |
|--------|------------------------------------------------------------------------|
| `R`    | Running o Runnable (en CPU o lista para ejecutar)                      |
| `S`    | Sleeping interrumpible (dormido < ~20s, se despierta por senales)      |
| `I`    | Idle: dormido > ~20s (en macOS, no confundir con el `I` de Linux)      |
| `U`    | Uninterruptible wait (tipicamente I/O; en Linux es `D`)                |
| `T`    | Stopped por senal (`SIGSTOP`/`SIGTSTP`) o por el debugger              |
| `Z`    | Zombie (termino pero el padre no hizo `wait()`)                        |

Ademas de la letra principal, `ps` agrega flags: `<` indica nice negativo (alta prioridad), `N` lo opuesto. `s` marca al lider de sesion, `+` al proceso en foreground, `l` a uno multi-hilo y `L` a paginas lockeadas en RAM.

### Correspondencia con los estados de la teoria

La teoria del curso (capitulo *Procesos y threads*, Notas 5-6) define cuatro estados logicos por los que pasa toda tarea: `RUNNING`, `RUNNABLE` (o `READY`), `SLEEPING` (o `WAITING`), `ZOMBIE` y `TERMINATED`. Los codigos de `ps` son las distintas formas en que esos estados se exponen al usuario en cada UNIX:

| Estado teorico                 | Linux (`ps`) | macOS (`ps`) | Significado kernel                                                   |
|--------------------------------|--------------|--------------|----------------------------------------------------------------------|
| `RUNNING` / `RUNNABLE`         | `R`          | `R`          | Tarea en la CPU o en la cola de listos del scheduler                 |
| `SLEEPING` (interrumpible)     | `S`          | `S` / `I`    | Dormida en una `wait_queue`, se despierta por evento o senal         |
| `SLEEPING` (no interrumpible)  | `D`          | `U`          | Dormida durante I/O sincrono, no se la puede despertar con senales   |
| Detenida por senal             | `T`          | `T`          | Recibio `SIGSTOP`/`SIGTSTP`, sale por `SIGCONT`                      |
| `ZOMBIE`                       | `Z`          | `Z`          | Termino con `exit()` pero el padre todavia no hizo `wait()`          |

El estado `TERMINATED` no se ve con `ps` porque, una vez que el padre hace `wait()`, el descriptor se libera y el proceso desaparece de la tabla del kernel.

Para ver de un vistazo cuantos procesos hay en cada estado:

```bash
ps -Ao stat | awk '{print substr($1,1,1)}' | sort | uniq -c | sort -rn
```

## 1.2 Ordenar por uso de CPU y de memoria

Una particularidad de macOS es que su `ps` viene de BSD y no soporta `--sort`. En su lugar usa los flags posicionales `-r` y `-m`:

```bash
# por cpu, descendente
ps aux -r | head -n 10

# por memoria residente, descendente
ps aux -m | head -n 10
```

En Linux, por contraste, lo natural seria `ps aux --sort=-%cpu | head` y `ps aux --sort=-%mem | head`. Conceptualmente son lo mismo.

## 1.3 Procesos lanzados por el usuario actual

En BSD el flag `-u` espera un UID numerico, asi que la forma compacta del Linux clasico (`ps -u <user> u`) no funciona. Lo que si funciona es filtrar por usuario real con `-U` y elegir las columnas con `-o`:

```bash
ps -U "$USER" -o user,pid,%cpu,%mem,stat,command
```

Si por algun motivo se prefiere quedarse en `ps aux`, se puede filtrar despues con `awk`:

```bash
ps aux | awk -v u="$USER" '$1 == u'
```

Las dos alternativas listan los procesos del usuario que esta corriendo el shell, que es lo que pide el ejercicio.

## Conexion con la teoria

`ps` no es un comando autonomo: lee la informacion directamente de las estructuras del kernel que la teoria llama *task descriptors*. En Linux esta informacion vive bajo `/proc/<PID>/` (un pseudo-filesystem expuesto por el kernel) y `ps` la formatea para el usuario. Cada columna que devuelve corresponde a un campo del descriptor:

- `PID` -> identificador unico de la tarea.
- `STAT` -> campo `state` del descriptor.
- `%CPU` y `TIME` -> contadores de tiempo de ejecucion acumulados por el scheduler.
- `VSZ` y `RSS` -> tamano del *mapa de memoria* del proceso (espacio virtual reservado vs paginas residentes).
- `COMMAND` -> programa del cual el proceso es una instancia.

El ejercicio sirve por lo tanto para *ver materializados* los conceptos abstractos vistos en clase: la tabla de procesos, los estados de cada tarea y los recursos asociados (memoria, CPU, terminal).
