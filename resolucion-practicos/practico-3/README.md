# Practico 3

Tareas, concurrencia y planificacion de cpu.

## Resoluciones

- [Resolucion 1 - Ejercicio 1 (ps aux: estados, orden por CPU/mem, procesos del usuario)](./resolucion-1/README.md)
- [Resolucion 2 - Ejercicio 2 (runseconds.sh, fondo, SIGSTOP y SIGCONT)](./resolucion-2/README.md)
- [Resolucion 3 - Ejercicio 3 (pstree, primer proceso del sistema)](./resolucion-3/README.md)
- [Resolucion 4 - Ejercicio 4 (experimento de planificacion con nice/renice/top)](./resolucion-4/README.md)
- [Resolucion 5 - Ejercicio 5 (counter.c, race condition y flock)](./resolucion-5/README.md)
- [Resolucion 6 - Ejercicio 6 (pthreads_example.c, mutex y stacks)](./resolucion-6/README.md)
- [Resolucion 7 - Ejercicio 7 (tiempo de espera promedio: FCFS vs SJF)](./resolucion-7/README.md)
- [Resolucion 8 - Ejercicio 8 (representacion del estado de una tarea no RUNNING)](./resolucion-8/README.md)
- [Resolucion 9 - Ejercicio 9 (flujo de ejecucion ante interrupcion del timer)](./resolucion-9/README.md)
- [Resolucion 10 - Ejercicio 10 (diferencia entre trapframe y contexto salvado)](./resolucion-10/README.md)

## Notas

- Los ejercicios practicos (1 a 6) asumen un sistema GNU/Linux, tal como lo pide el enunciado (por ejemplo `/proc/<PID>/sched`).
- En macOS varios comandos existen pero con flags distintos, y `/proc` no esta disponible. Cuando la diferencia es relevante, se aclara en la resolucion correspondiente.
- Los ejercicios 7 a 10 son teoricos.
