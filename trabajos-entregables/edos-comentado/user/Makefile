# =============================================================================
# user/Makefile -- Compila los programas de usuario y arma el EFS.
# -----------------------------------------------------------------------------
# Rol dentro de EDOS:
#   Compila cada programa userland (por ahora, sólo `init`) linkeándolo con
#   `edoslib.o` (nuestra libc) y `usys.o` (wrappers de syscalls). Luego llama
#   a mkefs.sh, que empaqueta los .bin de todos los programas en efsfiles.c
#   (dentro del kernel).
#
# ⚠ Cuidado (macOS vs Linux): el toolchain de RISC-V se llama distinto en
#   cada plataforma. Cambiá la variable PREFIX si no compila.
# =============================================================================

# ---- Prefijo del cross-compiler ---------------------------------------------
# GNU-Linux:
# PREFIX = riscv64-linux-gnu-
# macOS (con `brew install riscv-gnu-toolchain`):
PREFIX = riscv64-unknown-elf-

CC = $(PREFIX)gcc                  # compilador C cross para RISC-V
OBJDUMP = $(PREFIX)objdump         # extrae .asm legible para debug
OBJCOPY = $(PREFIX)objcopy         # convierte ELF → binario plano

# ---- Flags de compilación ---------------------------------------------------
#  -O                  : optimizaciones básicas (suficiente para código docente).
#  -Wall -Wextra       : todos los warnings.
#  -MD                 : genera archivos .d para dependencias.
#  -mcmodel=medany     : modelo de memoria "medium/any" (adecuado para RV32).
#  -march=rv32ima      : ISA base + M (mul/div) + A (atomics).
#  -mabi=ilp32         : int/long/puntero = 32 bits.
#  -ffreestanding      : no asumir libc.
#  -nostdlib           : no linkear libc.
#  -fno-common         : cada símbolo global en su propia sección.
#  -fno-omit-frame-pointer : mantener frame pointer (mejor stack traces).
#  -fno-stack-protector    : sin canaries (los canaries necesitarían libc).
#  -nostartfiles       : no incluir crt0 default (usamos nuestro `start`).
#  -static             : linkear estáticamente.
#  -march=rv32ima_zicsr_zifencei : Zicsr y Zifencei explícitos (toolchain nuevo).
#  -std=gnu17    : usar C17 (evita que `bool` sea keyword en C23+).
CFLAGS= -O -Wall -Wextra -MD -mcmodel=medany -march=rv32ima_zicsr_zifencei \
        -mabi=ilp32 -ffreestanding -nostdlib -fno-common \
        -fno-omit-frame-pointer -fno-stack-protector -nostartfiles -static \
        -std=gnu17

# Programas a compilar. Agregar más nombres acá cuando haya.
PROGS = init

# ---- Regla por defecto: construir todos y llamar a mkefs.sh -----------------
all: $(PROGS)
	./mkefs.sh $(PROGS) README      # también empaqueta el archivo README.

# ---- init ------------------------------------------------------------------
# Linkea init con edoslib.o (utilidades) y usys.o (stubs de syscalls).
# -Wl,-Tuser.ld : pasa el linker script al ld.
# --build-id=none: sin metadata de build-id (no la necesitamos).
# -e start      : entry point = start.
# Después:
#   objdump -d init > init.asm     (para leer el ensamblador generado)
#   objcopy ... init.bin           (ELF → binario plano que mkefs.sh embebe)
init: init.c edoslib.o usys.o
	$(CC) $(CFLAGS) -Wl,-Tuser.ld,--build-id=none -o init -e start \
	 edoslib.o usys.o init.c
	$(OBJDUMP) -d init > init.asm
	$(OBJCOPY) -S -O binary init init.bin

# ---- objetos auxiliares -----------------------------------------------------
edoslib.o: edoslib.c
	$(CC) $(CFLAGS) -c edoslib.c

usys.o: usys.s
	$(CC) $(CFLAGS) -c usys.s

# ---- limpieza ---------------------------------------------------------------
clean:
	rm $(PROGS) *.o *.bin *.d *.asm
