# Resolucion 9 - Practico 3

Ejercicio 9: idem al ejercicio anterior pero cuando el proceso ejecuta el syscall `read` sobre un archivo en disco. Esa operacion puede tardar mucho. Describir tambien como el proceso continua su ejecucion.

La descripcion sigue el modelo xv6 sobre RISC-V visto en la teoria. La diferencia con el ejercicio 8 esta en el motivo por el que el proceso entra al kernel y, sobre todo, en el **estado en que queda mientras espera** la respuesta del disco.

## Punto de partida

El proceso `P` esta en modo usuario y ejecuta:

```c
n = read(fd, buf, len);
```

`read` es una funcion de la libc que prepara los argumentos en los registros y dispara la trampa al kernel con la instruccion `ECALL` (en RISC-V). A partir de ese momento empieza la rutina del kernel.

## Paso 1. Trap por syscall (HW + uservec + usertrap)

El comportamiento del hardware es identico al del ejercicio 8: salva `pc` en `sepc`, salva `sstatus.SPP = U`, codifica el motivo en `scause` (en este caso, `8 = environment call from U-mode`), pasa a modo supervisor y salta a `stvec` -> `uservec`.

`uservec` salva los registros del proceso en su `trapframe` y conmuta a la pagetable y al stack del kernel. Despues llama a `usertrap()`.

`usertrap()` mira `scause`, ve que es un syscall (no una interrupcion), avanza `sepc` para que la vuelta apunte a la instruccion siguiente al `ECALL`, habilita interrupciones y llama a `syscall()`.

## Paso 2. Despacho a `sys_read`

`syscall()` lee el numero de syscall del registro `a7` (en xv6 es la convencion) y consulta la tabla `syscalls[]`. Para `SYS_read` ejecuta `sys_read()`, que:

1. Recupera los argumentos del trapframe (`fd`, `buf`, `len`).
2. Valida `fd` y obtiene el `struct file` correspondiente del descriptor.
3. Llama a `fileread(f, buf, len)`.

`fileread`, segun el tipo de `file`, despacha a una rutina especifica. Para un archivo regular en disco (file backed por inodo), termina llamando a `readi(ip, ...)` (lectura de inodo). `readi` recorre los bloques del archivo. Para cada bloque que no este en cache va al subsistema de buffer cache:

```c
struct buf *bp = bread(dev, blockno);
```

## Paso 3. El bloque no esta en cache: arranca I/O y se duerme

`bread()` busca el bloque en la cache. Hay dos casos:

- **Cache hit**: el bloque ya esta en RAM. Devuelve el buffer y se sigue copiando datos al usuario sin bloquearse.
- **Cache miss**: hay que ir al disco. Llama a `virtio_disk_rw(bp, 0)` para encolar una lectura al controlador VirtIO; despues llama a:

  ```c
  while ((bp->flags & B_VALID) == 0)
      sleep(bp, &lock);
  ```

`sleep()` es la primitiva clave: pone al proceso a dormir hasta que llegue el `wakeup` correspondiente. Su implementacion atraviesa un par de pasos delicados:

1. Toma `p->lock`.
2. Libera el lock que protege la condicion (`&lock` -> el lock del buffer).
3. Setea `p->chan = bp` (el "canal" sobre el que se duerme; aca el puntero al buffer).
4. Setea **`p->state = SLEEPING`**.
5. Llama a `sched()` -> `swtch(&p->context, &cpu->scheduler)`.

A partir de aca el proceso `P` esta `SLEEPING`, fuera de la cola de listos, y la CPU pasa al scheduler. Es justamente esto lo que distingue al `read` que va a disco del simple agotamiento de quantum: como la operacion puede tardar mucho, el kernel **no** deja al proceso `RUNNABLE`. Lo bloquea y le cede la CPU a quien la pueda aprovechar.

## Paso 4. El scheduler corre otro proceso `Q`

`scheduler()` busca un proceso `RUNNABLE` y hace `swtch(&cpu->scheduler, &Q->context)`. `Q` reanuda donde habia sido desplanificado. Mientras `Q` (o cualquier sucesion de procesos) corre, el disco sigue procesando la lectura **en paralelo** al CPU.

Si no hubiera otros procesos listos, el scheduler entra en un loop ocioso esperando interrupciones (en xv6 con `wfi`).

## Paso 5. El disco termina y manda IRQ

Cuando el controlador VirtIO termina de leer el bloque, lanza una interrupcion. El flujo es:

1. La CPU atrapa la IRQ siguiendo el mismo mecanismo del ejercicio 8 (entra por `uservec` o `kernelvec` segun donde estaba; tipicamente entra por `kernelvec` porque el sistema esta corriendo otro proceso del kernel).
2. `devintr()` reconoce la IRQ del disco y llama a `virtio_disk_intr()`.
3. `virtio_disk_intr()` toma el `bp` cuya operacion termino, marca el buffer como `B_VALID` (los datos ya estan disponibles), y llama a:

   ```c
   wakeup(bp);
   ```

`wakeup(bp)` recorre la tabla de procesos y, para cada uno con `state == SLEEPING && chan == bp`, le cambia el estado a **`RUNNABLE`**. En este caso, ese proceso es `P`. Su `chan` se vuelve `0`.

Notar que `wakeup` no le devuelve la CPU a `P` inmediatamente: solo lo vuelve a poner en la cola de listos. Decide cuando reanudarlo el scheduler.

