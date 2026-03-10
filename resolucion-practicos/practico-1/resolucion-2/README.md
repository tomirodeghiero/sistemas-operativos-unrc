# Resolucion 2 - Practico 1

Ejercicio 2: comandos de shell sobre redireccion, secuencia y concurrencia.

Esta carpeta incluye dos archivos de ejemplo:

- `correos1.txt`
- `correos2.txt`

Ambos tienen una direccion por linea y contienen duplicados, como pide el enunciado.

## 2.a Crear dos archivos y generar lista ordenada sin duplicados

Comandos:

```bash
# desde esta carpeta (resolucion-2)
cat correos1.txt correos2.txt > /tmp/correos_todos.txt
sort /tmp/correos_todos.txt > /tmp/correos_ordenados.txt
uniq /tmp/correos_ordenados.txt > correos_ordenados_sin_duplicados.txt
```

Explicacion:

1. `cat` concatena ambos archivos y guarda el resultado en un temporal en `/tmp`.
2. `sort` ordena alfabeticamente todas las lineas.
3. `uniq` elimina lineas repetidas consecutivas.  
   Importante: `uniq` funciona correctamente para "quitar duplicados globales" cuando la entrada ya viene ordenada.
4. El archivo final queda en la carpeta de la resolucion: `correos_ordenados_sin_duplicados.txt`.

Verificacion:

```bash
cat correos_ordenados_sin_duplicados.txt
```

## 2.b Igual que 2.a pero en un solo comando usando operador secuencial

Comando (una sola linea):

```bash
cat correos1.txt correos2.txt > /tmp/correos_todos.txt ; sort /tmp/correos_todos.txt > /tmp/correos_ordenados.txt ; uniq /tmp/correos_ordenados.txt > correos_ordenados_sin_duplicados.txt
```

Explicacion:

- El operador `;` ejecuta comandos en secuencia, de izquierda a derecha.
- Cada comando se ejecuta cuando termina el anterior.
- Este formato cumple lo pedido por el ejercicio: "un unico comando usando operador secuencial".

Nota:

- Si se quisiera cortar la ejecucion ante error, se usaria `&&` en lugar de `;`.  
  En este punto no se usa porque el enunciado pide operador secuencial.

## 2.c Filtrar y contar direcciones terminadas en `google.com`

Comando:

```bash
grep -E '@google\.com$' correos_ordenados_sin_duplicados.txt | wc -l
```

Explicacion:

- `grep -E` busca lineas que cumplan una expresion regular.
- `@google\.com$` significa:
  - `@google` literal.
  - `\.` punto literal (no "cualquier caracter").
  - `com` literal.
  - `$` fin de linea (asegura que termina exactamente en `google.com`).
- `wc -l` cuenta cuantas lineas pasaron el filtro.

Con los archivos de ejemplo de esta carpeta, el resultado esperado es:

```text
3
```

## Limpieza opcional de temporales

Comando:

```bash
rm -f /tmp/correos_todos.txt /tmp/correos_ordenados.txt
```
