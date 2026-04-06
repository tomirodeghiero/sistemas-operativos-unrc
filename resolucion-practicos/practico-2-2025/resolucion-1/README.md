# Practico 2 - Ejercicio 1

## Que pide el ejercicio
Trabajar con `counter.c`, crear `counter.dat` con valor inicial `0`, ejecutar pruebas secuenciales y concurrentes, explicar el resultado y corregir la concurrencia usando `flock()`.

## Archivos usados
- `counter.c` (version base)
- `counter_flock.c` (version corregida con lock)
- `counter.dat` (archivo compartido)

## Preparacion
Desde la carpeta del practico:

```bash
cd resolucion-practicos/practico-2
```

Inicializo el archivo:

```bash
echo 0 > counter.dat
```

Compilacion:

```bash
gcc -Wall -Wextra -O2 -o counter counter.c
gcc -Wall -Wextra -O2 -o counter_flock counter_flock.c
```

## Prueba 1: ejecucion secuencial
Comando:

```bash
./counter ; ./counter
cat counter.dat
```

Resultado esperado:
- Cada proceso incrementa 1000 veces.
- Dos ejecuciones seguidas => total 2000.
- En este programa se imprime con 5 digitos, asi que normalmente queda `02000`.

Esto en secuencial sale bien porque no hay intercalado entre procesos.

## Prueba 2: ejecucion concurrente
Comando:

```bash
echo 0 > counter.dat
./counter & ./counter
wait
cat counter.dat
```

Para verlo mejor, se puede repetir:

```bash
for i in $(seq 1 10); do
  echo 0 > counter.dat
  ./counter & ./counter
  wait
  echo "run $i => $(cat counter.dat)"
done
```

## Analisis del problema (sin lock)
Cuando no hay proteccion, el valor final puede quedar menor a 2000 (y variar entre corridas).

La razon es que la operacion sobre el archivo no es atomica:
1. leer
2. convertir a entero
3. sumar 1
4. escribir

Si dos procesos leen el mismo valor antes de escribir, uno pisa el resultado del otro (lost update). Por eso se pierden incrementos.

Ejemplo rapido:
- A lee 42
- B lee 42
- A escribe 43
- B escribe 43

Deberia terminar en 44, pero termina en 43.

## Correccion con `flock()`
La solucion es encerrar toda la seccion critica con lock exclusivo:

```c
flock(fd, LOCK_EX);
/* read -> increment -> write */
flock(fd, LOCK_UN);
```

En `counter_flock.c` esto esta aplicado alrededor de todo el bloque leer/modificar/escribir.

## Verificacion de la version corregida
Comando:

```bash
echo 0 > counter.dat
./counter_flock & ./counter_flock
wait
cat counter.dat
```

Resultado esperado: `02000` en todas las corridas.

## Conclusiones
- Secuencial: da bien (2000).
- Concurrente sin lock: resultado incorrecto/no deterministico.
- Concurrente con `flock()`: resultado correcto y estable.

Con esto queda resuelto el punto de concurrencia del ejercicio.
