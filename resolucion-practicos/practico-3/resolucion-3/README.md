# Resolucion 3 - Practico 3

Ejercicio 3: usar `pstree` para visualizar el arbol de procesos del sistema y determinar el primer proceso lanzado y su PID.

## Instalacion de pstree en macOS

macOS no trae `pstree` por defecto (`zsh: command not found: pstree`). Se instala via Homebrew:

```bash
brew install pstree
```

Verificacion:

```bash
which pstree
# /opt/homebrew/bin/pstree
```

Nota: el `pstree` que instala Homebrew en macOS es la implementacion de **Fred Hucht (v2.40)**, que tiene **opciones distintas** al `pstree` de Linux (PSmisc). En particular los PIDs se muestran **siempre** por defecto y el flag `-p` significa "filtrar por un PID especifico", no "mostrar PIDs".

## Arbol completo de procesos

Sin argumentos imprime el arbol entero con PIDs incluidos:

```bash
pstree
```

Salida (primeras lineas, ejemplo real):

```text
-+= 00001 root /sbin/launchd
 |--= 00312 root /usr/libexec/logd
 |--= 00313 root /usr/libexec/smd
 |--= 00314 root /usr/libexec/UserEventAgent (System)
 |--= 00316 root /System/Library/Frameworks/CoreServices.framework/.../fseventsd
 ...
```

- `-+=` y `|--=` son los conectores del arbol.
- Los numeros de 5 digitos son los PIDs.
- El `=` al final del prefijo indica que el proceso es un *process group leader*.

## Opciones utiles

```bash
pstree -w                   # wide: no trunca lineas largas
pstree -g 3                 # conectores en UTF-8 (mas prolijos)
pstree -u "$USER"           # solo ramas que contengan procesos del usuario
pstree -U                   # omite ramas que solo tienen procesos root
pstree -s bash              # solo ramas que contengan la cadena "bash"
pstree -p <PID>             # solo ramas que contengan ese PID
pstree -l 3                 # profundidad maxima de 3 niveles
pstree <PID>                # arranca el arbol desde ese PID (no filtra)
```

Ejemplo util: ver todos los procesos del usuario con grafica UTF-8:

```bash
pstree -g 3 -u "$USER"
```

## Primer proceso del sistema

El primer proceso lanzado por el kernel en espacio de usuario tiene **PID 1** y es el ancestro de todos los demas. En `pstree`, es siempre el nodo raiz (`-+= 00001 ...`):

```bash
pstree | head -n 1
```

```text
-+= 00001 root /sbin/launchd
```

Confirmacion con `ps`:

```bash
ps -p 1 -o pid,ppid,user,comm
```

```text
  PID  PPID USER COMM
    1     0 root /sbin/launchd
```

## Por que PID 1

- El kernel arranca un proceso interno con `PID 0` (scheduler/swapper, no visible como proceso de usuario).
- Luego ejecuta el programa pasado por el bootloader como `init` (o equivalente). Ese programa recibe `PID 1`.
- El proceso `PID 1` se encarga de lanzar el resto del espacio de usuario (servicios, login, etc.).
- Si un proceso queda huerfano (su padre muere), el kernel lo re-asigna a `PID 1`, que luego hace `wait()` para evitar zombies.

Variantes segun el sistema:

| Sistema                  | Proceso PID 1         |
|--------------------------|-----------------------|
| macOS                    | `launchd`             |
| Linux moderno            | `systemd`             |
| Linux tradicional        | `init` (SysV, BusyBox)|
| FreeBSD                  | `init`                |
