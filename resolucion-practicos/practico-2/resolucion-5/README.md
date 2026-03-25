# Practico 2 - Ejercicio 5

## Pregunta
Cual es la diferencia entre el trapframe y el contexto salvado de cada tarea? Como se usa cada uno?

## Respuesta corta
No son lo mismo y se usan en momentos distintos:

- **Trapframe:** se usa en la entrada/salida de traps (syscalls, interrupciones, excepciones), sobre todo para volver a user mode.
- **Contexto salvado (context):** se usa en el cambio de contexto entre tareas dentro del kernel (scheduler/switch).

## Diferencia conceptual

### Trapframe
Es la "foto" del estado de CPU del proceso en el instante del trap.
Normalmente guarda muchos registros (practicamente todos los de usuario), ademas del PC de retorno (`sepc`/`epc`) y bits de estado.

Se guarda en el stack de kernel del proceso (o estructura asociada) cuando la CPU entra por interrupcion/trap.

### Contexto salvado de tarea
Es el conjunto minimo de registros que necesita el kernel para pausar y reanudar ejecucion de una tarea en modo kernel durante `context_switch`.
En general incluye registros callee-saved + `sp` + `ra` (segun ABI/arquitectura).

No representa "todo user mode", solo lo necesario para retomar el hilo de ejecucion del kernel de esa tarea.

## Como se usa cada uno (flujo tipico)

1. Un proceso corre en user mode.
2. Ocurre trap (timer, syscall, excepcion).
3. El kernel guarda **trapframe** del proceso actual.
4. Si hace falta planificar otro, se ejecuta `switch_context` y ahi se guarda/restaura **contexto de kernel** de tareas.
5. Cuando le toca volver a ese proceso, el kernel restaura su **trapframe** y hace `sret/iret` para regresar a user mode.

## En una frase
- El **trapframe** permite volver correctamente al punto de user mode interrumpido.
- El **contexto** permite que el scheduler cambie de tarea dentro del kernel sin perder donde estaba cada una.

## Detalle importante para examen
En RISC-V (y en general):
- El retorno de syscall se refleja en registros del trapframe (por ejemplo `a0`).
- Por eso en `fork()`, cambiar `a0` en trapframe del padre/hijo define que ve cada uno al volver a user mode.
