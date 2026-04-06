# Practico 2 (2026) - Ejercicio 5

Analizar el codigo assembly de `myprog`.

## Herramientas usadas

```bash
nm -n myprog
otool -tvV myprog
otool -Iv myprog
nm -m main.o
otool -rvV main.o
```

## a) Direcciones de `main()` y `hello()`

Comando:

```bash
nm -n myprog | rg ' _main$| _hello$'
```

Resultado en esta compilacion:

- `_main`: `0x0000000100000460`
- `_hello`: `0x00000001000004a0`

## b) Si la invocacion a `hello()` desde `main()` fue resuelta por el linker

Si.

- En `main.o`, `_hello` aparece externo sin resolver.
- En `myprog`, `main` ya tiene salto directo `bl _hello`.

Conclusion: el linker resolvio `main -> hello` en link-time.

## c) Como se resolvio la invocacion a `printf()`

En `myprog`, `main` llama a un `symbol stub`:

```asm
bl 0x1000004ac ; symbol stub for: _printf
```

Entradas relevantes:

- `__TEXT,__stubs`: `0x00000001000004ac -> _printf`
- `__DATA_CONST,__got`: `0x0000000100004000 -> _printf`

Conclusion: el linker resuelve `main -> stub`, y `dyld` resuelve la direccion real de `printf` en runtime.
