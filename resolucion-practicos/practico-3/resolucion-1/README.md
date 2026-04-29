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
