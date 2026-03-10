# Resolucion 11 - Practico 1

Ejercicio 11: dado un arreglo de enteros de tamaño `N`, crear `p` procesos hijos para que cada uno calcule el minimo de su porcion (`N/p` elementos), enviarlo al padre por un pipe y que el padre calcule el minimo global.

## Archivo fuente

- `minimo_global_pipe.c`

## Enfoque

1. El programa recibe por argumentos:
   - `p` (cantidad de hijos),
   - los `N` valores del arreglo.
2. Valida:
   - `p > 0`,
   - `N >= p`,
   - `N % p == 0` (para dividir exactamente en bloques de `N/p`).
3. Crea **un pipe** compartido.
4. Crea `p` hijos con `fork()`:
   - cada hijo calcula el minimo local de su bloque,
   - escribe al pipe una estructura `{child_id, local_min}`,
   - finaliza.
5. El padre:
   - lee `p` resultados del pipe,
   - computa el minimo global,
   - espera a todos los hijos con `waitpid()`.

Implementacion: ver `minimo_global_pipe.c`.

## Compilacion

```bash
gcc -Wall -Wextra -o minimo_global_pipe minimo_global_pipe.c
```

## Ejecucion de ejemplo

```bash
./minimo_global_pipe 3 8 4 7 10 -2 5
```

Interpretacion:

- `p = 3`
- `N = 6`
- bloque por hijo: `N/p = 2` elementos
- bloques:
  - hijo 0: `[8, 4]` -> min `4`
  - hijo 1: `[7, 10]` -> min `7`
  - hijo 2: `[-2, 5]` -> min `-2`
- minimo global esperado: `-2`
