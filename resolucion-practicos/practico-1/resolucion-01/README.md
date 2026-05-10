# Resolución 1 — Práctico 1

**Ejercicio 1 (incisos a–m):** familiarización con el shell, el modelo de
procesos, el sistema de archivos y la API de argumentos en C.

## Marco teórico aplicable

Las notas del curso describen al *shell* como un intérprete REPL (read,
execute, print, loop) que permite al usuario interactuar con el SO mediante
comandos. Cada comando que se ejecuta da lugar a un nuevo proceso, creado por
el shell vía `fork()` + `exec()`. El proceso recibe argumentos en su función
`main(argc, argv, envv)` y un conjunto de *variables de ambiente* heredadas
del shell. Estos conceptos serán transversales a todo el práctico.

## 1.a — Procesos en ejecución y PID del primer proceso

```bash
ps -e -o pid,ppid,state,comm | head -n 20
```

`ps -e` lista todos los procesos del sistema; `-o` permite seleccionar las
columnas relevantes (PID, PPID, estado y nombre del comando). El proceso con
**PID 1** es el primer proceso lanzado por el kernel apenas terminó la
inicialización del SO y es el ancestro de todos los procesos de usuario:

- En sistemas Linux modernos suele ser `systemd` (o `init` en variantes más
  clásicas).
- En macOS se trata de `launchd`.

