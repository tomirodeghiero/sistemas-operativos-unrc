# Resolucion 7 - Practico 3

Ejercicio 7: tres procesos con las rafagas de CPU siguientes llegan al mismo tiempo a la ready queue:

| Proceso | Duracion de la proxima rafaga |
|---------|-------------------------------|
| P1      | 8                             |
| P2      | 3                             |
| P3      | 5                             |

Calcular el **tiempo de espera promedio** bajo FCFS (FIFO) y SJF (Shortest Job First), ambos no expropiativos.

## Definicion

**Tiempo de espera** de un proceso = tiempo desde que llega a la ready queue hasta que empieza a ejecutar (no incluye su propio tiempo de ejecucion ni I/O). Como los tres procesos llegan en t = 0, el tiempo de espera de cada uno coincide con su **tiempo de inicio** en la CPU.

## FCFS (orden de llegada: P1, P2, P3)

Se ejecutan en el orden en que aparecen: P1, luego P2, luego P3.

```text
        |  P1 (8)   |   P2 (3)  |   P3 (5)  |
 0           8          11          16
```

| Proceso | Inicio | Espera |
|---------|--------|--------|
| P1      | 0      | 0      |
| P2      | 8      | 8      |
| P3      | 11     | 11     |

Tiempo de espera promedio:

```text
(0 + 8 + 11) / 3 = 19 / 3 ≈ 6.33
```

## SJF (Shortest Job First)

Se ordenan por rafaga creciente: P2 (3), P3 (5), P1 (8).

```text
 |  P2 (3)  |   P3 (5)  |   P1 (8)   |
 0          3           8           16
```

| Proceso | Inicio | Espera |
|---------|--------|--------|
| P2      | 0      | 0      |
| P3      | 3      | 3      |
| P1      | 8      | 8      |

Tiempo de espera promedio:

```text
(0 + 3 + 8) / 3 = 11 / 3 ≈ 3.67
```

## Conclusion

| Algoritmo | Promedio |
|-----------|----------|
| FCFS      | 6.33     |
| SJF       | 3.67     |

SJF minimiza el tiempo de espera promedio cuando se conocen las duraciones. Se puede probar que **SJF es optimo** en esa metrica entre los algoritmos no-expropiativos para una misma carga que llega al mismo tiempo. La desventaja es:

1. Requiere conocer o estimar la rafaga futura, lo que en la practica se hace con promedios exponenciales de las rafagas pasadas.
2. Puede producir **starvation** de los procesos largos si llegan continuamente procesos cortos.
