# Resolucion 3 - Practico 3

Ejercicio 3: usar `pstree` para visualizar el arbol de procesos del sistema y determinar cual es el primer proceso lanzado y su PID.

## Instalar `pstree` en macOS

macOS no trae `pstree` por defecto (`zsh: command not found: pstree`), asi que primero hay que instalarlo via Homebrew:

```bash
brew install pstree
which pstree
# /opt/homebrew/bin/pstree
```

Hay un detalle a tener presente: el `pstree` que distribuye Homebrew para macOS es la implementacion de **Fred Hucht (v2.40)**, no la de PSmisc que se usa en Linux. Las opciones son distintas. La diferencia mas notoria es que en este `pstree` los PIDs se muestran siempre por defecto y `-p` significa "filtrar por un PID", no "mostrar los PIDs". A la hora de copiar comandos del manual de Linux, conviene tenerlo en cuenta.

## Ver el arbol completo

Sin argumentos imprime todo el arbol con los PIDs incluidos:

```bash
pstree
```

Las primeras lineas son algo asi:

```text
-+= 00001 root /sbin/launchd
 |--= 00312 root /usr/libexec/logd
 |--= 00313 root /usr/libexec/smd
 |--= 00314 root /usr/libexec/UserEventAgent (System)
 |--= 00316 root /System/Library/Frameworks/CoreServices.framework/.../fseventsd
 ...
```

Los conectores `-+=` y `|--=` arman la jerarquia, los numeros de cinco digitos son los PIDs y el `=` final marca a los lideres de grupo de procesos (process group leaders).

## Algunas opciones que vienen bien

```bash
pstree -w                   # wide: no trunca lineas largas
pstree -g 3                 # conectores en UTF-8 (queda mas prolijo)
pstree -u "$USER"           # solo ramas que tocan procesos del usuario
pstree -U                   # esconde ramas que solo tienen procesos root
pstree -s bash              # ramas que contengan la cadena "bash"
pstree -p <PID>             # filtra por un PID concreto
pstree -l 3                 # limita a 3 niveles de profundidad
pstree <PID>                # arranca el arbol desde ese PID (no filtra)
```

Una combinacion util en el dia a dia es ver solo lo "mio" con grafica linda:

```bash
pstree -g 3 -u "$USER"
```

## El primer proceso del sistema

El primer proceso de espacio de usuario que arranca el kernel siempre tiene **PID 1** y es ancestro de todos los demas. En la salida de `pstree` aparece como nodo raiz:

```bash
pstree | head -n 1
```

```text
-+= 00001 root /sbin/launchd
```

Se confirma con `ps`:

```bash
ps -p 1 -o pid,ppid,user,comm
```

```text
  PID  PPID USER COMM
    1     0 root /sbin/launchd
```

Notar que el PPID de PID 1 es 0: ese 0 corresponde al proceso interno del kernel (scheduler/swapper) que no aparece en el espacio de usuario, y por eso no es visible con `ps`.

## Por que justamente PID 1

La logica es la siguiente: cuando el kernel termina de inicializarse arma manualmente un proceso en espacio de usuario y lo lanza, que es el primero al que le toca PID 1. Ese proceso (`launchd`, `systemd`, `init`, etc., segun el sistema) se encarga de levantar el resto del userland: servicios, login manager, daemons. Como bonus, el kernel le re-asigna como hijos a cualquier proceso que quede huerfano (es decir, cuyo padre haya muerto antes de hacer `wait()`), con la intencion de que PID 1 los recoja con `wait()` y evitar zombies acumulados.

Quien cumple ese rol depende del sistema:

| Sistema                  | Proceso PID 1         |
|--------------------------|-----------------------|
| macOS                    | `launchd`             |
| Linux moderno            | `systemd`             |
| Linux tradicional        | `init` (SysV, BusyBox)|
| FreeBSD                  | `init`                |

En cualquier caso, el rol conceptual es el mismo: ser el ancestro comun de todo el espacio de usuario.
