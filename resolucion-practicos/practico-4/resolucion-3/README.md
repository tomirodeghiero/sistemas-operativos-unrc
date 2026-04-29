# Resolucion 3 - Practico 4

Ejercicio 3: un planificador recalcula una vez por segundo la prioridad de cada proceso usando

$$\text{priority} = \frac{\text{recent\_cpu\_usage}()}{2} + 60$$

donde el valor numerico es invertido (a **mayor valor, menor prioridad**). Tres procesos en el sistema con usos recientes de CPU de 30, 15 y 10 para P1, P2 y P3 respectivamente.

1. ¿Como prioriza a los procesos orientados a CPU y a los orientados a I/O?
2. ¿Cual es el objetivo del planificador?

## Conceptos teoricos

De las notas del curso:

- En el algoritmo basado en *prioridades*, se elige siempre al proceso con mayor prioridad. La asignacion puede ser **estatica** (al crear el proceso) o **dinamica** (en run-time).
- El problema clasico es la **starvation**: un proceso de baja prioridad puede no ser planificado nunca si llegan continuamente procesos de mas alta prioridad.
- Una solucion es el **aging** (envejecimiento): incrementar gradualmente la prioridad de procesos que llevan mucho tiempo esperando.
- La idea de *Multilevel Feedback (MLF)* lleva esto al extremo: un proceso que **no usa todo el quantum se considera interactivo y se sube de nivel** (mas prioridad), y uno que lo consume completo se considera *CPU-bound* y se baja. Esa estrategia "prioriza a los procesos orientados a entrada-salida y contribuye a mejorar los tiempos de respuesta" (cita literal del capitulo).

La formula del enunciado es exactamente un caso particular de aging dinamico de esta familia: la prioridad numerica crece con el uso reciente de CPU, y como en este sistema **mayor valor implica menor prioridad**, mas uso de CPU castiga al proceso.

## Calculo de las prioridades

| Proceso | recent_cpu | priority = recent_cpu/2 + 60 | Posicion |
|---------|------------|------------------------------|----------|
| P1      | 30         | 30/2 + 60 = **75**           | menor prioridad |
| P2      | 15         | 15/2 + 60 = **67.5**         | intermedia |
| P3      | 10         | 10/2 + 60 = **65**           | mayor prioridad |

Es decir, el orden por prioridad (de mas alta a mas baja) es:

$$P_3 \;\succ\; P_2 \;\succ\; P_1.$$

## 1. Como prioriza a CPU-bound vs I/O-bound

Un proceso **orientado a CPU** ejecuta rafagas largas: gasta su quantum sin bloquearse, asi que su contador `recent_cpu_usage` crece rapido cada segundo (la formula recalcula y suma uso). Su `priority` numerica sube y, como mayor valor implica menor prioridad efectiva, **el algoritmo lo posterga**. P1 es ejemplo en la tabla: uso reciente 30, prioridad 75 (la mas baja del trio).

Un proceso **orientado a I/O** se bloquea seguido en `read`/`write` y rara vez consume el quantum entero. Su `recent_cpu_usage` queda chico, su `priority` numerica se mantiene cerca del piso (60), y por lo tanto **el algoritmo lo favorece** y lo planifica enseguida cuando vuelve de I/O. P3 es ejemplo: uso 10, prioridad 65 (la mas alta).

El "+60" funciona como piso fijo de la prioridad (ningun proceso baja de 60) y el "/2" amortigua el crecimiento, evitando que un pico breve de uso de CPU castigue de manera desproporcionada al proceso. Esto da un comportamiento suave: la prioridad cambia en forma gradual con el patron de uso.

> **Observacion**: en realidad la formula provista es la receta clasica del scheduler historico de **4.3BSD UNIX** (multilevel feedback con decay del `recent_cpu`), donde el contador se decae exponencialmente entre recalculos para no quedar pegados con el comportamiento viejo. La teoria del curso describe una idea equivalente cuando habla de *aging* y de MLF "que se mueven entre niveles".

## 2. Objetivo del planificador

El objetivo es **minimizar el tiempo de respuesta** del sistema y **maximizar el throughput** privilegiando a los procesos interactivos / I/O-bound, sin negarles servicio definitivamente a los CPU-bound.

Mas concretamente:

- **Mejor responsiveness para procesos interactivos**: editores, shells, terminales y servicios atendiendo requests son tipicamente I/O-bound. Que el planificador les de prioridad alta cuando vuelven de un bloqueo significa que la latencia entre que llega un dato (tecla, paquete, lectura de disco) y que se procesa, es minima.
- **Mejor uso de los dispositivos de I/O**: si los procesos I/O-bound se planifican rapido, mantienen los dispositivos ocupados y se solapa I/O con CPU. Los CPU-bound pueden usar la CPU mientras los otros estan bloqueados sin perjudicar el throughput global.
- **Evitar starvation pese a usar prioridades**: como `recent_cpu_usage` solo refleja la *historia reciente*, un proceso que llega como CPU-bound pero deja de demandar CPU vera caer su priority y volvera a ser elegible. La penalizacion no es para siempre. Esto es exactamente el rol que la teoria le adjudica al *aging*.

Resumiendo en una linea: el planificador busca **un compromiso entre utilizacion de CPU, throughput y tiempo de respuesta** que la teoria del curso identifica como el objetivo central de los short-term schedulers en sistemas time-sharing.
