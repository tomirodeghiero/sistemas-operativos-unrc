# Resolucion 4 - Practico 4

Ejercicio 4: dada la tabla del ejercicio 1, dar la secuencia de planificacion con prioridades

| Proceso | CPU | Prioridad |
|---------|-----|-----------|
| P1      | 8   | 1         |
| P2      | 3   | 3         |
| P3      | 5   | 2         |

Asumiendo que los tres arriban en t = 0.

## Conceptos teoricos

De la teoria del curso (capitulo *Planificacion de uso de CPU*, seccion *Uso de prioridades*):

- Se asigna un numero de prioridad a cada proceso. La relacion de orden puede ser arbitraria: a menor valor mayor prioridad o a mayor valor mayor prioridad. **El enunciado no fija la convencion**, asi que se debe elegir y justificar.
- El algoritmo puede ser preemptivo o no. Como aqui los tres procesos arriban en t = 0 y nadie se incorpora despues, no hay diferencia entre la version preemptiva y la no preemptiva: una vez decidido el orden, ningun nuevo arribo lo puede alterar.
- Asumimos prioridades **estaticas** (no se modifican durante la corrida). El enunciado no menciona aging.

### Convencion de prioridades

Se adopta la convencion mas extendida en bibliografia (Silberschatz, Tanenbaum) y la que usa Linux en el rango *real-time*: **menor numero implica mayor prioridad**. Es la misma convencion que aparece en la teoria del curso al describir el rango Linux 0-99 para procesos real-time, donde 0 es la mayor prioridad.

Bajo esta convencion, el orden decreciente de prioridad es:

$$P_1 \;(\text{prio } 1) \;\succ\; P_3 \;(\text{prio } 2) \;\succ\; P_2 \;(\text{prio } 3).$$

(Si en cambio el enunciado quisiera la convencion contraria, alcanza con invertir el orden: P2 - P3 - P1. Se discute al final.)

## Secuencia de planificacion

Como los tres llegan en t = 0, en ese instante todos estan READY y el scheduler elige al de mayor prioridad: P1. P1 corre hasta terminar (la version no preemptiva no le quita CPU; la preemptiva tampoco, porque nadie con prioridad mayor aparece). Luego entran en orden P3 y P2.

### Diagrama de Gantt

```text
| P1                   | P3            | P2       |
0                      8               13          16
```

| Proceso | Inicio | Fin | Espera (Inicio - Arribo) | Turnaround (Fin - Arribo) |
|---------|--------|-----|--------------------------|---------------------------|
| P1      | 0      | 8   | 0                        | 8                         |
| P3      | 8      | 13  | 8                        | 13                        |
| P2      | 13     | 16  | 13                       | 16                        |

Promedios:

- Tiempo de espera medio: $(0 + 8 + 13)/3 = 7$.
- Turnaround medio: $(8 + 13 + 16)/3 \approx 12.33$.

## Observaciones

1. La secuencia depende **exclusivamente** del numero de prioridad. Notar que P1, ademas de tener mayor prioridad, es el de rafaga mas larga: el algoritmo prioriza segun la prioridad asignada y no segun la rafaga. Esto provoca un perjuicio para los demas procesos respecto de SJF: con SJF la espera promedio hubiera sido $(0 + 8 + 11)/3 \approx 6.33$ (orden P2-P3-P1), claramente menor.
2. El algoritmo de prioridades, tal como lo presenta la teoria, **no garantiza optimalidad** en tiempo de espera ni en turnaround promedio: solo respeta la importancia relativa que el sistema asigna a cada proceso.
3. **Riesgo de starvation**: si en este sistema llegaran continuamente procesos con prioridad 1 o 2, P2 (prioridad 3) podria quedar pospuesto indefinidamente. La solucion estandar es **aging**, que la teoria menciona explicitamente: incrementar la prioridad de los procesos que llevan mucho tiempo esperando.

### Si la convencion fuera "mayor numero, mayor prioridad"

El orden seria $P_2 \succ P_3 \succ P_1$ y el Gantt:

```text
| P2       | P3            | P1                   |
0          3               8                      16
```

| Proceso | Inicio | Fin | Espera | Turnaround |
|---------|--------|-----|--------|------------|
| P2      | 0      | 3   | 0      | 3          |
| P3      | 3      | 8   | 3      | 8          |
| P1      | 8      | 16  | 8      | 16         |

Promedios: espera $(0 + 3 + 8)/3 \approx 3.67$; turnaround $(3 + 8 + 16)/3 = 9$. Se obtienen los mismos numeros que SJF, porque en este caso particular las "prioridades altas" coinciden con las rafagas mas cortas.

Como criterio de la teoria, ambas convenciones son legitimas; la respuesta principal del ejercicio es la primera (P1-P3-P2) y se aclara la alternativa para que quede explicita la dependencia del resultado respecto del criterio de orden.
