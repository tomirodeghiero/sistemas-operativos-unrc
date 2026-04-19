# Parte 2 - Entrega (S-mode con manejo de traps)

**Universidad Nacional de Río Cuarto**

**Sistemas Operativos (Código 1965) - Taller 1**

**Alumno:** Tomás Rodeghiero

**Abril de 2026**

En este paso trabajé con el kernel que arranca en machine mode (M) y luego cambia a supervisor mode (S) antes de entrar a `kernel_main()`.

## Qué se hizo

1. Se ejecutó el código base de `01-hello-world-S-mode`.
2. Se verificó cómo se invoca `trap(cause)` desde `start.s`.
3. Se ejecutó el caso original (paso 4).
4. Se comentó la primera línea de `kernel_main()` y se volvió a ejecutar (paso 5).
5. Se implementó el TO DO en `trap()` para identificar tipo de trap y causa.

## Análisis de la invocación `trap(cause)`

En `start.s`, dentro de `s_trap`, se hace:

- `csrr a0, scause`
- `call trap`

Eso significa que el valor de `scause` se pasa como primer argumento en `a0` (convención de llamada RISC-V), y `trap(uint cause)` lo recibe en C como `cause`.

## ¿Por qué había excepción en el paso 4?

Porque en `kernel_main()` se ejecuta `csrr a0, mhartid`, y `mhartid` es un CSR de machine mode. Como `kernel_main()` ya corre en supervisor mode, esa lectura provoca una excepción de instrucción ilegal.

En RISC-V, `scause = 2` corresponde a `Illegal Instruction`, que es exactamente lo que se observa al ejecutar.

## Evidencias incluidas

- `00-paso4-base.txt`: ejecución con `mhartid` activo (dispara trap).
- `01-paso5-sin-mhartid.txt`: ejecución comentando la primera línea de `kernel_main()` (imprime Hello World).
- `02-paso7-trap-implementado.txt`: ejecución final con TO DO implementado mostrando tipo/código de trap.
- `kernel.c`: archivo final solicitado para entregar.

## Salidas observadas

### Paso 4 (base)

```text
trap!!!
```

### Paso 5 (comentando primera línea)

```text
Hello World from supervisor mode!
```

### Paso 7 (TO DO implementado)

```text
trap!!!
trap type: exception
exception code: 2
exception detail: Illegal Instruction
```

## Ejecutar localmente

```bash
cd trabajos-entregables/taller1-riscv/01-hello-world-S-mode
make -B PREFIX=riscv64-unknown-elf-
make qemu PREFIX=riscv64-unknown-elf-
```

Salir de QEMU: `Ctrl-A` y luego `x`.
