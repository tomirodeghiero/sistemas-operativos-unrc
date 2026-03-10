# Resolucion 4 - Practico 1

Ejercicio 4: escribir un shell script con los comandos del ejercicio anterior, probar con `sh`, luego agregar linea de interprete y dar permisos de ejecucion.

Tomamos como base el comando del ejercicio 3:

```bash
cat correos1.txt correos2.txt | sort | uniq > correos_ordenados_sin_duplicados.txt
```

## Archivos de esta carpeta

- `correos1.txt`
- `correos2.txt`
- `procesar_correos_sin_interprete.sh`
- `procesar_correos.sh`

## 4.1 Probar script con `sh` (sin linea de interprete)

Implementacion: ver `procesar_correos_sin_interprete.sh`.

Ejecucion:

```bash
sh procesar_correos_sin_interprete.sh
```

Explicacion:

- Como llamamos explicitamente a `sh`, no hace falta que el archivo tenga shebang.
- El script concatena, ordena, elimina duplicados y deja el resultado en `correos_ordenados_sin_duplicados.txt`.

## 4.2 Agregar linea de interprete y permisos

Ahora usamos `procesar_correos.sh` (ver archivo en esta carpeta), que ya incluye linea interprete.

Dar permisos de ejecucion:

```bash
chmod +x procesar_correos.sh
```

Ejecucion directa:

```bash
./procesar_correos.sh
```

Explicacion:

- `#!/bin/sh` indica al sistema que interprete debe ejecutar el script.
- `chmod +x` habilita el bit de ejecucion.
- Luego se puede ejecutar con `./script` sin anteponer `sh`.

## Verificacion

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
