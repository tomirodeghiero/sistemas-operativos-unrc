# Resolucion 8 - Practico 3

Ejercicio 8: como se representa el estado de ejecucion completo de una tarea que no esta RUNNING.

## Contexto

Una tarea (proceso o hilo) puede estar en varios estados: `RUNNING`, `RUNNABLE` (lista para ejecutar pero sin CPU), `SLEEPING` (bloqueada en un evento), `ZOMBIE`, `STOPPED`, etc. Solo las tareas `RUNNING` estan usando una CPU en un instante dado; las demas estan "congeladas" y para poder reanudarlas mas tarde el SO debe tener guardada toda la informacion que la CPU tenia cuando esa tarea dejo de correr.

En xv6 (y en Linux conceptualmente igual) cada tarea se representa con un **Process Control Block** (PCB). Para una tarea que no esta RUNNING el PCB almacena todo lo necesario para reanudarla:

## Componentes del estado completo

1. **Registros de la CPU salvados (contexto de CPU)**

   - Todos los registros de proposito general (en RISC-V: `x0`-`x31`, especialmente los callee-saved `s0`-`s11`, `sp`, `ra`).
   - Los registros de punto flotante si la tarea los uso.
   - El program counter (`pc`) o el equivalente `ra` de donde retomar.
   - El stack pointer (`sp`).

   En xv6 esto se guarda en `struct context` (para context switches entre tareas en modo kernel) y en `struct trapframe` (para traps desde modo usuario). Ambos estan en el PCB / `struct proc`.

2. **Stacks**

   - Stack en modo kernel propio de la tarea (cada proceso tiene uno).
   - Stack en modo usuario, que es parte del espacio de direcciones del proceso.

3. **Espacio de direcciones virtuales**

   - Tabla de paginas raiz (registro `satp` en RISC-V / `CR3` en x86).
   - Permite que al reanudarla la MMU vuelva a mapear sus regiones de codigo, datos, heap y stack.

4. **Estado de ejecucion logico**

   - Campo `state`: `RUNNABLE`, `SLEEPING`, `ZOMBIE`, etc.
   - Si esta en `SLEEPING`: el canal / recurso sobre el que duerme (`chan` en xv6).
   - Si esta en `ZOMBIE`: el codigo de salida (`xstate`) para que el padre lo recupere con `wait()`.

5. **Metadatos de gestion**

   - `pid`, `ppid`, `parent`.
   - Tabla de descriptores de archivos abiertos.
   - Directorio de trabajo actual (`cwd`).
   - Nombre del ejecutable (`name`).
   - Senales pendientes y mascara (en sistemas que las implementan).

## En una frase

El estado completo de una tarea no-RUNNING esta formado por: **los valores de todos los registros de la CPU al momento de ser desplanificada + sus stacks + el puntero a su espacio de direcciones + la informacion logica de gestion (estado, parentesco, files, cwd, etc.)**. Todo junto vive en la estructura que el SO llama PCB (en xv6: `struct proc`).

## Diagrama conceptual (xv6)

```text
                          struct proc
         +----------------------------------------+
         | pid, state (RUNNABLE/SLEEPING/...)     |
         | parent, chan, xstate                   |
         | pagetable  (espacio de direcciones)    |
         | sz (tamano del espacio usuario)        |
         | kstack (stack kernel para esta tarea)  |
         | trapframe*  ---->  regs usuario + sepc |
         | context     ---->  regs callee-saved + |
         |                    ra, sp para swtch() |
         | ofile[]  (tabla de fds)                |
         | cwd                                    |
         | name                                   |
         +----------------------------------------+
```

Cuando el scheduler decide volver a correr esa tarea, hace:

1. `satp = p->pagetable` -> restaura el address space.
2. `swtch(&cpu->scheduler_context, &p->context)` -> restaura los registros callee-saved y salta a donde esa tarea habia quedado.
3. Si finalmente debe volver a modo usuario, `usertrapret()` usa el trapframe para restaurar el resto de los registros y hace `sret`.
