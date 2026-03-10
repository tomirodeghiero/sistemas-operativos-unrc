# Resolucion 3 - Practico 1

Ejercicio 3: repetir el item `2.a` pero sin usar archivos temporales (usando pipes).

## Objetivo

A partir de dos archivos de correos con posibles duplicados:

- `correos1.txt`
- `correos2.txt`

generar un archivo final ordenado y sin duplicados, evitando `/tmp` y cualquier archivo intermedio.

## Comando de resolucion (con pipes)

```bash
cat correos1.txt correos2.txt | sort | uniq > correos_ordenados_sin_duplicados.txt
```

## Explicacion

- `cat correos1.txt correos2.txt` concatena ambos archivos y envía el resultado por salida estándar.
- `| sort` recibe ese flujo y ordena las lineas.
- `| uniq` elimina repetidos consecutivos; como la entrada ya viene ordenada por `sort`, elimina todos los duplicados globales.
- `> correos_ordenados_sin_duplicados.txt` redirige el resultado final al archivo de salida.

Con esto se resuelve exactamente lo pedido: mismo resultado que `2.a` pero sin temporales.

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
