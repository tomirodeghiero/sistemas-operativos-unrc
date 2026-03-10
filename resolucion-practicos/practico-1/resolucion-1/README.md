# Resolucion 1 - Practico 1

Ejercicio 1 (incisos `a` a `m`): comandos, entorno de shell, archivos, enlaces y argumentos en C.

## 1.a Procesos en ejecucion y PID del primer proceso

Comando:

```bash
ps -e -o pid,ppid,state,comm | head -n 20
```

Explicacion:

`ps -e` lista todos los procesos activos. Con `-o` mostramos columnas utiles para analizar jerarquia (`PPID`) y estado (`state`). El proceso con `PID 1` es el primer proceso de espacio de usuario iniciado por el kernel y actua como ancestro de muchos otros procesos.

Interpretacion esperada:

- En Linux, `PID 1` suele ser `systemd` o `init`.
- En macOS, `PID 1` suele ser `launchd`.

## 1.b Usuarios conectados y usuario actual

Comandos:

```bash
who
who am i
whoami
```

Explicacion:

`who` muestra todas las sesiones abiertas. `who am i` se enfoca en tu terminal actual. `whoami` muestra el usuario efectivo con el que se estan ejecutando tus procesos en esa shell.

## 1.c Directorio corriente (dos formas)

Comandos:

```bash
pwd
echo "$PWD"
```

Explicacion:

`pwd` consulta el directorio actual. `PWD` es una variable de ambiente mantenida por el shell con ese mismo valor. Ver ambos ayuda a validar que el contexto de trabajo es correcto.

## 1.d Directorio home del usuario

Comandos:

```bash
echo "$HOME"
cd ~
pwd
```

Explicacion:

`HOME` representa el directorio personal del usuario. El simbolo `~` es un atajo de shell a ese directorio. Con `cd ~` seguido de `pwd` verificas la ubicacion real del home.

## 1.e Metadatos de archivos en carpeta actual y carpeta raiz

Comandos:

```bash
ls -la .
ls -la /
stat -x README.md /
```

Explicacion:

`ls -la` muestra entradas ocultas y metadatos basicos: permisos, duenio, grupo, tamano y fecha. `stat` agrega detalle tecnico, por ejemplo inode, cantidad de enlaces y timestamps de acceso/modificacion/cambio.

## 1.f Ver contenido de archivo y por paginas

Comandos:

```bash
cat README.md
more README.md
less README.md
```

Explicacion:

`cat` imprime todo el archivo de una sola vez. `more` y `less` paginan la salida, algo util para archivos largos. `less` es mas flexible porque permite desplazarte hacia arriba y hacia abajo.

## 1.g Variables de ambiente de la sesion

Comando:

```bash
env | sort
```

Explicacion:

Las variables de ambiente forman parte del contexto del proceso shell y se heredan a procesos hijos. Con `env` las listamos y con `sort` las ordenamos para facilitar lectura y analisis.

Variables importantes:

- `USER`: usuario actual.
- `HOME`: directorio personal.
- `PWD`: directorio actual.
- `SHELL`: interprete de comandos.
- `PATH`: rutas de busqueda de ejecutables.

## 1.h Directorios en los que shell busca comandos (`PATH`)

Comando:

```bash
echo "$PATH" | tr ':' '\n'
```

Explicacion:

`PATH` contiene directorios separados por `:`. El shell recorre esos directorios en orden para resolver nombres de comandos sin ruta explicita. Separarlos por linea facilita ver el orden real de busqueda.

## 1.i Ubicacion de un comando con `which`

Comando:

```bash
which cat
```

Explicacion:

`which` indica el ejecutable que el shell encontrara para ese comando segun el `PATH` vigente. Sirve para detectar conflictos cuando hay varias versiones instaladas.

Ejemplo:

```text
/bin/cat
```

## 1.j Crear un archivo usando al menos tres comandos

Comandos:

```bash
touch archivo.txt
echo "Primera linea" > archivo.txt
date "+%Y-%m-%d %H:%M:%S" >> archivo.txt
echo "Tercera linea" >> archivo.txt
cat archivo.txt
```

Explicacion:

`touch` crea el archivo (si no existe). `>` redirige y sobreescribe contenido. `>>` redirige agregando al final. `cat` confirma el resultado final.

## 1.k Crear un enlace simbolico

Comandos:

```bash
ln -s archivo.txt archivo_link.txt
ls -li archivo.txt archivo_link.txt
```

Explicacion:

Un symbolic link es una referencia por ruta al archivo destino. No copia datos. Con `ls -li` se ve que el enlace simbolico tiene inode propio y muestra la flecha `->` al archivo real.

## 1.l Eliminar enlace simbolico y comparar con hard link

Comandos:

```bash
rm archivo_link.txt
ls -li archivo.txt
ln archivo.txt archivo_hard.txt
ls -li archivo.txt archivo_hard.txt
```

Explicacion:

Eliminar el symbolic link no elimina el archivo original, solo borra la referencia. En cambio, un hard link (`ln` sin `-s`) crea otra entrada de directorio al mismo inode.

Diferencias clave:

- Symbolic link: apunta por ruta, puede quedar roto si el destino cambia.
- Hard link: apunta al inode, no se rompe por renombrar el archivo.
- Symbolic link: puede cruzar sistemas de archivos.
- Hard link: normalmente no cruza sistemas de archivos.

## 1.m Programa en C que muestra argumentos y ejecucion

Implementacion: ver archivo `hello.c` en esta carpeta.

Compilar y ejecutar:

```bash
gcc -Wall -Wextra -o hello hello.c
./hello arg1 arg2 arg3
```

Explicacion:

`argc` es la cantidad de argumentos recibidos. `argv` es el arreglo de strings con esos argumentos. `argv[0]` representa como se invoco el programa (`./hello` en este caso), por eso al pasar tres argumentos extra el total es `4`.

Salida esperada:

```text
argc = 4
argv[0] = ./hello
argv[1] = arg1
argv[2] = arg2
argv[3] = arg3
```

Por que usar `./hello`:

- El shell busca comandos solo en directorios listados en `PATH`.
- El directorio actual (`.`) normalmente no esta incluido por seguridad.
- `./hello` indica ruta relativa explicita al ejecutable en la carpeta actual.
