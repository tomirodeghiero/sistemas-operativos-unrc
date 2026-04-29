# Practico 3

Procesos, threads y concurrencia.

## Resoluciones

- [Resolucion 1 - Ejercicio 1 (ps aux: estados, orden por CPU/mem, procesos del usuario)](./resolucion-1/README.md)
- [Resolucion 2 - Ejercicio 2 (runseconds.sh, fondo, SIGSTOP y SIGCONT)](./resolucion-2/README.md)
- [Resolucion 3 - Ejercicio 3 (pstree, primer proceso del sistema)](./resolucion-3/README.md)
- [Resolucion 4 - Ejercicio 4 (experimento de planificacion con nice/renice/top)](./resolucion-4/README.md)
- [Resolucion 5 - Ejercicio 5 (counter.c, race condition y flock)](./resolucion-5/README.md)
- [Resolucion 6 - Ejercicio 6 (pthreads-example.c, mutex, stacks y address space compartido)](./resolucion-6/README.md)
- [Resolucion 7 - Ejercicio 7 (Java multithreading: synchronized, wait/notify y equivalente al ej 6)](./resolucion-7/README.md)
- [Resolucion 8 - Ejercicio 8 (operaciones del kernel ante interrupcion del timer y fin de quantum)](./resolucion-8/README.md)
- [Resolucion 9 - Ejercicio 9 (syscall read sobre disco: SLEEPING y vuelta por wakeup)](./resolucion-9/README.md)
- [Resolucion 10 - Ejercicio 10 (demostracion semi-formal de Peterson: exclusion mutua y progreso)](./resolucion-10/README.md)
- [Resolucion 11 - Ejercicio 11 (productor-consumidor con semaforos POSIX, threads y procesos)](./resolucion-11/README.md)

## Notas

- Los ejercicios practicos (1 a 6 y 11) asumen un sistema GNU/Linux, tal como lo pide el enunciado (por ejemplo `/proc/<PID>/sched`, `sem_init` no deprecado, `shm_open`).
- En macOS varios comandos existen pero con flags distintos, y `/proc` no esta disponible. Cuando la diferencia es relevante, se aclara en la resolucion correspondiente.
- Los ejercicios 7 a 10 son teoricos. El 7 ademas trae un programa Java de ejemplo en `resolucion-7/CounterExample.java`.
- Las resoluciones 8, 9 y 10 toman como referencia el modelo xv6 sobre RISC-V visto en la teoria, que es la base con la que se discuten traps, scheduling y sincronizacion.
