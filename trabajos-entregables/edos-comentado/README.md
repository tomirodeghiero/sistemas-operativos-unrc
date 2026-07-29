# EDOS — Versión comentada línea por línea

Kernel didáctico para RISC-V 32 (QEMU virt), con todos los archivos fuente
comentados en español para que cualquier estudiante que recién empieza con
Sistemas Operativos pueda entender el código de arriba a abajo sin
necesidad de buscar afuera.

Es una copia funcional de `trabajos-entregables/08-process/`. Compila y corre
exactamente igual; lo único que cambia es que cada archivo tiene un
comentario de cabecera, cada función un mini-bloque, y casi cada línea no
trivial un comentario al costado.

---

## Cómo compilar

Requiere el toolchain cross para RISC-V:

- **macOS**: `brew tap riscv-software-src/riscv && brew install riscv-gnu-toolchain`
- **Linux**: `sudo apt install gcc-riscv64-linux-gnu` (y ajustar `PREFIX` en los Makefiles)

Requiere QEMU con soporte RISC-V:

- **macOS**: `brew install qemu`
- **Linux**: `sudo apt install qemu-system-misc`

Después, desde esta carpeta:

```bash
make            # compila userland + kernel + genera kernel.asm
make qemu       # arranca QEMU con el kernel
make clean      # limpia binarios
```

Para salir de QEMU: `Ctrl+A` y luego `X`.

---

## Orden RECOMENDADO de lectura

Leelos EN ESTE ORDEN, de más simple/fundacional a más complejo. Cada archivo
depende conceptualmente de los anteriores.

### 1. Definiciones básicas (los "cimientos")

1. **`types.h`** — Primero, porque define `uint8`, `uint32`, `paddr`, `vaddr`, `size_t`, `NULL`. Todo el kernel usa estos tipos.
2. **`klib.h`** — API de la mini-libc del kernel (`printf`, `memcpy`, `panic`). Aparece en casi todos los `.c`.
3. **`spinlock.h`** — Tipo `spinlock` y funciones `acquire/release`. Muy corto, mejor sacarlo del camino ya.

### 2. Arquitectura (la capa de hardware)

4. **`arch/riscv32/riscv32.h`** — EL header más importante. Define CSR helpers, `struct trap_frame`, `struct context`, flags de PTE, direcciones MMIO. Todo lo que sea "cómo hablar con la CPU" está acá. Se accede como `arch.h` (symlink).
5. **`kernel.ld`** — Layout del kernel en memoria (dónde arranca, dónde va `.text`, `.bss`, dónde termina). Ver esto ANTES de arch.s ayuda a entender qué es `__stack0`, `__kernel_end`, etc.
6. **`arch/riscv32/riscv32.s`** — Punto de entrada del kernel (`boot`), transición M→S mode, PMP, delegación de traps, y la implementación de `context_switch`.
7. **`arch/riscv32/riscv32-traps.s`** — ISRs de bajo nivel: `s_trap`, `u_trap`, `u_trap_ret`, `m_trap`. Explica el "truco del sscratch" para intercambiar sp usuario ↔ kernel.
8. **`arch/riscv32/riscv32.c`** — Funciones de arch escritas en C: rearme del timer, manipulación de tablas de páginas Sv32 (`map_page`, `unmap_page`, `map_kernel_memory`).

### 3. Núcleo del kernel

9. **`kmain.c`** — El "main" del kernel. Muy cortito. Coordina el arranque de CPU 0 y CPU N y llama al scheduler.
10. **`console.h` + `console.c`** — Driver del UART. Es el I/O más simple del kernel: sale printf, entra teclado.
11. **`klib.c`** — Implementación de `printf`, `memset`, `strcmp`, etc.
12. **`spinlock.c`** — Implementación de los locks (con `__sync_lock_test_and_set` y push/pop de IRQs). Explica por qué NO se puede tomar un lock con IRQs habilitadas.

### 4. Memoria (páginas + memoria virtual)

13. **`kalloc.h` + `kalloc.c`** — Allocator de páginas físicas (free-list simple). Muy fácil.
14. **`vm.h` + `vm.c`** — Envoltorios de alto nivel sobre `map_page`: `map_region`, `unmap_region`, `copy_from_user`, `copy_to_user`.

### 5. Procesos y planificación

15. **`task.h`** — El PCB (`struct task`), estados (RUNNABLE, RUNNING, WAITING, ...), API del scheduler y de las primitivas de sync (yield, sleep, suspend, wakeup).
16. **`task.c`** — Implementación del scheduler round-robin, context switch de alto nivel, creación de procesos (`create_process`, `exec`, `load_program`).

### 6. Traps y syscalls

17. **`trap.c`** — Handlers de alto nivel: `kernel_trap` (traps en S-mode) y `user_trap` (traps desde U-mode). Acá se decide si el trap es un timer IRQ, una syscall, un page fault, etc.
18. **`syscall.h`** — Números de las syscalls (contrato user ↔ kernel).
19. **`syscall.c`** — Dispatcher e implementación de cada `sys_*`. Muy pedagógico.

### 7. Filesystem embebido

20. **`efs.h`** — Descriptor `struct file` y la búsqueda por nombre.
21. **`efs.c`** — Implementación de `efs_file()` (búsqueda lineal).
22. **`efsfiles.c`** — Autogenerado por `user/mkefs.sh`. Contiene el binario de `init` como array de bytes.

### 8. Programas de usuario

