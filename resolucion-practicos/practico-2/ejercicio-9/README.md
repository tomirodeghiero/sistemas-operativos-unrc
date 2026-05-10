# Práctico 2 - Ejercicio 9

**Consigna:** escribir un archivo `Makefile` para compilar `main.c` y
`hello.c` (originales) y generar `myprog` con el comando `make`.

## Marco teórico

Las notas del curso (capítulo *Build systems*) explican que en un
proyecto real los pasos del *toolchain* (preprocesador, compilador,
ensamblador y linker) no se invocan a mano: se automatizan con un
*build system*. El más clásico en UNIX es **GNU make**, basado en un
archivo `Makefile` que describe los **objetivos** (*targets*), sus
**dependencias** y los **comandos** para construirlos.

La forma básica de una regla es:

```makefile
target: dependencies
<TAB>command
```

`make` construye un grafo de dependencias y, comparando los
*timestamps* de cada archivo, decide qué reglas disparar. Si el
objetivo es más nuevo que todas sus dependencias, no se reconstruye:
de ahí la **compilación incremental**.

Las notas también muestran cómo compactar las reglas usando
**variables automáticas**:

| Variable | Significado |
|----------|-------------|
| `$@`     | Nombre del *target* de la regla. |
| `$^`     | Lista completa de dependencias. |
| `$<`     | Primera dependencia (la "fuente"). |

y reglas de **patrón** (`%.o: %.c`) para evitar repetir la misma
acción para cada archivo fuente.

## Estructura del proyecto

Los originales del práctico (`main.c` y `hello.c`) viven en el
directorio padre. Para mantenernos consistentes con los demás
ejercicios, el `Makefile` se ubica dentro de `ejercicio-9/` y referencia
los fuentes vía `VPATH`:

```text
practico-2/
├── main.c          <- fuente original
├── hello.c         <- fuente original
├── ejercicio-9/
│   ├── Makefile
│   └── README.md   (este archivo)
```

## Makefile usado

```makefile
# VPATH le dice a make donde buscar los archivos fuente cuando no
# estan en el directorio actual.
VPATH = ..

CC     = gcc
CFLAGS = -Wall -Wextra -pedantic

# Objetivo por defecto: myprog
myprog: main.o hello.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

hello.o: hello.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f main.o hello.o myprog
```

Decisiones de diseño:

- **`VPATH = ..`** indica a `make` que busque las dependencias
  (`main.c`, `hello.c`) un nivel arriba si no las encuentra en el
  directorio actual. Es la forma idiomática de separar fuentes y
  artefactos de compilación.
- **Variables `CC` y `CFLAGS`** evitan repetir el comando del
  compilador y permiten cambiar opciones desde un único lugar.
- **Variables automáticas (`$@`, `$^`, `$<`)** hacen que las reglas
  sean más concisas y reutilizables.
- **`.PHONY: clean`** declara que `clean` no es un archivo, sino un
  *objetivo ficticio*. Si existiera un archivo llamado `clean`, sin
  esto `make` lo consideraría "ya construido" y no ejecutaría la
  regla.

## Cómo se construye el grafo de dependencias

```
+--------+
| myprog |
+--------+
   ^
   |
+--+--------------+
|                 |
+--------+   +---------+
| main.o |   | hello.o |
+--------+   +---------+
   ^             ^
   |             |
+--------+   +---------+
| main.c |   | hello.c |
+--------+   +---------+
```

`make myprog` requiere `main.o` y `hello.o`; `main.o` requiere
`main.c`; `hello.o` requiere `hello.c`. `make` recorre el grafo en
post-orden: primero compila los `.o` que falten o estén
desactualizados y solo entonces invoca al linker.

## Ejecución

Desde `ejercicio-9/`:

```bash
$ make clean
rm -f main.o hello.o myprog

$ make
gcc -Wall -Wextra -pedantic -c ../main.c -o main.o
gcc -Wall -Wextra -pedantic -c ../hello.c -o hello.o
gcc -Wall -Wextra -pedantic -o myprog main.o hello.o

$ ./myprog
Hello world
```

## Compilación incremental: el valor real de `make`

Si se vuelve a invocar `make` sin tocar nada, no hace nada porque
`myprog` es más nuevo que sus dependencias:

```bash
$ make
make: `myprog' is up to date.
```

Si se modifica solo un archivo fuente, `make` rehace **únicamente lo
necesario**:

```bash
$ touch ../hello.c
$ make
gcc -Wall -Wextra -pedantic -c ../hello.c -o hello.o
gcc -Wall -Wextra -pedantic -o myprog main.o hello.o
```

`main.o` no se recompiló porque `main.c` no cambió. Esto es
exactamente lo que las notas del curso muestran como motivación
central de `make`: en proyectos grandes, recompilar todo desde cero es
inviable; `make` escala porque solo toca lo que tenga que tocar.

## Versión más compacta (alternativa con regla de patrón)

La misma funcionalidad puede escribirse aún más breve usando una regla
de patrón `%.o : %.c`, que define cómo construir cualquier `.o` a
partir de un `.c`:

```makefile
VPATH  = ..
CC     = gcc
CFLAGS = -Wall -Wextra -pedantic

myprog: main.o hello.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f main.o hello.o myprog
```

Esta forma es la que aparece en la sección final de las notas del
curso. La versión con reglas explícitas (la que está activa en el
`Makefile` de este ejercicio) es más didáctica porque hace visibles
los tres comandos que el linker y el compilador van a ejecutar.

## Conexión con la teoría

El `Makefile` automatiza exactamente las mismas etapas del *toolchain*
que se vieron en los ejercicios 1–4:

| Comando del Makefile                          | Etapa                       |
|-----------------------------------------------|-----------------------------|
| `gcc -Wall ... -c ../main.c -o main.o`        | Preprocesar + compilar + ensamblar (`main.c → main.o`) |
| `gcc -Wall ... -c ../hello.c -o hello.o`      | Preprocesar + compilar + ensamblar (`hello.c → hello.o`) |
| `gcc -Wall ... -o myprog main.o hello.o`      | Linking (genera el ejecutable) |

`make` no agrega capacidades nuevas al proceso de compilación: lo que
agrega es **automatización con conocimiento de dependencias**. Es la
herramienta que permite mantener un proyecto razonablemente grande con
compilaciones rápidas y reproducibles.
