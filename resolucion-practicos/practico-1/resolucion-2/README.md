# Resolución 2 — Práctico 1

**Ejercicio 2:** comandos de shell sobre redirección, secuencia y
concurrencia.

## Marco teórico aplicable

Las notas del curso (capítulo 2, secciones *Operadores de redirección* y
*Ejecución secuencial de comandos*) describen los siguientes mecanismos:

- **Redirección:** todo proceso hereda tres descriptores abiertos a la
  terminal — `0` (stdin), `1` (stdout) y `2` (stderr). El shell, antes de
  hacer `exec()` del comando, puede *reasignar* esos descriptores a archivos
  usando los operadores `<`, `>` y `>>`. Esto se implementa con `open()` +
  `dup2()` + `close()`.
- **Operadores secuenciales:** `;` ejecuta una lista de comandos uno tras
  otro; `&&` ejecuta el segundo solo si el primero terminó con éxito (`$? =
  0`); `||` ejecuta el segundo solo si el primero terminó con error.
- **Pipes (`|`):** se resuelven en el ejercicio 3.

Los archivos de ejemplo `correos1.txt` y `correos2.txt` contienen direcciones
de correo (una por línea) con duplicados intencionales para que `sort | uniq`
tenga efecto observable.

## 2.a — Lista ordenada y sin duplicados (con archivos temporales en `/tmp`)

```bash
cat correos1.txt correos2.txt > /tmp/correos_todos.txt
sort /tmp/correos_todos.txt > /tmp/correos_ordenados.txt
uniq /tmp/correos_ordenados.txt > correos_ordenados_sin_duplicados.txt
```

**Justificación paso por paso:**

1. `cat correos1.txt correos2.txt > /tmp/correos_todos.txt` invoca al
   programa `cat`, que abre cada archivo, lee secuencialmente y vuelca los
   bytes a su salida estándar. El operador `>` redirige esa salida hacia el
   archivo temporal en `/tmp`.
2. `sort` ordena las líneas alfabéticamente. Es un paso necesario porque
   `uniq` solo compara líneas *consecutivas*.
3. `uniq` elimina líneas repetidas adyacentes. Combinado con `sort`, equivale
   a eliminar todos los duplicados globales.
4. El archivo final queda fuera de `/tmp` (en la carpeta de la resolución)
   para conservarlo como evidencia del ejercicio.

## 2.b — Lo mismo en una sola línea con el operador secuencial `;`

```bash
cat correos1.txt correos2.txt > /tmp/correos_todos.txt ; sort /tmp/correos_todos.txt > /tmp/correos_ordenados.txt ; uniq /tmp/correos_ordenados.txt > correos_ordenados_sin_duplicados.txt
```

El operador `;` *no propaga* información sobre el éxito de cada comando: la
secuencia avanza incluso si un paso intermedio falla. Si quisiéramos detener
la ejecución al primer error usaríamos `&&` en su lugar. El enunciado exige
explícitamente *operador secuencial*, por eso la solución usa `;`.

## 2.c — Filtrar y contar las direcciones terminadas en `google.com`

```bash
grep -E '@google\.com$' correos_ordenados_sin_duplicados.txt | wc -l
```

**Análisis del comando:**

- `grep -E` interpreta el patrón como expresión regular extendida.
- `@google\.com$`:
  - `@google`: coincidencia literal.
  - `\.`: punto literal (sin la barra invertida sería *cualquier carácter*).
  - `com`: literal.
  - `$`: ancla *fin de línea*; obliga a que la dirección termine exactamente
    ahí.
- `|` redirige la salida de `grep` a la entrada de `wc -l`, que cuenta líneas
  recibidas.

Con los archivos de ejemplo provistos, el resultado es:

```text
3
```

## Limpieza opcional de los temporales

```bash
rm -f /tmp/correos_todos.txt /tmp/correos_ordenados.txt
```

El uso de `/tmp` es razonable porque el SO se encarga de limpiar su contenido
periódicamente (en muchas distribuciones, en cada reinicio).
