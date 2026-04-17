# Parte 1 – Entrega

En esta parte adjunto dos cosas:

- `kernel.c`, que es el archivo de código fuente que pide la consigna.
- `00-qemu-output.txt`, donde se ve la salida del kernel al ejecutarlo en QEMU.

Al correrlo, la salida que tiene que aparecer es:

```text
Hello World!
```

Para ejecutar el kernel:

1. Abrir `00-hello-world/kernel.c` en el editor.
2. En otra terminal ejecutar:

```bash
cd trabajos-entregables/taller1-riscv/00-hello-world
make qemu PREFIX=riscv64-unknown-elf-
```

3. Se debería ver `Hello World!` en la consola.
4. Para salir de QEMU: `Ctrl-A` y luego `x`.
