# Resolución 4 — Práctico 1

**Ejercicio 4:** convertir el comando del ejercicio 3 en un *shell script*,
probarlo invocándolo con `sh` y luego con shebang + permisos de ejecución.

## Marco teórico aplicable

Un *shell script* es un archivo de texto que contiene una secuencia de
comandos para que el shell los lea e interprete uno por uno. Existen dos
formas de ejecutarlo:

1. **Pasarlo explícitamente al intérprete:** `sh script.sh`. En este caso,
   el shell que se invoca *lee y ejecuta* el archivo; el archivo no necesita
   permiso de ejecución porque el SO solo necesita poder leerlo.
2. **Hacerlo ejecutable directamente:** se le agrega como primera línea una
   *línea de intérprete* (también llamada *shebang*) `#!/bin/sh` y se le
   otorga el bit de ejecución con `chmod +x`. Cuando el usuario invoca
   `./script.sh`, el kernel detecta el shebang en `execve()` y lanza al
   intérprete indicado pasándole el script como argumento. Es exactamente el
   mecanismo descripto en el capítulo 2 de las notas para programas no
   compilados.

## Archivos de esta carpeta

- `correos1.txt`, `correos2.txt`: entradas de prueba.
- `procesar_correos_sin_interprete.sh`: script sin shebang, pensado para
  invocarse como `sh procesar_correos_sin_interprete.sh`.
- `procesar_correos.sh`: script con shebang, pensado para invocarse
  directamente.

## 4.1 — Probar el script con `sh` (sin shebang)

```bash
sh procesar_correos_sin_interprete.sh
```

El binario `sh` recibe el path del archivo, lo lee, lo interpreta y ejecuta
los comandos uno por uno. Como `sh` se invoca explícitamente, el archivo no
necesita permiso de ejecución.

## 4.2 — Agregar shebang y permisos de ejecución

```bash
chmod +x procesar_correos.sh
./procesar_correos.sh
```

**Lo que hace cada paso:**

- `chmod +x` activa el bit de ejecución (sobre el dueño, grupo y otros, o
  según se aplique). Sin este bit, el kernel rechaza la llamada `execve()`.
- `./procesar_correos.sh` invoca al ejecutable a través de la ruta relativa
  (recordar el inciso 1.m: `.` no está en `PATH`).
- En `execve()`, el kernel lee los primeros bytes del archivo, ve la
  secuencia mágica `#!`, extrae `/bin/sh` y lanza `/bin/sh` con el script
  como argumento.

## Recomendación adicional: `set -eu`

El script `procesar_correos.sh` incluye `set -eu`:

- `-e` aborta el script ante el primer error (exit status ≠ 0). Hace que el
  script falle ruidosamente en lugar de seguir con datos parciales.
- `-u` trata como error el uso de variables no definidas. Atrapa typos.

Estas opciones no son exigidas por el enunciado, pero son una buena práctica
defensiva en scripts no triviales.

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
