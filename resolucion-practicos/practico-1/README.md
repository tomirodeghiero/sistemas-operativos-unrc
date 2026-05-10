# Práctico 1 — Sistemas Operativos (UNRC, 2026)

**Tema:** Comandos, redirección, pipes, señales y llamadas al sistema.

**Lecturas requeridas (incluidas en esta carpeta):**

1. `notas-del-curso-01,02,03.pdf` — Capítulos 1, 2 y 3 de las Notas del curso.
2. `book-chapter-01.pdf` — Capítulo 1 de *Modern Operating Systems* (Tannenbaum & Bos).

## Marco conceptual

Las resoluciones de este práctico se apoyan en los siguientes conceptos teóricos
discutidos en las lecturas:

- **Sistema operativo:** capa de software que abstrae el hardware, administra
  los recursos (CPU, memoria, dispositivos) y provee servicios a las
  aplicaciones mediante *llamadas al sistema*.
- **Modo kernel vs modo usuario:** la CPU ejecuta el código del SO con
  privilegios totales (modo supervisor); los procesos de usuario corren con
  un conjunto restringido de instrucciones. El cambio se realiza vía *traps*
  (syscalls) o interrupciones.
- **Proceso:** instancia de un programa en ejecución, con su propio espacio de
  memoria (código, datos globales, pila), descriptor en el kernel (PCB) y
  archivos abiertos.
- **Multitarea / multiprogramación:** el SO mantiene varios procesos en
  memoria y multiplexa la CPU mediante *context switches*.
- **API POSIX:** define un conjunto coherente de syscalls (`fork`, `exec`,
  `wait`, `pipe`, `open/read/write/close`, `signal`, `alarm`, ...) que serán
  los pilares de los ejercicios 5 a 12.
- **Shell:** intérprete REPL que permite componer comandos mediante operadores
  de redirección (`<`, `>`, `>>`), secuencia (`;`), condicionales (`&&`,
  `||`), pipes (`|`) y ejecución concurrente (`&`).

Cada resolución indica explícitamente qué concepto teórico justifica la
respuesta.

## Resoluciones

- [Resolución 1 — Ejercicio 1 (incisos a–m): comandos, entorno y argumentos en C](./resolucion-01/README.md)
- [Resolución 2 — Ejercicio 2 (a–c): redirección, secuencia y filtrado](./resolucion-2/README.md)
- [Resolución 3 — Ejercicio 3: pipes (sin archivos temporales)](./resolucion-3/README.md)
- [Resolución 4 — Ejercicio 4: shell scripts y permisos de ejecución](./resolucion-4/README.md)
- [Resolución 5 — Ejercicio 5: programa C con `exit status` controlado](./resolucion-5/README.md)
- [Resolución 6 — Ejercicio 6: implementación propia de `system()` con `fork`/`exec`/`wait`](./resolucion-6/README.md)
- [Resolución 7 — Ejercicio 7: comunicación padre/hijo por `pipe()`](./resolucion-7/README.md)
- [Resolución 8 — Ejercicio 8: comunicación por FIFO (named pipe)](./resolucion-8/README.md)
- [Resolución 9 — Ejercicio 9: mini shell `minish`](./resolucion-9/README.md)
- [Resolución 10 — Ejercicio 10: captura y manejo de `SIGINT`](./resolucion-10/README.md)
- [Resolución 11 — Ejercicio 11: cómputo del mínimo global con `p` hijos y un pipe](./resolucion-11/README.md)
- [Resolución 12 — Ejercicio 12: lectura con `timeout` mediante `alarm()`](./resolucion-12/README.md)

## Documento final en LaTeX

La resolución integral del práctico, redactada en formato académico, se
encuentra en [`practico-01.tex`](./practico-01.tex). El archivo es
auto-contenido y se compila con `pdflatex`.