## Paso 6. El scheduler vuelve a `P`

En algun momento posterior, `scheduler()` lo elige y hace `swtch(&cpu->scheduler, &p->context)`. `P` reanuda **exactamente donde habia parado**: dentro de `sleep()`, despues del `swtch()` de salida.

`sleep()` continua su rutina:

1. Toma de nuevo el lock que habia liberado en el paso 3.4.
2. Marca `p->state = RUNNING`, `p->chan = 0`.
3. Retorna al llamador, es decir al `while ((bp->flags & B_VALID) == 0) sleep(...)` de `bread()`.

Como ahora `B_VALID` esta seteado, el `while` termina, `bread` retorna el buffer y `readi` puede copiar los bytes desde el buffer al `buf` de usuario via `copyout()`.

## Paso 7. Retorno a modo usuario

Una vez que `readi` termina:

1. `fileread` y `sys_read` retornan al `usertrap()` original.
2. El valor de retorno de la syscall (la cantidad de bytes leidos) se escribe en `p->trapframe->a0`.
3. `usertrap()` invoca a `usertrapret()` (igual que en el ejercicio 8).
4. `usertrapret` prepara `stvec`, `sepc`, `sstatus`, etc., y salta a `userret`.
5. `userret` restaura todos los registros de usuario del trapframe (incluido `a0` con el valor de retorno) y ejecuta `sret`.
6. `sret` vuelve a modo usuario y reanuda en la instruccion siguiente al `ECALL`.

Desde el punto de vista del programa, simplemente la llamada `read(...)` retorno y la variable `n` recibio la cantidad de bytes leidos.

## Resumen de los estados por los que paso `P`

| Etapa                                                           | Estado de `P`        |
|-----------------------------------------------------------------|----------------------|
| Antes del `read`                                                | `RUNNING` (en CPU)   |
| Dentro de `sys_read`/`bread`, esperando I/O por cache miss      | **`SLEEPING`**       |
| Despues del `wakeup(bp)`                                        | **`RUNNABLE`**       |
| Cuando el scheduler lo vuelve a elegir                          | `RUNNING`            |

Notar la diferencia con el ejercicio 8:

- **Timer**: el quantum se acabo pero el proceso podia seguir consumiendo CPU. Lo dejamos `RUNNABLE`.
- **Read sobre disco**: el proceso no puede avanzar hasta que llegue el dato. Lo dejamos `SLEEPING`. El disco actua en paralelo al CPU y al terminar dispara un `wakeup` que recien ahi lo pone `RUNNABLE`.

## Como continua su ejecucion

El proceso continua su ejecucion gracias al par de primitivas **`sleep()` / `wakeup()`** del kernel:

1. `sleep()` lo saco de la CPU **y** lo bloqueo (estado `SLEEPING`, asociado al canal que es el buffer pendiente).
2. La interrupcion del disco corre en cualquier CPU disponible y, sin que `P` haya hecho nada, dispara `wakeup(bp)` que cambia el estado del proceso a `RUNNABLE`.
3. La proxima ronda del scheduler lo elige y le hace `swtch` para reanudarlo.
4. `P` reanuda dentro de `sleep()`, vuelve al `while` de `bread`, copia los datos al usuario y la syscall retorna a modo usuario por la ruta `usertrap -> usertrapret -> userret -> sret`.

Es un esquema asincronico clasico: el syscall **no** ocupa CPU mientras se hace la I/O; lo unico que mantiene es el espacio de memoria, el trapframe y el contexto guardado para poder retomar exactamente donde lo dejo.

## Diagrama del flujo

```text
  [user mode]                                        ECALL read(...)
        |
        v
  uservec -> usertrap() -> syscall() -> sys_read() -> fileread() -> readi() -> bread()
                                                                                |
                                       (cache miss -> virtio_disk_rw)           |
                                                                                v
                                                                            sleep(bp, ...)
                                                                  p->state = SLEEPING
                                                                  swtch -> scheduler
                                                                                |
                                                                                v
                                                              [otros procesos corren]
                                                                                |
                                                                  HW IRQ del disco -> devintr()
                                                                  virtio_disk_intr -> wakeup(bp)
                                                                  p->state = RUNNABLE
                                                                                |
                                                                  scheduler elige a P
                                                                                v
                                                                  sleep() retorna -> bread()
                                                                  -> readi() copia datos al user buf
                                                                  -> sys_read retorna n
                                                                                |
                                                                                v
                                                  usertrap -> usertrapret -> userret -> sret
                                                                                |
                                                                                v
  [user mode] read() retorno con n bytes leidos
```

Puntos clave que pide el ejercicio:

1. El syscall entra al kernel por la misma maquinaria de traps del ejercicio 8.
2. Si el bloque no esta en la buffer cache, el kernel emite la lectura al disco y bloquea al proceso con `sleep()`.
3. El estado del proceso queda en **`SLEEPING`**, asociado al canal del buffer.
4. Cuando el disco termina, su interrupcion corre `wakeup()`, que pasa al proceso a **`RUNNABLE`**.
5. El scheduler lo retoma con `swtch()` y la rutina del kernel continua exactamente donde habia dormido (`sleep` retorna). De ahi se completa la copia al buffer de usuario y se vuelve a modo usuario por `usertrapret`/`userret`/`sret`.
