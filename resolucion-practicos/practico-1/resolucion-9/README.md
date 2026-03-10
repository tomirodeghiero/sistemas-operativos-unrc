# Resolucion 9 - Practico 1

Ejercicio 9: implementar `minish.c`, un mini shell que:

- lee comandos en bucle,
- ejecuta cada comando,
- termina al recibir `Ctrl-D` (EOF),
- reconoce los operadores pedidos en el enunciado.

## Archivo fuente

- `minish.c`

## Operadores soportados

El programa reconoce y ejecuta:

1. `cmd1 & cmd2` (fondo/concurrencia)
2. `cmd < input_file` (redireccion entrada)
3. `cmd > output_file` (redireccion salida)
4. `cmd >> output_file` (append)
5. `cmd1 ; cmd2` (secuencial)
6. `cmd1 || cmd2` (condicional OR)
7. `cmd1 && cmd2` (condicional AND)
8. `cmd1 | cmd2` (pipe)

## Como funciona (resumen)

1. `getline()` lee una linea.
2. Se parsea la linea buscando operadores.
3. Para ejecutar comandos usa `fork()`, `execvp()`, `waitpid()`, `pipe()`, `dup2()`.
4. El bucle finaliza cuando `getline()` devuelve EOF (Ctrl-D).

## Compilacion

```bash
gcc -Wall -Wextra -o minish minish.c
```

## Ejecucion interactiva

```bash
./minish
```

Prompt:

```text
minish$
```

Salir:

- Presionar `Ctrl-D`.

## Ejemplos de uso (uno por cada item)

```bash
echo hola ; echo mundo
sleep 1 & echo "sigo en foreground"
echo texto > /tmp/minish_demo.txt
echo mas >> /tmp/minish_demo.txt
cat < /tmp/minish_demo.txt
false || echo "se ejecuto OR"
true && echo "se ejecuto AND"
echo hola | tr a-z A-Z
```

## Nota

Esta version es un mini shell educativo y no implementa comillas complejas ni expansiones avanzadas de shells reales (bash/zsh). Para el alcance del ejercicio, cumple con los operadores pedidos.
