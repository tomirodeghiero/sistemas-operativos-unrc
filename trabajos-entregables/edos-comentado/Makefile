# =============================================================================
# Makefile -- Build del kernel EDOS + arranque en QEMU
# -----------------------------------------------------------------------------
# Rol dentro de EDOS:
#   Este Makefile:
#     1) Compila los programas de userland (delegando a user/Makefile).
#     2) Compila y linkea el kernel usando kernel.ld como linker script.
#     3) Genera kernel.asm (útil para debug con GDB).
#     4) Lanza QEMU con el kernel cargado en 0x80000000.
#
# ⚠ Cuidado:
#   - Los .s se referencian por nombre (arch.s, trap.s); son symlinks a los
#     archivos reales bajo arch/riscv32/. NO los borres.
#   - Los flags -ffreestanding -nostdlib son obligatorios: al no haber libc,
#     el kernel provee su propio memset/printf/etc. en klib.c.
# =============================================================================

QEMU=qemu-system-riscv32           # emulador de RISC-V 32.

# ---- Prefijo del cross-compiler ---------------------------------------------
# GNU-Linux:
# PREFIX = riscv64-linux-gnu-
# macOS (brew: riscv-gnu-toolchain):
PREFIX = riscv64-unknown-elf-

CC = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump

# ---- Flags de compilación del kernel ----------------------------------------
#  Igual que en user/Makefile pero:
#    -no-pie -fno-pie : sin PIC (usamos direcciones fijas por kernel.ld).
#    -nostartfiles    : sin crt0 default.
#    -fmax-errors=1   : cortar al primer error (feedback más rápido).
#  -march=rv32ima_zicsr_zifencei : incluye extensión Zicsr (CSRs) y Zifencei
#                                   (fence.i). En toolchains nuevos hay que
#                                   pedirlas explícito.
#  -std=gnu17                     : usar C17 (evita que `bool` sea keyword en C23+).
CFLAGS = -O -Wall -Wextra -MD -mcmodel=medany -march=rv32ima_zicsr_zifencei \
         -mabi=ilp32 -ffreestanding -nostdlib -fno-common \
         -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie \
         -nostartfiles -fmax-errors=1 -std=gnu17

# =============================================================================
# Regla principal: kernel
# -----------------------------------------------------------------------------
# Depende de TODO .s, .c del kernel y de todos los .c/.s de user/. Cuando algo
# cambia se relinkea.
#
# Pasos:
#   1) `make -C user` construye init.bin y actualiza efsfiles.c.
#   2) gcc compila y linkea:
#        -T kernel.ld            usar nuestro linker script.
#        -Wl,--build-id=none     no meter build-id.
#        -Wl,-Map=kernel.map     generar map de linking (útil para debug).
#        -o kernel               binario final.
#      Fuentes: arch.s (arranque), trap.s (ISRs), *.c (todo lo demás).
#   3) `objdump -d` genera kernel.asm (para leer el ensamblador linkeado).
# =============================================================================
kernel: *.s *.c user/*.c user/*.s
	make -C user
	$(CC) $(CFLAGS) -T kernel.ld -Wl,--build-id=none,-Map=kernel.map -o kernel \
	arch.s trap.s *.c
	$(OBJDUMP) -d kernel > kernel.asm

# =============================================================================
# Arrancar QEMU con el kernel
# -----------------------------------------------------------------------------
#   -machine virt   máquina virtio de RISC-V (la única que soportamos).
#   -bios none      NO cargar SBI (arrancamos en M-mode nosotros).
#   -nographic      salida por consola serie (el UART que mapeamos).
#   -smp 1          1 CPU (todavía no manejamos SMP completo).
#   -kernel kernel  carga la imagen en 0x80000000 y hace jump ahí.
#
# ⚠ Para salir de QEMU: Ctrl+A, X.
# =============================================================================
qemu: kernel
	$(QEMU) -machine virt -bios none -nographic  -smp 1 -kernel kernel

# =============================================================================
# Limpieza
# =============================================================================
clean:
	make -C user clean
	rm kernel.map *.asm kernel
