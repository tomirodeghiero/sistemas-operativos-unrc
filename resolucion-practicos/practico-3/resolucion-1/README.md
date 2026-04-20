# Resolucion 1 - Practico 3

Ejercicio 1: ejecutar `ps aux` para ver los procesos del sistema y entender su salida con `man ps`.

## Salida tipica y columnas

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

Columnas relevantes:

- `USER`: duenio del proceso.
- `PID`: identificador del proceso.
- `%CPU`, `%MEM`: uso porcentual.
- `VSZ`, `RSS`: memoria virtual y residente (KB).
- `TTY`: terminal controladora (`?` = ninguna).
- `STAT`: estado del proceso (ver abajo).
- `START`, `TIME`: inicio y tiempo de CPU consumido.
- `COMMAND`: linea de comandos.

## 1.1 Estados de los procesos

La columna `STAT` codifica el estado. Codigos en macOS (BSD):

| Codigo | Estado                                                                 |
|--------|------------------------------------------------------------------------|
| `R`    | Running o Runnable (en CPU o lista para ejecutar)                      |
| `S`    | Sleeping interrumpible (dormido < ~20s, se despierta por senales)      |
| `I`    | Idle: dormido > ~20s (en macOS, no confundir con el `I` de Linux)      |
| `U`    | Uninterruptible wait (tipicamente I/O; en Linux es `D`)                |
| `T`    | Stopped por senal (`SIGSTOP`/`SIGTSTP`) o por el debugger              |
| `Z`    | Zombie (termino pero el padre no hizo `wait()`)                        |

Letras adicionales en la misma columna:

- `<` alta prioridad (nice negativo), `N` baja prioridad (nice positivo).
- `s` lider de sesion, `+` en foreground group, `l` multi-hilo, `L` paginas en RAM lockeadas.

Ver conteo rapido por estado:

```bash
ps -Ao stat | awk '{print substr($1,1,1)}' | sort | uniq -c | sort -rn
```

## 1.2 Ordenar por uso de CPU y de memoria

En macOS el `ps` es BSD y no soporta `--sort`. Se usan los flags `-r` y `-m`:

Por uso de CPU (descendente):

```bash
ps aux -r | head -n 10
```

Por uso de memoria (descendente):

```bash
ps aux -m | head -n 10
```

## 1.3 Procesos lanzados por el usuario actual

En macOS (BSD) se usa `-U` para filtrar por usuario real y `-o` para elegir el formato de columnas:

```bash
ps -U "$USER" -o user,pid,%cpu,%mem,stat,command
```

Alternativa con filtro por texto sobre la salida de `ps aux`:

```bash
ps aux | awk -v u="$USER" '$1 == u'
```

Nota: en macOS, `ps -u <user> u` (la forma comun en Linux con flag posicional `u` para formato user-oriented) no funciona; `-u` en BSD espera recibir un uid a continuacion y emite `illegal argument`.
