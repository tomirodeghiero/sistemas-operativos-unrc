# Resolución 3 — Práctico 1

**Ejercicio 3:** repetir el inciso 2.a (lista ordenada y sin duplicados) sin
usar archivos temporales, recurriendo a *pipes*.

## Marco teórico aplicable

Las notas del curso introducen el *pipe* como un **buffer acotado en el
kernel** que implementa el patrón productor/consumidor (capítulo 2, sección
*Pipes*). El comando de shell `cmd1 | cmd2`:

1. Crea un pipe vía `pipe(p)` antes del `fork()`.
2. Lanza `cmd1` y `cmd2` *concurrentemente* mediante dos llamadas `fork()` +
   `exec()`.
3. Redirige `stdout` de `cmd1` al extremo de escritura del pipe (`p[1]`) y
   `stdin` de `cmd2` al extremo de lectura (`p[0]`), usando `dup2()`.
4. Cierra los descriptores que cada proceso no usa: si quedan abiertos, el
   lector nunca verá `EOF` y se bloqueará indefinidamente.

El productor se bloquea cuando el buffer del kernel se llena; el consumidor
se bloquea cuando el buffer está vacío y aún hay escritores. Cuando todos
los escritores cierran su extremo, `read()` devuelve `0` (EOF).

## Solución

```bash
cat correos1.txt correos2.txt | sort | uniq > correos_ordenados_sin_duplicados.txt
```

**Análisis del flujo de datos:**

```
correos1.txt + correos2.txt
        │  cat (lee y concatena)
        ▼
   pipe (kernel)
        │  sort (ordena)
        ▼
   pipe (kernel)
        │  uniq (elimina duplicados consecutivos)
        ▼
correos_ordenados_sin_duplicados.txt
```

- `cat correos1.txt correos2.txt` envía el contenido concatenado a su
  stdout.
- El primer `|` redirige stdout de `cat` al stdin de `sort`.
- `sort` ordena alfabéticamente las líneas recibidas.
- El segundo `|` redirige stdout de `sort` al stdin de `uniq`.
- `uniq` filtra duplicados *consecutivos* (que ahora son todos los duplicados
  porque la entrada está ordenada).
- `>` finalmente redirige la salida de `uniq` al archivo final.

**Ventaja sobre el ejercicio 2.a:** los datos viajan por buffers en memoria
sin tocar el disco con archivos temporales. Esto reduce I/O, evita
condiciones de carrera con `/tmp` y libera al usuario de la limpieza
posterior. Además, los tres procesos ejecutan **concurrentemente** — el
kernel multiplexa la CPU entre ellos.

## Verificación

```bash
cat correos_ordenados_sin_duplicados.txt
```

Salida esperada:

```text
abraham@google.com
daniel@google.com
david@google.com
jacob@gmail.com
joseph@yahoo.com
moses@gmail.com
samuel@outlook.com
```