Conceptualmente, el kernel inicializa los subsistemas y luego *lanza el
primer proceso preconfigurado* (Notas del curso, capítulo 1, sección "Inicio
del SO"). Ese primer proceso es responsable de poner en marcha al resto del
espacio de usuario (terminales, *daemons*, sesiones de login, etc.).

## 1.b — Usuarios conectados y usuario actual

```bash
who
who am i
whoami
```

- `who`: lista todas las sesiones abiertas en el sistema (línea por
  terminal/usuario).
- `who am i`: muestra únicamente la sesión asociada a la terminal actual.
- `whoami`: imprime el *nombre efectivo* (UID efectivo) del proceso shell.

La diferencia entre los tres comandos refleja la naturaleza *multiusuario* de
los sistemas tipo UNIX: el SO mantiene una tabla de sesiones (`utmp`/`wtmp`)
que asocia terminales con usuarios autenticados.

## 1.c — Directorio corriente (al menos dos formas)

```bash
pwd
echo "$PWD"
```

`pwd` (*print working directory*) es un comando externo/builtin que consulta
al kernel el directorio asociado al proceso shell. El shell mantiene además
una variable de ambiente `PWD` con el mismo valor; `echo "$PWD"` la imprime.
Un tercer enfoque es leer el enlace simbólico que el kernel mantiene por
proceso:

```bash
readlink /proc/$$/cwd   # solo en Linux
```

## 1.d — Directorio home del usuario

```bash
echo "$HOME"
cd ~
pwd
```

`HOME` es una variable de ambiente fijada por el sistema en el momento del
*login* (a partir de `/etc/passwd` o `/etc/login.defs`). El símbolo `~`
es un atajo que el shell expande a `$HOME` antes de pasar los argumentos al
programa.

## 1.e — Metadatos de archivos en la carpeta corriente y en `/`

```bash
ls -la .
ls -la /
stat README.md /
```

`ls -la` muestra entradas ocultas y los metadatos básicos: permisos, dueño,
grupo, tamaño y fecha. `stat` agrega información de bajo nivel (inodo,
cantidad de hard links, timestamps de acceso/modificación/cambio, dispositivo
contenedor). El detalle es relevante porque, según las notas del curso, los
archivos en UNIX son *secuencias de bytes* identificadas por un inodo y los
metadatos viven en la entrada del directorio + el inodo, no en los datos
mismos.

## 1.f — Listar contenido de un archivo (sin editor)

```bash
cat README.md
more README.md
less README.md
```

- `cat`: vuelca todo el contenido por la salida estándar de una vez.
- `more`/`less`: paginadores; `less` es más rico (permite navegar hacia
  atrás).

Internamente, todos invocan `read()` sobre el descriptor asociado al archivo
y `write()` sobre `stdout` (descriptor 1). Es el mecanismo descripto en el
capítulo 2 de las notas para la operación de E/S sobre archivos.

## 1.g — Variables de ambiente de la sesión

```bash
env | sort
```

Las variables de ambiente forman parte del *contexto* del proceso shell y se
pasan a cada proceso hijo a través del tercer parámetro de `main`
(`char *envv[]`). Algunas relevantes:

| Variable | Significado                                                       |
|----------|-------------------------------------------------------------------|
| `USER`   | nombre del usuario autenticado.                                   |
| `HOME`   | directorio de trabajo del usuario.                                |
| `PWD`    | directorio actual del shell.                                      |
| `SHELL`  | intérprete configurado para el usuario.                           |
| `PATH`   | lista de directorios donde el shell busca ejecutables.            |
| `LANG`   | configuración regional (idioma, codificación).                    |

## 1.h — Directorios donde el shell busca comandos (`PATH`)

```bash
echo "$PATH" | tr ':' '\n'
```

`PATH` es una lista de directorios separados por `:`. Cuando el usuario
ingresa un comando sin ruta, el shell recorre estos directorios en orden y
ejecuta el primer archivo regular con permiso de ejecución cuyo nombre
coincida. Por seguridad, el directorio actual `.` *no* suele estar incluido,
de ahí la necesidad de invocar binarios locales con `./hello` (inciso m).

## 1.i — Ubicación de un comando con `which`

```bash
which cat
```

`which` consulta el `PATH` y reporta la primera coincidencia. Es útil para
detectar conflictos cuando hay múltiples versiones instaladas (por ejemplo,
una de Homebrew y otra del sistema).

## 1.j — Crear un archivo con al menos tres comandos

```bash
touch archivo.txt
echo "Primera linea" > archivo.txt
date "+%Y-%m-%d %H:%M:%S" >> archivo.txt
echo "Tercera linea" >> archivo.txt
cat archivo.txt
```

- `touch` crea (o actualiza el `mtime`) del archivo.
- `>` *redirige y trunca*: el shell, antes de invocar `exec()`, abre el
  archivo con `O_WRONLY|O_CREAT|O_TRUNC` y reasigna el descriptor 1 (stdout)
  con `dup2`.
- `>>` *abre en modo append* (`O_APPEND`).
- `cat` muestra el resultado.

Esta secuencia ilustra la idea del listado 5 de las notas (redirección de
salida del proceso hijo) aplicada al shell interactivo.

## 1.k — Crear un enlace simbólico

```bash
ln -s archivo.txt archivo_link.txt
ls -li archivo.txt archivo_link.txt
```

Un *symbolic link* es un archivo cuyo contenido es la *ruta* del destino. Con
`ls -li` se observa que el enlace tiene su propio inodo (≠ del original) y
que aparece la flecha `->` indicando el blanco.

## 1.l — Eliminar el enlace simbólico y comparar con hard link

```bash
rm archivo_link.txt
ls -li archivo.txt
ln archivo.txt archivo_hard.txt
ls -li archivo.txt archivo_hard.txt
```

Borrar el symlink **no** afecta al archivo original: solo se elimina la
entrada de directorio que contenía la ruta. Un *hard link* (`ln` sin `-s`),
en cambio, crea otra entrada de directorio que apunta al **mismo inodo**. El
contador de referencias (`st_nlink`) crece, y el archivo solo desaparece del
disco cuando ese contador llega a cero.

| Característica           | Symbolic link               | Hard link                          |
|--------------------------|-----------------------------|------------------------------------|
| Tipo de referencia       | Por ruta (texto)            | Por inodo                          |
| Inodo propio             | Sí (distinto del original)  | No (comparte el inodo del archivo) |
| Sobrevive al rename      | Solo si la ruta sigue válida| Sí                                 |
| Sobrevive al borrado del original | No (queda *roto*)   | Sí (mientras quede algún enlace)   |
| Cruza filesystems        | Sí                          | No                                 |
| Se permite sobre directorios | Sí                      | Generalmente no                    |

## 1.m — Programa en C que muestra los argumentos recibidos

Implementación: ver [`hello.c`](./hello.c).

```bash
gcc -Wall -Wextra -o hello hello.c
./hello arg1 arg2 arg3
```

Salida esperada:

```text
argc = 4
argv[0] = ./hello
argv[1] = arg1
argv[2] = arg2
argv[3] = arg3
```

**Justificación.** El compilador `gcc` produce un ejecutable ELF que, al ser
invocado por el shell, sigue el patrón `fork()` + `exec()`: el shell crea un
hijo con `fork()`, el hijo reemplaza su imagen de memoria con el código de
`hello` mediante `execve()` y comienza a ejecutar desde `main`. La llamada
recibe `argc` (cantidad de argumentos) y `argv` (arreglo de strings, con
`argv[0]` igual a la forma en que se invocó al programa). Este es exactamente
el modelo descripto en el capítulo 2 de las notas (Listado 1).

**¿Por qué `./hello arg1 ... argn`?**

Cuando el usuario tipea un nombre sin slashes, el shell asume que se trata de
un comando externo y lo busca **únicamente** en los directorios listados en
`PATH`. El directorio actual `.` no suele estar incluido para evitar el
ataque clásico de un binario malicioso colocado en `cwd`. Para indicarle al
shell que el ejecutable es relativo al directorio corriente se utiliza el
prefijo `./`, que es interpretado como ruta relativa explícita y, por lo
tanto, se omite la búsqueda en `PATH`.
