# Resolucion 8 - Practico 3

Ejercicio 8: describir las operaciones del kernel cuando un proceso es interrumpido por el timer y ya gasto su quantum, hasta volver de la interrupcion. Indicar en que estado el kernel deja al proceso.

La descripcion sigue el modelo visto en la teoria: xv6 sobre RISC-V (privilegio S, MMU Sv39). En Linux los nombres cambian (`do_IRQ`, `schedule()`, `task_struct`), pero la secuencia conceptual es la misma.

## Punto de partida

Un proceso `P` esta corriendo en modo usuario (`U`) cuando ocurre una interrupcion del **timer**: el hardware del CLINT genera una IRQ periodica que el kernel previamente programo al inicializarse (delegada de M-mode a S-mode). Por configuracion del scheduler en Linux/xv6, esa IRQ marca el fin del *time slice* asignado a `P`.

## Paso 1. La CPU atrapa la interrupcion (en hardware)

Al arribar la IRQ, sin intervencion de software:

1. La CPU verifica que las interrupciones del modo S esten habilitadas (`sstatus.SIE = 1`).
2. Salva en `sepc` la direccion de la proxima instruccion que debia ejecutarse en `P` (donde retomar despues).
3. Salva el modo previo en `sstatus.SPP` (`U`, modo usuario).
4. Codifica el motivo en `scause` (interrupcion de timer en supervisor).
5. Cambia el modo de la CPU a supervisor (`S`).
6. Salta a la direccion almacenada en `stvec`. En xv6, mientras `P` estaba en modo usuario, `stvec` apuntaba a `uservec` (en `trampoline.S`).

Hasta aca, el unico cambio de estado del proceso es que su `pc` paso a vivir en `sepc`.

## Paso 2. `uservec` salva el estado de usuario en el trapframe

`uservec` es codigo del kernel mapeado en una pagina especial (`TRAMPOLINE`) que existe tanto en la pagetable del proceso como en la del kernel. Asi puede ejecutarse antes de cambiar la MMU. Sus tareas son:

1. Salvar **todos los registros de proposito general** del proceso en su `trapframe` (cuya direccion virtual estaba precargada en `sscratch`).
2. Cargar el `satp` del kernel desde el trapframe -> la MMU pasa a usar la tabla de paginas del kernel.
3. Cargar el stack kernel del proceso.
4. Saltar a `usertrap()` (rutina C en `trap.c`).

Al final del paso 2, el snapshot del estado de usuario de `P` ya esta seguro en su `trapframe`.

## Paso 3. `usertrap()` reconoce el motivo

En modo supervisor, con la pagetable del kernel y el stack kernel de `P`:

1. Setea `stvec = kernelvec` (si entrar al kernel cae otro trap, lo maneja un handler distinto).
2. Salva `sepc` en el trapframe (por si la llamada al scheduler lo sobrescribe mas adelante).
3. Lee `scause` y, via `devintr()`, identifica que es la **interrupcion del timer**.
4. `devintr()` actualiza el contador global de ticks y despierta a los procesos durmiendo en ese canal.
5. Como fue una interrupcion de timer, `usertrap()` invoca a `yield()`: este es exactamente el punto en el que el quantum se considera consumido.

## Paso 4. `yield()` cambia el estado a `RUNNABLE`

`yield()` hace tres cosas:

1. Toma el lock del proceso (`acquire(&p->lock)`).
2. Cambia `p->state` de `RUNNING` a **`RUNNABLE`**.
3. Llama a `sched()`.

`sched()` chequea invariantes (lock tomado, interrupciones deshabilitadas, no estamos en modo dormido) y llama a `swtch(&p->context, &cpu->scheduler)`, que:

- Salva los registros **callee-saved** de `P` (`ra`, `sp`, `s0..s11`) en `p->context`.
- Restaura los registros del contexto del scheduler (`cpu->scheduler`).
- Retorna a la rutina `scheduler()`, que estaba esperando exactamente despues de su propio `swtch()`.

A partir de aca, `P` esta **congelado en `RUNNABLE`**, sin CPU. Su trapframe guarda el estado de modo usuario y su `context` guarda el punto al que retornara dentro de `sched()` la proxima vez.

## Paso 5. El scheduler elige otro proceso `Q`

`scheduler()` en `proc.c` recorre la tabla de procesos buscando uno en estado `RUNNABLE`. Cuando encuentra a `Q`:

