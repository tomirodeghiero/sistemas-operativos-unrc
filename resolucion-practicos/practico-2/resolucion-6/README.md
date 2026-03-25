# Practico 2 - Ejercicio 6

## Datos
Rafagas de CPU:
- P1 = 8
- P2 = 3
- P3 = 5

Suposicion usada: todos llegan en `t=0`.

## 1) FCFS (FIFO)
Orden de ejecucion (segun el orden dado):
- P1 -> P2 -> P3

### Tiempos de espera
- `W(P1) = 0`
- `W(P2) = 8` (espera a que termine P1)
- `W(P3) = 8 + 3 = 11` (espera a P1 y P2)

Promedio:

`W_prom = (0 + 8 + 11) / 3 = 19/3 = 6.33`

## 2) SJF (Shortest Job First)
Orden por rafaga mas corta:
- P2 (3) -> P3 (5) -> P1 (8)

### Tiempos de espera
- `W(P2) = 0`
- `W(P3) = 3`
- `W(P1) = 3 + 5 = 8`

Promedio:

`W_prom = (0 + 3 + 8) / 3 = 11/3 = 3.67`

## Conclusion
- **FCFS:** tiempo de espera promedio = **6.33**
- **SJF:** tiempo de espera promedio = **3.67**

Para estos datos, **SJF mejora** el tiempo de espera promedio respecto de FCFS.
