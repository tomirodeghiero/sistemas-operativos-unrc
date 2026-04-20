# Resolucion 9 - Practico 3

Ejercicio 9: describir el flujo de ejecucion cuando un proceso es interrumpido por el timer hasta retornar de la interrupcion.

La descripcion toma como referencia xv6 sobre RISC-V en modo Sv39, que es el modelo visto en la teoria.

## Punto de partida

La tarea P esta en modo usuario ejecutando codigo normal cuando ocurre una **interrupcion del timer** generada por el hardware (CLINT en RISC-V, interrupcion de reloj periodica programada al arranque del kernel).

## Paso 1: hardware atrapa la interrupcion

La CPU, al recibir la interrupcion del timer:

1. Verifica que las interrupciones esten habilitadas en supervisor mode (`sstatus.SIE = 1` para traps en modo S; el timer de M-mode se delega luego a S).
2. Salva el `pc` en `sepc`.
3. Salva el modo previo en `sstatus.SPP` (aqui: `U`, modo usuario).
4. Pone en `scause` el codigo de la interrupcion (timer = supervisor timer interrupt).
5. Cambia a supervisor mode.
6. Salta a la direccion almacenada en `stvec`. En xv6, cuando el proceso estaba en modo usuario, `stvec` apunta a `uservec` (en `trampoline.S`).

Esto todo lo hace la CPU en un solo ciclo de trap, sin intervencion de software.

## Paso 2: uservec en el trampoline

`uservec` es codigo del kernel mapeado en la ultima pagina del espacio de direcciones del proceso (`TRAMPOLINE`), por lo que sigue siendo ejecutable aunque todavia este activa la `pagetable` del proceso. Sus funciones:

1. Salva **todos los registros de proposito general** en el `trapframe` del proceso (cuya direccion esta pre-cargada en `sscratch`).
2. Carga el `satp` del kernel desde el trapframe -> la CPU pasa a usar la tabla de paginas del kernel.
3. Carga el stack kernel del proceso.
4. Salta a `usertrap()` (C) en `trap.c`.

## Paso 3: usertrap()

En modo supervisor, con la pagetable del kernel y el stack del kernel del proceso:

1. Setea `stvec = kernelvec` (si ocurre otro trap dentro del kernel, ahora se maneja distinto).
2. Salva `sepc` en el trapframe (por si la llamada al scheduler la sobrescribe).
3. Lee `scause` y reconoce que es una interrupcion del timer (chequea `devintr()`).
4. `devintr()` reconoce al timer, actualiza el contador global de ticks y despierta a los procesos dormidos esperando tick.
5. Como fue interrupcion del timer, `usertrap()` invoca a **`yield()`** para ceder la CPU. Este es el comportamiento que hace preemptive al planificador: en cada tick, la tarea corriente se pone en `RUNNABLE` y pide al scheduler correr a otra.

## Paso 4: yield() y context switch

`yield()`:

1. Toma el lock del proceso (`p->lock`).
2. Cambia el estado de `RUNNING` a `RUNNABLE`.
3. Llama a `sched()`.

`sched()` hace algunos chequeos de invariantes y llama a `swtch(&p->context, &cpu->scheduler)`, que:

- Guarda los **registros callee-saved** (`ra`, `sp`, `s0..s11`) de la tarea actual en `p->context`.
- Carga los del scheduler desde `cpu->scheduler`.
- Retorna con `ret` a la funcion `scheduler()`.

A partir de aqui la tarea P queda "congelada" en `RUNNABLE`. El scheduler puede elegir otro proceso Q y hacer `swtch(&cpu->scheduler, &Q->context)`, con lo que Q reanuda donde habia sido desplanificado anteriormente.

## Paso 5: eventualmente, el scheduler reanuda a P

Cuando el scheduler decide volver a P, hace:

1. `swtch(&cpu->scheduler, &p->context)` -> reanuda `sched()` en el punto donde habia cedido.
2. `sched()` retorna a `yield()`, que suelta el lock y retorna.
3. `yield()` retorna a `usertrap()`.

## Paso 6: usertrapret()

`usertrap()` continua llamando a `usertrapret()`:

1. Deshabilita interrupciones (`sstatus.SIE = 0`) mientras manipula estado sensible.
2. Setea `stvec = uservec` otra vez (para el proximo trap desde usuario).
3. Escribe en el trapframe los valores del kernel que `uservec` necesitara en la proxima entrada (satp del kernel, trap handler, hartid, etc.).
4. Prepara `sstatus` para que `sret` vuelva a modo usuario (`SPP = 0`).
5. Carga en `sepc` el `pc` donde la tarea fue interrumpida.
6. Salta a `userret` (en `trampoline.S`) pasandole el `satp` del proceso.

## Paso 7: userret restaura y ejecuta sret

`userret`:

1. Conmuta a la tabla de paginas del proceso (`csrw satp, a0; sfence.vma`).
2. Restaura **todos los registros de proposito general** desde el trapframe.
3. Ejecuta `sret`.

`sret`:

- Restaura `pc = sepc` -> la instruccion que seguia a la interrumpida en el proceso.
- Restaura el modo segun `sstatus.SPP` (`U`, modo usuario).
- Rehabilita interrupciones (`sstatus.SIE = SPIE`).

La tarea P continua ejecutando como si nada hubiera pasado (salvo que ya transcurrio tiempo real porque la CPU estuvo ocupada corriendo otros procesos en el medio).

## Resumen de la ruta completa

```text
  [user mode, proceso P]
           |
           |  HW: timer irq
           v
  stvec -> uservec (trampoline.S)
           | salva regs en trapframe, cambia satp, salta a C
           v
  usertrap() en trap.c
           | devintr() reconoce timer, yield()
           v
  yield() -> sched() -> swtch()        <-- context switch saliente
           | pasa a cpu->scheduler
           v
  scheduler() elige otro proceso Q
           | swtch() -- Q ejecuta ...
           | ... eventualmente, scheduler vuelve a elegir P
           v
  swtch() retorna a sched()/yield() en P
           v
  usertrap() -> usertrapret()
           | prepara trapframe y stvec
           v
  userret (trampoline.S)
           | restaura regs y satp del proceso
           v
  sret
           v
  [user mode, proceso P continua]
```

Puntos clave:

- Hay **dos** context switches: uno al entrar al scheduler, otro al salir hacia la tarea siguiente.
- El trapframe guarda los registros de modo usuario; el `struct context` guarda los callee-saved para `swtch()` entre kernel y scheduler.
- `sret` (no una instruccion de retorno comun) es la que completa el retorno a modo usuario restaurando modo y `pc` en un solo paso atomico.
