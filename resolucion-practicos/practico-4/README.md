# Practico 4

Planificacion de uso de CPU.

## Resoluciones

- [Resolucion 1 - Ejercicio 1 (FCFS y SJF: tiempo de espera promedio)](./resolucion-1/README.md)
- [Resolucion 2 - Ejercicio 2 (Round Robin con quantum 2 y comparacion con FCFS/SJF)](./resolucion-2/README.md)
- [Resolucion 3 - Ejercicio 3 (planificador con prioridad dinamica priority = recent_cpu/2 + 60)](./resolucion-3/README.md)
- [Resolucion 4 - Ejercicio 4 (planificacion por prioridades estaticas)](./resolucion-4/README.md)
- [Resolucion 5 - Ejercicio 5 (Multilevel Feedback de tres niveles, quantum 3, 2 y 1 ms)](./resolucion-5/README.md)
- [Resolucion 6 - Ejercicio 6 (SMP: por que processor affinity y load balancing se contraponen)](./resolucion-6/README.md)

## Notas

- Los ejercicios son todos teorico-analiticos. Las unidades del enunciado son microsegundos en los ejercicios 1, 2 y 4 y milisegundos en el 5; en lo que sigue se trabaja con los numeros sin escribir la unidad cuando no hay ambiguedad.
- La teoria de referencia es la del capitulo *Planificacion de uso de CPU* de las notas del curso. Se citan las definiciones que correspondan en cada resolucion.
- Para los algoritmos no preemptivos (FCFS, SJF, prioridades) se siguio la presentacion clasica del capitulo: una vez que un proceso obtiene la CPU, no la libera hasta que termina su rafaga. Para Round Robin y MLF se considera preemption por timer, tal como define el quantum en la teoria.
