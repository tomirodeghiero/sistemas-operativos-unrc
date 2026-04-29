# Resolucion 5 - Practico 4

Ejercicio 5: describir la secuencia de pasos y los estados de los procesos para un scheduler **Multilevel Feedback (MLF)** de tres niveles con quantum 3, 2 y 1 ms en L0, L1 y L2 respectivamente, dadas las tareas:

| Task | Comportamiento (linea temporal)                |
|------|------------------------------------------------|
| T1   | CPU = 5 ms ; WAIT = 5 ms ; CPU = 1 ms          |
| T2   | CPU = 6 ms ; CPU = 3 ms ; CPU = 3 ms           |
| T3   | CPU = 1 ms ; WAIT = 2 ms ; CPU = 1 ms          |

Las tres ingresan a L0 en t = 0 en el orden T1, T2, T3.

## Conceptos teoricos y convenciones del MLF

De la teoria del curso (seccion *Multilevel queues* y subseccion *Multilevel feedback*):

- Es un esquema basado en prioridades. Se mantienen varias colas, cada una con un *quantum* propio. El nivel L0 tiene la prioridad mas alta y el ultimo nivel (LN) la mas baja.
- El scheduler ejecuta siempre al primer proceso del nivel mas alto que tenga procesos en READY.
- Dentro de cada nivel se aplica RR.
- En MLF la pertenencia a una cola es dinamica:
  - Un proceso que **consume todo el quantum** se considera CPU-bound y se *baja un nivel*.
  - Un proceso que **no consume todo el quantum** (cedio voluntariamente, tipicamente por bloqueo en I/O) se considera interactivo y *sube de nivel* (o se mantiene en su nivel actual si ya esta en L0).

Se asume **preemption entre niveles**: si mientras un proceso ejecuta en LN llega/despierta un proceso en un nivel mas alto, el de menor nivel es desalojado y reencolado al final de su nivel (sin cambiar de nivel, porque no consumio el quantum).

Para T2 se asume que las tres celdas "CPU = 6, CPU = 3, CPU = 3" describen rafagas continuas de CPU **sin bloqueos intermedios** (no hay WAIT entre ellas en la tabla). Es decir, T2 es completamente CPU-bound y necesita un total de 12 ms de CPU sin ceder voluntariamente.

Notacion para la tabla temporal: `Ti:n` significa "tarea i, con n ms de CPU pendientes en su rafaga actual". `<>` denota cola vacia. Los tiempos estan en ms.

## Tabla temporal

| Time  | CPU                            | T1   | T2  | T3   | L0                     | L1             | L2         |
|-------|--------------------------------|------|-----|------|------------------------|----------------|------------|
| 0     | -                              | RDY  | RDY | RDY  | `<T1:5, T2:6, T3:1>`   | `<>`           | `<>`       |
| 0-3   | T1                             | RUN  | RDY | RDY  | `<T2:6, T3:1>`         | `<>`           | `<>`       |
| 3     | demote T1                      | RDY  | RDY | RDY  | `<T2:6, T3:1>`         | `<T1:2>`       | `<>`       |
| 3-6   | T2                             | RDY  | RUN | RDY  | `<T3:1>`               | `<T1:2>`       | `<>`       |
| 6     | demote T2                      | RDY  | RDY | RDY  | `<T3:1>`               | `<T1:2, T2:9>` | `<>`       |
| 6-7   | T3                             | RDY  | RDY | RUN  | `<>`                   | `<T1:2, T2:9>` | `<>`       |
| 7     | T3 -> WAIT                     | RDY  | RDY | WAIT | `<>`                   | `<T1:2, T2:9>` | `<>`       |
| 7-9   | T1                             | RUN  | RDY | WAIT | `<>`                   | `<T2:9>`       | `<>`       |
| 9     | T1 -> WAIT; T3 wakeup          | WAIT | RDY | RDY  | `<T3:1>`               | `<T2:9>`       | `<>`       |
| 9-10  | T3                             | WAIT | RDY | RUN  | `<>`                   | `<T2:9>`       | `<>`       |
| 10    | T3 termina                     | WAIT | RDY | TERM | `<>`                   | `<T2:9>`       | `<>`       |
| 10-12 | T2                             | WAIT | RUN | TERM | `<>`                   | `<>`           | `<>`       |
| 12    | demote T2                      | WAIT | RDY | TERM | `<>`                   | `<>`           | `<T2:7>`   |
| 12-13 | T2                             | WAIT | RUN | TERM | `<>`                   | `<>`           | `<>`       |
| 13    | T2 reencola en L2              | WAIT | RDY | TERM | `<>`                   | `<>`           | `<T2:6>`   |
| 13-14 | T2                             | WAIT | RUN | TERM | `<>`                   | `<>`           | `<>`       |
| 14    | T1 wakeup -> L1; T2 desalojado | RDY  | RDY | TERM | `<>`                   | `<T1:1>`       | `<T2:5>`   |
| 14-15 | T1                             | RUN  | RDY | TERM | `<>`                   | `<>`           | `<T2:5>`   |
| 15    | T1 termina                     | TERM | RDY | TERM | `<>`                   | `<>`           | `<T2:5>`   |
| 15-20 | T2                             | TERM | RUN | TERM | `<>`                   | `<>`           | `<T2:4..0>`|
| 20    | T2 termina                     | TERM | TERM| TERM | `<>`                   | `<>`           | `<>`       |

