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

## 2. SJF (no preemptivo)

Como SJF es no preemptivo, en t = 0 solo esta P1 listo (P2 todavia no arribo) y se le otorga la CPU. P1 corre los 8 unidades sin ser interrumpido. Cuando libera la CPU en t = 8 ya estan en la cola READY P2 (rafaga 3) y P3 (rafaga 5); el scheduler elige el de menor rafaga, P2. Al terminar P2 en t = 11 queda solo P3, que ejecuta hasta t = 16.

Diagrama de Gantt:

```text
| P1                   | P2       | P3            |
0                      8          11              16
```

| Proceso | Arribo | Inicio | Fin | Espera                     |
|---------|--------|--------|-----|----------------------------|
| P1      | 0.0    | 0      | 8   | 0.0                        |
| P2      | 0.5    | 8      | 11  | 7.5                        |
| P3      | 3.0    | 11     | 16  | 8.0                        |

Tiempo de espera promedio:

$$\overline{W}_{\text{SJF}} = \frac{0 + 7.5 + 8}{3} \approx 5.17.$$

## Comparacion

En este caso particular el resultado de FCFS y SJF coincide. La razon es la siguiente:

- En t = 0 P1 esta solo en la cola, asi que cualquier algoritmo no preemptivo le tiene que dar la CPU (no hay con quien comparar la rafaga).
- Una vez que P1 termina (t = 8), ya estan en READY P2 y P3. El orden por menor rafaga (3 < 5) coincide con el orden de arribo (P2 antes que P3). Por eso ambos algoritmos generan la misma secuencia.

En general SJF sera mejor o igual que FCFS si el primer proceso fuera mas corto que los siguientes; aqui no se observa diferencia porque la primera rafaga ya esta fijada por la llegada temprana de P1.

> **Observacion adicional (no pedida)**: si en cambio se considerara la variante preemptiva *Shortest Remaining Time First* (SRTF), el resultado cambia. En t = 0.5 llega P2 con rafaga 3, menor que el remanente de P1 (7.5), asi que P2 desaloja a P1; luego de terminar P2 y P3 (mas cortos que el remanente de P1), P1 termina ultimo. La espera promedio caeria a (8 + 0 + 0.5)/3 ≈ 2.83. Esto ilustra por que los textos suelen presentar SJF como ejemplo del beneficio de planificar por menor rafaga, pero recien con la version preemptiva se aprovecha la informacion en pleno.
