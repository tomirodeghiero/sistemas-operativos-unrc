# Resolucion 10 - Practico 3

Ejercicio 10: diferencia entre el **trapframe** y el **contexto salvado** de cada tarea. Cuando se usa cada uno.

## Idea general

Ambos guardan registros de la CPU, pero en momentos distintos del flujo del kernel y con proposito distinto:

- **Trapframe**: registros salvados cuando la CPU **atraviesa la frontera entre modo usuario y modo kernel** (entrada y salida del kernel por un trap).
- **Contexto salvado** (`struct context` en xv6): registros salvados cuando **el kernel cambia de tarea dentro de modo supervisor** (context switch entre una tarea y el scheduler, y entre el scheduler y otra tarea).

Son dos niveles distintos de salvataje y se usan para cosas distintas.

## Trapframe

Definido en xv6 en `kernel/proc.h`. Contiene:

- Registros de proposito general del modo usuario (`x0`-`x31`) incluyendo `sp` y `ra` de modo usuario.
- `pc` previo al trap (guardado originalmente por hw en `sepc`, copiado al trapframe).
- Informacion auxiliar que `uservec` necesita para volver al kernel en la proxima entrada: `kernel_satp`, `kernel_sp`, `kernel_trap`, `kernel_hartid`.

Cuando se escribe:

- En **`uservec`** (en `trampoline.S`), apenas entra al kernel desde modo usuario. Se salvan todos los registros del proceso antes de que el kernel los pise.

Cuando se lee / restaura:

- En **`userret`** (tambien en `trampoline.S`), justo antes de ejecutar `sret` para volver a modo usuario. Se restauran todos los registros y el `pc` de modo usuario.

Existe **uno por proceso**. Su direccion fisica se mapea en una direccion virtual fija del espacio de memoria del proceso (`TRAPFRAME`) para que `uservec` pueda accederlo sin importar si la pagetable activa es la del proceso o la del kernel.

Esencialmente, el trapframe es el snapshot del "modo usuario" de la tarea.

## Contexto salvado (struct context)

Definido en xv6 en `kernel/proc.h`. Contiene **solo** los registros **callee-saved** de la convencion de llamada RISC-V:

```text
ra, sp, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11
```

Nada mas: ni los caller-saved (que el codigo C que llame a `swtch` ya sabe que pueden cambiar), ni registros de estado.

Cuando se escribe / lee:

- En la funcion **`swtch(old, new)`** en `swtch.S`. Esta funcion salva los callee-saved en `*old` y los restaura desde `*new`. Es el corazon del context switch dentro del kernel.

Existe **uno por tarea** (en `struct proc`) y ademas **uno por CPU** (el contexto del scheduler, en `struct cpu`).

Esencialmente, el contexto guarda el "estado minimo necesario para reanudar una funcion en C". Al llamar a `swtch()` el resto de los registros importantes ya se guardaron en el stack por el codigo que llamo a `swtch`, gracias a la convencion de llamada.

## Como se usan en una preemption completa

Repaso del ejercicio 9 para ubicar ambos:

```text
modo usuario (P)
    |  irq timer
    v
uservec              ---- escribe TRAPFRAME de P
    |
usertrap() -> yield() -> sched()
    |
swtch(&P.context, &cpu.scheduler)   ---- escribe P.context, lee scheduler.context
    v
scheduler() (supervisor)
    |
swtch(&cpu.scheduler, &Q.context)   ---- escribe scheduler.context, lee Q.context
    v
(Q reanuda aqui en el kernel)
    |
usertrapret() -> userret   ---- lee TRAPFRAME de Q
    v
sret
    v
modo usuario (Q)
```

En esa secuencia:

- El **trapframe** se usa dos veces: `uservec` lo escribe al entrar, `userret` lo lee al salir. Preserva el estado "usuario" de la tarea.
- El **contexto salvado** se usa en cada llamada a `swtch()`. Preserva solo lo necesario para reanudar la funcion C correspondiente (`sched()` o `scheduler()`).

## Comparacion en tabla

|                              | Trapframe                                                                     | Contexto salvado                                                          |
|------------------------------|-------------------------------------------------------------------------------|---------------------------------------------------------------------------|
| Definido en                  | `struct trapframe` (proc.h)                                                   | `struct context` (proc.h)                                                 |
| Que guarda                   | Todos los registros de proposito general + `pc` usuario + info aux del kernel | Solo callee-saved (`ra`, `sp`, `s0`-`s11`)                                |
| Cuando se escribe            | `uservec` al entrar al kernel desde usuario                                   | `swtch()` en cada context switch                                          |
| Cuando se restaura           | `userret` al volver a usuario                                                 | `swtch()` en cada context switch                                          |
| Uno por                      | Proceso                                                                       | Proceso **y** CPU (para el scheduler)                                     |
| Cruza frontera user/kernel   | Si (es justamente para eso)                                                   | No (siempre en modo supervisor)                                           |
| Depende de la convencion C   | No, salva todo explicitamente                                                 | Si: los caller-saved quedan en el stack por la convencion RISC-V          |

## En una frase

- **Trapframe** = "estado de la tarea en modo usuario", salvado al entrar/salir del kernel.
- **Contexto** = "estado minimo para reanudar la funcion del kernel que hizo el switch", salvado en cada `swtch()` entre kernel y scheduler.
