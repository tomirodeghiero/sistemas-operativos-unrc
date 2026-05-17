# Taller 4: Threads y cambios de contexto

**Universidad Nacional de Río Cuarto** · Sistemas Operativos (Código 1965)

**Alumno:** Tomás Rodeghiero · **Fecha:** Mayo de 2026

## Consigna

Taller 4: threads y cambios de contexto.

1. Descomprimir el paso `04-context-switch` y leer el `README.md`.
2. Analizar cómo están implementados los threads y el `context_switch`
   en `kernel.c`, `arch.c` y `arch.s` (ejercicio 1).
3. Responder por escrito por qué se guardan los registros `s0-s11`
   y por qué no se guardan todos (ejercicios 2 y 3).
4. Modificar `kernel.c` para crear un `task_b` además de `task_a` y
   producir la traza (ejercicio 4):

   ```
   main_thread -> task_a -> task_b -> task_a -> main_thread -> task_b
   ```

## Estructura de la entrega

- `04-context-switch/`: paso original tal como viene en el `.tgz`,
  con el `kernel.c` **sin modificar** (queda como referencia).
- `entrega/kernel.c`: versión modificada que se entrega.
- `entrega/respuestas.txt`: respuestas a los cuatro ejercicios del README.
- `entrega/salida-kernel.txt`: salida de consola observada al ejecutar el kernel modificado.
- `informe-taller4-context-switch.tex` / `.pdf`: informe.
- `README.md`: este archivo.

## Archivos modificados

Solo `kernel.c`. No toqué `arch.c`, `arch.s`, el `Makefile`, ni el resto del proyecto.

## Compilar y ejecutar

Requisitos: toolchain RISC-V y `qemu-system-riscv32`.

Para probar la solución, copio el `kernel.c` modificado sobre el original,
compilo y lanzo QEMU en una sola línea:

```bash
cp entrega/kernel.c 04-context-switch/kernel.c \
  && make -C 04-context-switch clean \
  && make -C 04-context-switch qemu PREFIX=riscv64-unknown-elf-
```

`make qemu` invoca:

```bash
qemu-system-riscv32 -machine virt -bios none -nographic --no-reboot -smp 1 -kernel kernel
```

Para salir de QEMU: `Ctrl-A` y luego `x`.

## Salida observada

```text
In kernel_main() thread
Task A on cpu 0
Task B on cpu 0
Task A again on cpu 0
In kernel_main() thread again
Task B again on cpu 0
Kernel stopped at kernel.c:68
```

Las seis líneas que preceden al `stop()` corresponden, en orden, a la
traza pedida: `main_thread → task_a → task_b → task_a → main_thread → task_b`.

## Captura de pantalla

Para generar la captura `captura-salida-kernel.png` desde el entorno local:

- En macOS: ejecutar `make qemu PREFIX=...` en una terminal y, una vez
  que aparece la salida, presionar `Cmd+Shift+4` y seleccionar el área
  de la terminal. Guardar la imagen como `entrega/captura-salida-kernel.png`.
- En Linux: usar `gnome-screenshot -a`, `flameshot gui` o equivalente.

La salida en texto plano queda en `entrega/salida-kernel.txt` por si el
corrector prefiere copiarla directamente.

## Respuestas a los ejercicios

Están en `entrega/respuestas.txt`. En síntesis:

- **Ej. 1:** Un hilo es su pila más su `sp` guardado. `init_task_context`
  (en `arch.c`) arma la pila inicial con 12 ceros y el `pc` en lugar de
  `ra`. `context_switch` (en `arch.s`) hace push/pop de `ra` y `s0-s11`,
  intercambia `sp` y retorna; el `ret` final salta al punto donde el
  hilo entrante había cedido la CPU.
- **Ej. 2:** `s0-s11` son *callee-saved* en la ABI de RISC-V. El hilo
  que cede la CPU puede tener variables vivas ahí; sin guardarlas
  quedarían corruptas cuando reanude. Junto con `ra` (que dice a dónde
  volver) son lo mínimo para que el cambio de pila sea transparente.
- **Ej. 3:** Los *caller-saved* (`t0-t6`, `a0-a7`) no hace falta
  guardarlos porque, por la ABI, el compilador ya derramó los que el
  hilo llamador quería preservar. Y los registros especiales (`sp`,
  `tp`, `gp`, `zero`) o se intercambian aparte (`sp`) o son propiedad
  de la CPU, no del hilo. Guardar más sería desperdicio.
- **Ej. 4:** Se agregó `task_b` y se ajustó `kernel_main`, `task_a` y
  `task_b` para producir la traza pedida (ver el `kernel.c` modificado).

## Cambio en `kernel.c`

Agregué un segundo task (`task_b`) con su propia pila y `sp` guardado,
y dispuse los `context_switch` en cada tarea para producir la traza
pedida. Las pistas claves:

- `task_a` cede primero a `task_b` y, cuando lo reanuden, cede a `main`.
- `task_b` cede primero a `task_a` y, cuando lo reanuden, cede a `main`.
- `kernel_main()` arranca `task_a`, y cuando vuelve, arranca `task_b`.

No modifiqué la lógica de bajo nivel (`init_task_context`, `context_switch`
en `arch.s`), solo el código C de las tareas.
