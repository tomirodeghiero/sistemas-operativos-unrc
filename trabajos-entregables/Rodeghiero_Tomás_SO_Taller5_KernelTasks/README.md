# Taller 5: Preemptive kernel tasks

**Universidad Nacional de Río Cuarto** · Sistemas Operativos (Código 1965)

**Alumno:** Tomás Rodeghiero · **Fecha:** Junio de 2026

## Consignas

Se encuentranan en la carpeta `05-kernel-tasks`. Hay siete ejercicios en el archivo `README.html`:

1. Tiempo hasta el overflow de un timer de 32 bits a 1 MHz.
2. Ídem con 64 bits.
3. Análisis de la creación y manejo de tareas:
   diagrama de transiciones, política de scheduling, qué corre cuando no
   hay tareas `RUNNABLE`.
4. Dónde y cuándo se guarda el *trap frame*.
5. Compilar, correr y analizar la salida (con distintos `QUANTUM`).
6. Modificar el kernel para dar un quantum **propio a cada tarea**.
7. Modificar `trap()` para que un task pueda **recuperarse** de una
   *invalid instruction exception*.

## Estructura de la entrega

- `05-kernel-tasks/`: paso original, intacto (queda como referencia).
- `entrega/task.h`, `task.c`, `kmain.c`, `trap.c`: versiones modificadas
  de los cuatro archivos que toqué.
- `entrega/respuestas.txt`: respuestas a los siete ejercicios.
- `entrega/salida-kernel.txt`: salida observada con el kernel modificado.
- `entrega/salida-kernel-original.txt`: salida observada con el kernel
  sin modificar (para comparar).
- `informe-taller5-kernel-tasks.pdf`: informe.
- `README.md`: este archivo.

## Archivos modificados

Cuatro archivos.

- `task.h`: agregué `int quantum` en `struct task` y un tercer parámetro
  `int quantum` en `create_task()`.
- `task.c`: `create_task()` guarda el quantum recibido (con caída al
  `QUANTUM` por defecto si pasa `<= 0`) y `scheduler()` usa
  `next_task->quantum` en vez de la constante `QUANTUM`.
- `kmain.c`: cada `create_task()` pasa un quantum distinto
  (`A=1`, `B=3`, `C=2`) para que la diferencia se note.
- `trap.c`: en la rama `ILLEGAL_INSTRUCTION`, en vez de `terminate(-1)`
  hago `ra += 4` para saltar la instrucción y dejo seguir a la tarea
  (todas las instrucciones miden 4 bytes porque el `Makefile` compila
  con `-march=rv32imazicsr`, sin la extensión C).

## Compilar y ejecutar

Requisitos: toolchain `riscv64-unknown-elf-` y `qemu-system-riscv32`.

Para probar la solución, copio los cuatro archivos modificados sobre el
paso original, limpio y compilo:

```bash
cp entrega/task.h entrega/task.c entrega/kmain.c entrega/trap.c \
   05-kernel-tasks/
make -C 05-kernel-tasks clean PREFIX=riscv64-unknown-elf-
make -C 05-kernel-tasks qemu  PREFIX=riscv64-unknown-elf-
```

`make qemu` invoca:

```bash
qemu-system-riscv32 -machine virt -bios none -nographic --no-reboot \
                    -smp 2 -kernel kernel
```

Para salir de QEMU: `Ctrl-A` y luego `x`.

## Respuestas a los ejercicios

Están en `entrega/respuestas.txt` y desarrolladas en el informe. En síntesis:

- **Ej. 1:** ~49.71 días (~49 d 17 h 2 min 47 s).
- **Ej. 2:** ~584.942 años. Por eso `CLINT_MTIME` en RISC-V es de 64 bits.
- **Ej. 3:** Estados `UNUSED → RUNNABLE → RUNNING ↔ {WAITING, RUNNABLE,
  TERMINATED}`, con `create_task`/`scheduler`/`yield`/`suspend`/`wakeup`/
  `terminate` controlando cada arista. Política: *round-robin* con desalojo
  por timer. Cuando no hay `RUNNABLE`, el `while(true)` de `scheduler()`
  gira en vacío (*busy idle*), no hay `wfi` ni idle task.
- **Ej. 4:** En la pila kernel del task vigente, en la entrada a `s_trap`
  (`arch.s`), que reserva 30 palabras y hace push de los 30 registros de
  `struct trap_frame` antes de `call trap`.
- **Ej. 5:** Compila con un warning de `tf` no usado en `trap()`. La
  salida muestra A y C alternándose, B durmiendo y los dos primeros
  muriendo al ejecutar `unimp` en `ticks==3`.
- **Ej. 6:** Quantum como campo de `struct task` y nuevo parámetro de
  `create_task`; el scheduler lo carga al elegir la tarea.
- **Ej. 7:** En la rama `ILLEGAL_INSTRUCTION` de `trap()`, `ra += 4`
  antes del `set_trap_ra(ra)` final; así el `sret` de `s_trap` salta a
  la siguiente instrucción en vez de re-ejecutar la inválida.
