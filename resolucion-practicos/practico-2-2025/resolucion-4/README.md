# Practico 2 - Ejercicio 4

## Enunciado
Describir el flujo de ejecucion cuando un proceso es interrumpido por el timer hasta retornar de la interrupcion.

## Flujo completo (paso a paso)

### 1) El proceso esta corriendo en user mode
La CPU esta ejecutando instrucciones del proceso actual.

### 2) Llega la interrupcion del timer
El reloj de hardware dispara una interrupcion periodica (tick).

### 3) La CPU cambia a modo kernel
Ante la interrupcion:
- entra a modo supervisor/kernel,
- salta al handler de interrupciones (segun el vector de traps),
- deja de ejecutar temporalmente el codigo de usuario.

### 4) Cambio de stack y guardado de contexto
Si la interrupcion ocurrio en user mode, el kernel cambia al stack de kernel del proceso y guarda el estado de CPU (trapframe):
- pc/sepc (donde retomar),
- registros generales,
- flags/estado de control.

Con esto el proceso queda "congelado" de forma segura.

### 5) Se ejecuta el timer interrupt handler
El handler del timer:
- reconoce/acknowledgea la interrupcion del reloj,
- actualiza contadores de ticks/tiempo,
- puede actualizar contabilidad del proceso (uso de CPU).

### 6) Decision de planificacion
Aca hay dos caminos:

- **Camino A (no desalojo):**
  si el proceso actual todavia puede seguir (por ejemplo no agoto quantum), el kernel no cambia de tarea.

- **Camino B (desalojo/preemption):**
  si agoto quantum, el kernel marca el proceso como `RUNNABLE` y hace `yield()`/`schedule()` para elegir otro proceso.

### 7) (Si hay cambio de contexto) scheduler
Si hay preemption:
- se guarda el contexto de kernel del proceso saliente,
- scheduler elige otro proceso `RUNNABLE`,
- se restaura el contexto del elegido,
- queda listo su trapframe para volver a user mode.

### 8) Retorno de interrupcion
Cuando termina el manejo del trap (con o sin cambio de proceso), el kernel ejecuta la instruccion de retorno de interrupcion (`sret` en RISC-V, `iret` en x86):
- restaura registros desde trapframe,
- restaura `pc` de retorno,
- vuelve a user mode.

### 9) Continuacion de ejecucion
- Si no hubo cambio de tarea, sigue el mismo proceso desde la instruccion interrumpida.
- Si hubo cambio, continua el nuevo proceso desde el punto donde habia quedado antes.

## Resumen corto
La interrupcion de timer fuerza una entrada al kernel, guarda el estado del proceso actual, permite que el scheduler decida si desalojar o no, y finalmente retorna con `sret/iret` al proceso que corresponda.