23. **`user/user.ld`** — Layout del binario userland (arranca en vaddr 0).
24. **`user/usys.s`** — Stubs de syscall en assembly. Un `ecall` por función.
25. **`user/edoslib.h`** — API que ven los programas de usuario.
26. **`user/edoslib.c`** — Implementación userland de `printf`, `memcpy`, y el `start` (crt0).
27. **`user/init.c`** — El proceso `init`, la única "aplicación" que corre. Súper cortito: imprime, duerme y termina.
28. **`user/Makefile` + `user/mkefs.sh`** — Cómo se compila userland y cómo se empaqueta en el EFS del kernel.

### 9. Build system

29. **`Makefile`** — Build del kernel + regla `qemu` para correrlo.

---

## Diagrama de arranque (bird's-eye view)

```
   Firmware QEMU carga la imagen en 0x80000000 y salta a `boot`
              │
              ▼
   arch/riscv32/riscv32.s : boot
     - guarda hartid en tp
     - asigna sp a cada CPU
     - configura PMP
     - delega traps a S-mode (medeleg/mideleg)
     - habilita timer IRQ
     - mret ─────────────► S-mode
              │
              ▼
   arch/riscv32/riscv32.s : supervisor
     - instala s_trap como handler
     - call kernel_main
              │
              ▼
   kmain.c : kernel_main()
     - CPU 0: init_kalloc, init_vm, create_process("init")
     - todas: enable_paging, scheduler()
              │
              ▼
   task.c : scheduler()
     - loop { por cada task RUNNABLE, context_switch }
              │
              ▼  (context_switch)
   task.c : start_process()
     - release(&task->lock)
     - return_to_user_mode()
              │
              ▼
   trap.c : return_to_user_mode()
     - stvec = u_trap
     - sscratch = kstack top
     - satp = task->pgtbl
     - u_trap_ret(trap_frame)
              │
              ▼
   arch/riscv32/riscv32-traps.s : u_trap_ret
     - restaura regs
     - sret ─────────────► U-mode
              │
              ▼
   user/edoslib.c : start()
     - main() (init.c)
     - exit()
              │
              ▼ (syscall ecall)
   arch/riscv32/riscv32-traps.s : u_trap
     - swap sp <-> sscratch (ahora usa kstack)
     - guarda trap_frame
     - call user_trap
              │
              ▼
   trap.c : user_trap()
     - switch(scause) { SYSCALL → syscall(task) → ... }
     - return_to_user_mode() (loop)
```

---

## Ciclo de vida de un proceso

```
   create_process("init")
        │
        ▼
   ┌─────────┐   scheduler   ┌─────────┐  yield/timer  ┌─────────┐
   │CREATED  │──────────────▶│RUNNABLE │──────────────▶│RUNNING  │
   └─────────┘               └─────────┘◀──────────────└────┬────┘
                                  ▲                         │
                                  │ wakeup(cond)            │ sleep(n)
                                  │                         │ suspend(cond)
                                  └─────────────────────────┘
                                       ┌─────────┐
                                       │WAITING  │
                                       └─────────┘

   Cuando exit(): RUNNING ──▶ ZOMBIE ──▶ (scheduler libera kstack) ──▶ UNUSED
```

---

## Glosario de siglas

| Sigla | Significado |
| :--- | :--- |
| **PCB** | Process Control Block. En EDOS: `struct task` (task.h). |
| **PTE** | Page Table Entry. Una fila de 32 bits en una tabla de páginas Sv32. |
| **MMU** | Memory Management Unit. Hardware que traduce vaddr → paddr. |
| **TLB** | Translation Lookaside Buffer. Caché de traducciones vaddr→paddr. Se vacía con `sfence.vma`. |
| **CSR** | Control-Status Register. Registros especiales de la CPU (satp, sstatus, sepc, ...). |
| **hart** | HARdware Thread. En RISC-V, cada núcleo lógico se llama "hart". Alternativa a "CPU". |
| **CLINT** | Core Local INTerruptor. Timer y software IRQ locales por hart. |
| **PLIC** | Platform-Level Interrupt Controller. Router de IRQs externas a los harts. |
| **MMIO** | Memory-Mapped I/O. Leer/escribir un dispositivo escribiendo bytes en una dirección física. |
| **satp** | Supervisor Address Translation and Protection. CSR que apunta a la tabla de páginas activa. |
| **sepc** | Supervisor Exception PC. PC guardado al ocurrir un trap. |
| **stvec** | Supervisor Trap Vector. Dirección del handler de traps de S-mode. |
| **scause** | Supervisor Cause. Motivo del último trap. |
| **stval** | Supervisor Trap Value. Dirección "culpable" en un page/access fault. |
| **sscratch** | Supervisor SCRATCH. Registro auxiliar; en EDOS guarda el tope del kstack. |
| **sstatus.SPP** | Supervisor Previous Privilege. Qué modo estaba antes del trap (para volver ahí con sret). |
| **sstatus.SIE** | Supervisor Interrupt Enable. Habilita IRQs de S-mode. |
| **ecall** | Environment Call. Instrucción que dispara un trap de "syscall". |
| **PMP** | Physical Memory Protection. Rangos de memoria accesibles por cada modo. |
| **UART** | Universal Asynchronous Receiver-Transmitter. Nuestra consola serie (16550 en QEMU). |
| **EFS** | Embedded File System. Los binarios de userland vienen dentro del kernel como arrays de bytes. |
| **Sv32** | Modo de paginación RISC-V 32 con 2 niveles y páginas de 4 KB. |
| **PPN** | Physical Page Number. Los 20 bits altos de una paddr (que caben en un PTE). |
| **ABI** | Application Binary Interface. Convención de qué registro es qué (a0 = 1er arg, a7 = syscall, etc.). |
