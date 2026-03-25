# Practico 2 - Ejercicio 2

## Pregunta
Como se representa el estado de ejecucion completo de una tarea que no esta RUNNING?

## Respuesta
Cuando una tarea no esta corriendo en CPU, su estado completo queda guardado en estructuras del kernel.

La idea es simple: no se pierde "donde iba" la tarea, sino que el SO guarda todo lo necesario para retomarla exactamente desde ese punto.

## Que guarda el sistema operativo
En general (PCB/task_struct + stack de kernel) se guarda:

- Estado de planificacion de la tarea (`RUNNABLE`, `SLEEPING`, etc).
- Contexto de CPU necesario para reanudarla (por ejemplo `pc`, `sp` y otros registros).
- Trapframe/contexto de interrupcion o de cambio de contexto (en stack de kernel).
- Referencias al espacio de memoria del proceso (tabla de paginas, stack de usuario, segmentos).
- Recursos asociados (archivos abiertos, senales, info de sincronizacion, etc).

## Interpretacion
Entonces, una tarea que no esta RUNNING no esta "desaparecida":

- Su contexto esta persistido en memoria del kernel.
- El scheduler la vuelve a elegir cuando corresponda.
- Al hacer context switch, el kernel restaura ese contexto y la ejecucion continua.

En resumen: el estado completo de una tarea no RUNNING se representa por su descriptor de proceso/tarea mas el contexto guardado en su stack de kernel.