Verificacion global: la CPU permanece ocupada de 0 a 20 ms sin huecos. La suma de rafagas de CPU es $T_1 = 6$, $T_2 = 12$, $T_3 = 2$, total 20 ms. Cuadra.

## Justificacion de cada paso

A continuacion se explica cada decision del scheduler que aparece en la tabla.

**t = 0 - 3** (`L0`, quantum 3, T1 elegida). El scheduler arranca con L0 = `<T1, T2, T3>`. Toma el primero (T1). T1 ejecuta los 3 ms de quantum. Como su rafaga era de 5 ms, le queda 2 ms y **consumio todo el quantum**: se la *baja* a L1 con remanente 2.

**t = 3 - 6** (T2). Ahora L0 = `<T2, T3>`. T2 corre 3 ms (quantum L0). Como su rafaga era de 6 ms (mas 3 + 3 = 12 ms totales), le quedan 9 ms. Consumio el quantum: se la *baja* a L1.

**t = 6 - 7** (T3). En L0 queda solo T3 con rafaga 1 ms. Empieza el quantum de 3 ms en L0, pero a t = 7 (antes de consumir el quantum) **completa la rafaga y entra a WAIT** por 2 ms. Como cedio voluntariamente, **no se baja**: se queda asociada a L0. La cola L0 queda vacia.

**t = 7 - 9** (T1, en L1, quantum 2). Como L0 = `<>`, el scheduler busca en L1 = `<T1, T2>` y elige T1. T1 ejecuta 2 ms y al final de t = 9 ocurren dos cosas: (i) T1 acaba la rafaga de 5 ms (3 en L0 + 2 en L1) **justo** cuando se cumple el quantum, y entra a WAIT por 5 ms; (ii) T3 despierta de su WAIT y vuelve a estar `RDY` en L0.

  Comentario sobre la coincidencia entre fin de quantum y bloqueo en T1: la interpretacion adoptada es que T1 cedio la CPU **por hacer I/O** (la rafaga termino) y no por timer. Bajo esa lectura, T1 *no* baja a L2; cuando despierte volvera a L1 con 1 ms de CPU pendiente. La interpretacion contraria (timer expulsa antes de notar el bloqueo) llevaria a T1 a L2 con la misma rafaga; el resto del trace cambiaria muy poco y T1 igual terminaria en t = 15. Se elige la interpretacion mas amigable con la regla de la teoria ("no usa todo el quantum -> no baja").

**t = 9 - 10** (T3 en L0, su ultima rafaga de 1 ms). Vuelve a L0 con RDY, el scheduler la elige por encima de cualquier proceso de L1, ejecuta su rafaga de 1 ms y **termina** en t = 10. Sale del sistema.

**t = 10 - 12** (T2 en L1, quantum 2). L0 vacia, L1 = `<T2>`. T2 corre 2 ms, le quedan 7 ms y consumio el quantum: se *baja* a L2.