1. Marca `Q->state = RUNNING`.
2. Llama a `swtch(&cpu->scheduler, &Q->context)` -> `Q` reanuda en su propia `sched()`/`yield()`.
3. `Q` desenrolla la pila hasta `usertrapret()` y retorna eventualmente a modo usuario.

Mientras tanto, `P` permanece en `RUNNABLE`, listo para ser elegido nuevamente cuando el scheduler vuelva a buscar candidatos.

## Paso 6. El scheduler vuelve a `P`

En algun momento posterior `scheduler()` lo elige otra vez:

1. `swtch(&cpu->scheduler, &p->context)` -> `P` reanuda dentro de `sched()`, justo despues del `swtch()` de salida del paso 4.
2. `sched()` retorna a `yield()`, que libera el lock y retorna.
3. `yield()` retorna a `usertrap()`.

## Paso 7. `usertrapret()` prepara el regreso a usuario

`usertrap()` continua en `usertrapret()`, que:

1. Deshabilita interrupciones mientras manipula estado sensible.
2. Restaura `stvec = uservec` (proxima entrada por trap desde usuario).
3. Reescribe en el trapframe los datos del kernel que `uservec` necesita la proxima vez (kernel\_satp, kernel\_sp, kernel\_trap, hartid).
4. Configura `sstatus.SPP = 0` (al hacer `sret` se vuelve a modo usuario).
5. Carga `sepc` con el `pc` original del proceso al ser interrumpido.
6. Salta a `userret` (en `trampoline.S`) pasandole el `satp` del proceso.

## Paso 8. `userret` y `sret`: vuelta a usuario

`userret`:

1. Conmuta a la pagetable del proceso (`csrw satp, a0; sfence.vma`).
2. Restaura todos los registros de proposito general desde el trapframe.
3. Ejecuta `sret`.

`sret`, en una sola operacion atomica:

- Restaura `pc` a partir de `sepc` (instruccion siguiente a la interrumpida).
- Restaura el modo segun `sstatus.SPP` -> vuelve a `U`.
- Re-habilita interrupciones (`SIE = SPIE`).

`P` vuelve a ejecutar como si nada hubiese ocurrido, salvo que paso tiempo real porque la CPU estuvo corriendo otros procesos en el medio.

## Estado final del proceso

A lo largo de la rutina hay **dos** estados a distinguir:

- Mientras `P` esta fuera de la CPU (entre los pasos 4 y 6), su estado en el PCB es **`RUNNABLE`**. Esa es la respuesta directa a la pregunta del enunciado: el kernel deja a `P` en `RUNNABLE` despues de consumir su quantum.
- Cuando finalmente retorna de la interrupcion, `P` esta nuevamente en `RUNNING` y ejecutando en modo usuario.

La clave es que la interrupcion del timer no bloquea al proceso: lo unico que hace es desalojarlo para darle CPU a otro. Por eso el estado correcto es `RUNNABLE` (sigue siendo elegible para ser planificado), no `SLEEPING` (que se reservaria para esperas por eventos como I/O).

## Resumen de la ruta completa

```text
  [user mode, proceso P]
           |
           |  HW: timer irq -> sepc, sstatus.SPP, scause, salta a stvec
           v
  uservec (trampoline.S)            <-- salva regs de usuario en trapframe
           |
  usertrap() en trap.c              <-- reconoce timer via devintr()
           |
  yield()  -> p->state = RUNNABLE   <-- *aca* el kernel deja a P
           |
  sched()  -> swtch(&p->context, &cpu->scheduler)   <-- 1er context switch
           v
  scheduler() elige otro Q
           | swtch(&cpu->scheduler, &Q->context)
           |  ... eventualmente, scheduler vuelve a P ...
           v
  swtch() reanuda en sched() / yield() de P
           v
  usertrapret() -> userret -> sret  <-- vuelve a modo usuario
           v
  [user mode, P continua ejecutando]
```

Puntos clave que pide el ejercicio:

1. La interrupcion la atrapa el hardware y se transfiere a `uservec` -> `usertrap()`.
2. El kernel detecta que es timer y, tratandose del fin del quantum, desaloja al proceso con `yield()`.
3. El estado del proceso en el PCB queda en **`RUNNABLE`** (ni `SLEEPING` ni `ZOMBIE`).
4. Entre los dos `swtch()` (saliente al scheduler, entrante a `Q`) hay tiempo arbitrario donde otros procesos pueden ejecutar.
5. La vuelta a usuario se hace por `usertrapret`/`userret`/`sret`, restaurando exactamente el estado guardado en el trapframe.
