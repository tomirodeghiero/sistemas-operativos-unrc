# Resolucion 1 - Practico 4

Ejercicio 1: dada la tabla de procesos

| Proceso | Tiempo de creacion (arribo) | Tiempo de uso de CPU |
|---------|-----------------------------|----------------------|
| P1      | 0.0                         | 8                    |
| P2      | 0.5                         | 3                    |
| P3      | 3.0                         | 5                    |

determinar el **tiempo de espera promedio** para los algoritmos:

1. FCFS (FIFO).
2. Shortest Job First (SJF).

## Conceptos teoricos

De las notas del curso (capitulo *Planificacion de uso de CPU*):

- **FCFS**: cola FIFO. Cuando un proceso pasa a `RUNNABLE` se encola al final; el primero de la cola toma la CPU. **No es preemptivo**: hasta que el proceso no termina, no libera la CPU.
- **SJF**: se elige el proceso con menor tiempo de proxima rafaga de CPU. La presentacion en la teoria es la version no preemptiva (*shortest job first* clasica de batch); cuando se aplica como *short-term* se la conoce como *shortest cpu-burst first*. En este ejercicio no se piden estimadores ni promedio exponencial, asi que se usan los tiempos exactos provistos.
- **Tiempo de espera (waiting time)** de un proceso: tiempo que el proceso pasa en la cola READY/RUNNABLE esperando por la CPU. Es decir,

  $$W_i = \text{Completion}_i - \text{Arrival}_i - \text{CPU}_i$$

  o, equivalentemente para algoritmos no preemptivos, $W_i = \text{Start}_i - \text{Arrival}_i$.

## 1. FCFS (FIFO)

El orden de creacion es P1, P2, P3, asi que la cola de arribo es la misma. Como el algoritmo no es preemptivo, P1 se aduena de la CPU desde t = 0 y la libera recien al terminar.

Diagrama de Gantt:

```text
| P1                   | P2       | P3            |
0                      8          11              16
```

| Proceso | Arribo | Inicio | Fin | Espera (Inicio - Arribo) |
|---------|--------|--------|-----|--------------------------|
| P1      | 0.0    | 0      | 8   | 0.0                      |
| P2      | 0.5    | 8      | 11  | 7.5                      |
| P3      | 3.0    | 11     | 16  | 8.0                      |

Tiempo de espera promedio:

$$\overline{W}_{\text{FCFS}} = \frac{0 + 7.5 + 8}{3} = \frac{15.5}{3} \approx 5.17.$$

Observacion clasica: P1 es la rafaga mas larga y llega primero, asi que arrastra el tiempo de espera de los demas (efecto *convoy*). Es exactamente la desventaja que la teoria le adjudica a FCFS ("comunmente produce tiempos promedio de espera grandes").

## 2. SJF (no preemptivo, todos arriban en t = 0)

El enunciado pide aplicar SJF **asumiendo que los tres procesos arriban en t = 0**. Es decir, descartamos las llegadas escalonadas (0, 0.5, 3) y suponemos que las tres rafagas estan disponibles desde el principio. El scheduler elige siempre la rafaga mas corta entre las que estan en READY:

- En t = 0: READY = {P1(8), P2(3), P3(5)}. La mas corta es P2 (3). P2 toma la CPU.
- En t = 3: READY = {P1(8), P3(5)}. La mas corta es P3 (5). P3 toma la CPU.
- En t = 8: READY = {P1(8)}. Toma la CPU P1 (no hay otra opcion).

Como SJF no es preemptivo, una vez asignada la CPU el proceso corre hasta terminar su rafaga.

Diagrama de Gantt:

```text
| P2       | P3            | P1                   |
0          3               8                      16
```

| Proceso | Arribo | Inicio | Fin | Espera (Inicio - Arribo) |
|---------|--------|--------|-----|--------------------------|
| P2      | 0      | 0      | 3   | 0                        |
| P3      | 0      | 3      | 8   | 3                        |
| P1      | 0      | 8      | 16  | 8                        |

Tiempo de espera promedio:

$$\overline{W}_{\text{SJF}} = \frac{0 + 3 + 8}{3} = \frac{11}{3} \approx 3.67.$$

## Comparacion

| Algoritmo | Orden       | Espera promedio |
|-----------|-------------|-----------------|
| FCFS      | P1 - P2 - P3 | 5.17            |
| SJF (todos en t=0) | P2 - P3 - P1 | **3.67**       |

SJF da menor tiempo de espera promedio. La razon es la propiedad clasica del algoritmo: **dado un conjunto fijo de rafagas conocidas, ejecutar primero las mas cortas minimiza la espera promedio**. La intuicion es que cuando un proceso esta en CPU, todos los demas suman tiempo de espera; por eso conviene "sacarse de encima" rapido a los cortos, asi solo los pocos largos siguen acumulando espera al final. Esta propiedad de optimalidad, demostrable formalmente, es la motivacion historica del algoritmo cuando se conocen las rafagas (tipico de sistemas batch / *long-term schedulers*).

FCFS, en cambio, padece el efecto **convoy**: como P1 (la rafaga mas larga) llega primero, arrastra la espera de P2 y P3 detras. La teoria del curso adjudica este comportamiento explicitamente a FCFS ("comunmente produce tiempos promedio de espera grandes").

> **Observacion (no pedida en el enunciado)**: si se conservaran las llegadas originales (0.0, 0.5, 3.0) y se aplicara SJF *no preemptivo*, en t = 0 solo P1 esta presente, asi que SJF se reduce a FCFS y el resultado coincide con el de la parte 1. La variante preemptiva *Shortest Remaining Time First* (SRTF), que tampoco es la pedida, si introduce una mejora porque en t = 0.5 puede desalojar a P1 al detectar que la rafaga restante de P2 es menor.