**t = 12 - 13** (T2 en L2, quantum 1). Solo T2 esta `RDY`. Corre 1 ms, le quedan 6 ms y consumio el quantum, pero ya esta en el ultimo nivel: se reencola en L2 (no hay nivel mas bajo).

**t = 13 - 14** (T2 en L2, quantum 1). Vuelve a tomar la CPU. Corre 1 ms, le quedan 5 ms. Pero a t = 14 **T1 termina su WAIT** y vuelve a la cola L1 con `RDY`. Como L1 tiene mayor prioridad que L2, T2 es **desalojada** (preemption entre niveles); como solo uso 1 ms en este nuevo quantum (que era 1, recien cumplido) se la reencola en L2.

  Aclaracion: aqui el desalojo y el cumplimiento del quantum de L2 ocurren al mismo instante (t = 14), por lo que el efecto neto es el mismo: T2 vuelve al final de L2.

**t = 14 - 15** (T1 en L1, ultima rafaga de 1 ms). T1 corre 1 ms y **termina** en t = 15.

**t = 15 - 20** (T2 en L2, los 5 ms remanentes). Sin nadie en L0 ni L1, T2 ocupa la CPU monopolicamente, ejecutando 5 quantums consecutivos de L2 (cada uno de 1 ms) hasta agotar los 5 ms restantes y **terminar** en t = 20.

## Estados de cada tarea a lo largo del tiempo

Resumen ejecutivo de los estados que atraviesa cada tarea (R = RUNNING, Y = READY, W = WAIT, T = TERMINATED):

```text
T1:  R(0..3) Y(3..7) R(7..9) W(9..14) Y(14..15) R(14..15) T(15..)
T2:  Y(0..3) R(3..6) Y(6..10) R(10..12) Y(12..12) R(12..13) Y(13..13) R(13..14) Y(14..15) R(15..20) T(20..)
T3:  Y(0..6) R(6..7) W(7..9) Y(9..9) R(9..10) T(10..)
```

(Los intervalos de longitud 0 que aparecen son el instante en que un evento -- demote, desalojo, reencolado -- pasa al proceso de RUN a READY antes de volver a tomar la CPU.)

## Metricas

Como cierre, aunque el enunciado no las pide, se calculan las metricas usuales con estos resultados:

| Tarea | Llegada | Fin | Turnaround | CPU usada | Espera (Turnaround - CPU - WAIT) |
|-------|---------|-----|------------|-----------|----------------------------------|
| T1    | 0       | 15  | 15         | 6         | 4                                |
| T2    | 0       | 20  | 20         | 12        | 8                                |
| T3    | 0       | 10  | 10         | 2         | 6                                |

T3 (el mas corto y con I/O) sale rapido del sistema; T2 (CPU-bound puro) es justamente el que mas baja en la jerarquia y termina ultimo. Es exactamente el comportamiento que la teoria del curso describe como objetivo del MLF: **mejorar tiempo de respuesta de procesos interactivos castigando a los CPU-bound** (sin starvation, porque al final T2 tambien termina).

## Observaciones finales

1. **Determinismo del trace**: el resultado depende fuertemente de las convenciones de empate. Aqui se resolvieron asi:
   - Si un nuevo arribo y un desalojo ocurren al mismo instante, primero entra el nuevo y despues el desalojado.
   - Si la rafaga termina justo al expirar el quantum, prevalece la lectura "I/O" (no demote).
   - Despertar de un WAIT vuelve al proceso al **mismo nivel** donde estaba antes del bloqueo.
2. **Aging**: la teoria menciona que MLF puede combinarse con aging para evitar starvation cuando un proceso quedo en el ultimo nivel y siguen llegando procesos a niveles mas altos. En este ejercicio no se aplica porque eventualmente todas las tareas terminan, pero es la objection clasica al esquema MLF puro.
3. **Cantidad de cambios de contexto**: en esta corrida hay 9 transiciones donde la CPU pasa de un proceso a otro. En sistemas reales el costo del context switch es no despreciable; un quantum mas grande lo reduciria a costa de empeorar el tiempo de respuesta de las tareas cortas como T3.
