# Resolucion 2 - Practico 4

Ejercicio 2: para la misma tabla de procesos del ejercicio 1, mostrar la secuencia de planificacion segun **Round Robin** con quantum `q = 2` (preemptive scheduling) hasta que todos terminen.

1. Calcular el **tiempo de permanencia (turnaround)** de cada proceso.
2. Comparar contra los turnaround de FCFS y SJF.

| Proceso | Arribo | CPU |
|---------|--------|-----|
| P1      | 0.0    | 8   |
| P2      | 0.5    | 3   |
| P3      | 3.0    | 5   |

## Conceptos teoricos

De la teoria del curso:

- **Round Robin**: variante preemptiva de FCFS. Se mantiene una cola FIFO de procesos `RUNNABLE`; al obtener la CPU se le asigna un *quantum*. Si el proceso consume el quantum sin liberar CPU voluntariamente, el timer lo desaloja y se reencola al final.
- **Quantum**: intervalo maximo de uso continuo de CPU. La teoria lo define en *ticks* del timer.
- **Tiempo de permanencia (turnaround)**: tiempo total que pasa el proceso en el sistema, $T_i = \text{Completion}_i - \text{Arrival}_i$. Equivale a la suma del tiempo en la CPU mas el tiempo en READY mas el tiempo en bloqueos. En este ejercicio no hay bloqueos por I/O.

Convencion para el reencolado: cuando varios eventos ocurren al mismo instante (un proceso recien llegado y un proceso que vuelve por consumir su quantum), se encola **primero** al recien llegado y **despues** al desalojado. Esta convencion es la habitual en los textos clasicos (Silberschatz, Tanenbaum) y es la que se asume en lo que sigue.

## Traza paso a paso

Notacion: la cola de READY se escribe como `[cabeza ... cola]`; entre parentesis se anota la **rafaga remanente** de cada proceso al momento de la operacion.

| Intervalo | CPU corriendo | Evento al final del intervalo                                          | Cola READY al cierre              |
|-----------|---------------|------------------------------------------------------------------------|-----------------------------------|
| 0 - 2     | P1            | t = 0.5: arriba P2; t = 2: P1 consume quantum, remanente 6, reencola   | [P2(3), P1(6)]                    |
| 2 - 4     | P2            | t = 3: arriba P3; t = 4: P2 consume quantum, remanente 1, reencola     | [P1(6), P3(5), P2(1)]             |
| 4 - 6     | P1            | t = 6: P1 consume quantum, remanente 4, reencola                       | [P3(5), P2(1), P1(4)]             |
| 6 - 8     | P3            | t = 8: P3 consume quantum, remanente 3, reencola                       | [P2(1), P1(4), P3(3)]             |
| 8 - 9     | P2            | t = 9: P2 termina (rafaga era 1 < q)                                   | [P1(4), P3(3)]                    |
| 9 - 11    | P1            | t = 11: P1 consume quantum, remanente 2, reencola                      | [P3(3), P1(2)]                    |
| 11 - 13   | P3            | t = 13: P3 consume quantum, remanente 1, reencola                      | [P1(2), P3(1)]                    |
| 13 - 15   | P1            | t = 15: P1 termina                                                     | [P3(1)]                           |
| 15 - 16   | P3            | t = 16: P3 termina                                                     | [ ]                               |

Verificacion: la CPU permanece ocupada de 0 a 16 sin huecos, y la suma de rafagas es 8 + 3 + 5 = 16. Cuadra.

### Diagrama de Gantt

```text
| P1 | P2 | P1 | P3 | P2 | P1 | P3 | P1 | P3 |
0    2    4    6    8    9    11   13   15   16
```

## 1. Turnaround de cada proceso

$T_i = \text{Completion}_i - \text{Arrival}_i$:

| Proceso | Arribo | Fin | Turnaround |
|---------|--------|-----|------------|
| P1      | 0.0    | 15  | 15.0       |
| P2      | 0.5    | 9   | 8.5        |
| P3      | 3.0    | 16  | 13.0       |

Promedio:

$$\overline{T}_{\text{RR}} = \frac{15 + 8.5 + 13}{3} = \frac{36.5}{3} \approx 12.17.$$

## 2. Comparacion con FCFS y SJF

Para que la comparacion sea cuantitativa hay que tener cuidado con las **suposiciones** de cada algoritmo, porque el enunciado del ejercicio 1 las trata distinto:

- **FCFS**: usa los tiempos de arribo originales (0.0, 0.5, 3.0). Orden P1 - P2 - P3, fines 8, 11, 16.
- **SJF**: el enunciado de la parte 2 del ejercicio 1 pide asumir que todos arriban en t = 0. Orden P2 - P3 - P1, fines 3, 8, 16.
- **RR**: en este ejercicio se mantienen los arribos originales (0.0, 0.5, 3.0).

| Proceso | $T$ FCFS  | $T$ SJF  | $T$ RR  |
|---------|-----------|----------|---------|
| P1      | 8.0       | 16       | 15.0    |
| P2      | 10.5      | 3        | 8.5     |
| P3      | 13.0      | 8        | 13.0    |
| **Promedio** | **10.50** | **9.00** | **12.17** |

Lectura cualitativa:

- **SJF** es el mejor de los tres en turnaround promedio (9.0). Coincide con la propiedad clasica: dado un conjunto fijo de rafagas, planificar primero las mas cortas minimiza la espera promedio (y, equivalentemente, el turnaround promedio).
- **FCFS** queda intermedio (10.5). Sufre el *convoy effect* porque P1 (la rafaga mas larga) llega primero y arrastra la espera de los demas, pero como cada proceso ejecuta de un solo tiron no paga el costo de fragmentacion.
- **RR** es el peor en promedio (12.17). El *quantum* chico (q = 2) hace que P1 termine ejecutando en cuatro porciones intercaladas con los otros; nadie corre "de un tiron". Pero a cambio, **P2 se beneficia notoriamente** (de 10.5 en FCFS a 8.5 en RR): la rafaga mas corta no queda atascada detras de la mas larga, lo que se traduce en mejor *tiempo de respuesta* para procesos cortos.
- **P1** sale claramente perjudicado en RR (de 8 a 15) por la fragmentacion.
- **P3** queda igual en FCFS y RR (13) porque su rafaga termina al final del schedule en ambos casos.

Esto coincide con la teoria: **RR no esta pensado para minimizar turnaround promedio** sino para acotar el tiempo de respuesta en sistemas *time-sharing*. El precio que se paga por el desalojo periodico es el costo del cambio de contexto y un mayor turnaround promedio cuando hay procesos largos.

> **Observacion**: si se incrementase el quantum (por ejemplo, $q$ tendiendo al maximo de la rafaga mas larga), RR converge a FCFS. En el extremo opuesto, $q$ muy chico hace que RR aproxime a *processor sharing* (todos progresan en paralelo), pero el overhead de cambios de contexto domina. Elegir un buen quantum es el compromiso clasico que la teoria menciona al introducir el algoritmo.
