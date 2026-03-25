# Practico 2 - Ejercicio 7

## Objetivo
Hacer un experimento simple de planificacion de CPU en GNU/Linux usando procesos de uso intensivo de CPU y observar el efecto de `nice`/`renice`.

## Archivo usado
- `infinite.sh` (bucle infinito CPU-bound)

Contenido:

```bash
#!/usr/bin/env bash
for (( ; ; )); do
  :
done
```

## Preparacion
Desde la carpeta del practico:

```bash
cd resolucion-practicos/practico-2
chmod 744 infinite.sh
```

Abrir **dos terminales**:
- Terminal A: lanzar procesos.
- Terminal B: monitorear (`top`/`ps`).

## 1) Ejecutarlo normalmente y observar con `top`

En Terminal A:

```bash
./infinite.sh
```

En Terminal B:

```bash
top
```

Que observar:
- El proceso aparece usando CPU alta (cerca de 100% de un core).
- `NI` (nice) deberia ser `0` (prioridad normal).

## 2) Ejecutar una instancia con menor prioridad (`nice` alto)

En Terminal A (dejar corriendo la anterior y lanzar otra):

```bash
nice -n 15 ./infinite.sh
```

En Terminal B:

```bash
top
```

Que observar:
- Ahora hay dos procesos CPU-bound compitiendo.
- El de `nice=15` tiene menor prioridad que el de `nice=0`.
- En igualdad de condiciones, el de `nice=0` suele recibir mas CPU.

## 3) Cambiar prioridad en caliente con `renice`

Primero obtener PID (Terminal B o A):

```bash
ps -eo pid,ni,pri,comm,args | grep infinite.sh | grep -v grep
```

Subir `nice` (bajar prioridad) de uno que este en `0`:

```bash
renice 19 -p <PID>
```

Volver a mirar en `top`.

Que observar:
- Cambia la columna `NI`.
- El proceso reniceado con valor mas alto (`19`) tiende a recibir menos CPU.

## 4) Proceso con mayor prioridad que la normal (`nice` negativo)

En Linux, para `nice` negativo normalmente hace falta privilegio de administrador.

Ejemplo:

```bash
sudo nice -n -10 ./infinite.sh
```

Que observar:
- `NI=-10` (o menor a 0).
- Ese proceso tiende a recibir mas CPU frente a los de `NI=0` o `NI=15`.

## Comandos utiles de verificacion

```bash
ps -eo pid,ni,pri,stat,comm,args | grep infinite.sh | grep -v grep
ps -axo pid,ni,pcpu,args | grep infinite.sh | grep -v grep
```

- `NI`: niceness (mas alto = menor prioridad).
- `PRI`: prioridad interna que usa scheduler.

## Cierre del experimento

Para terminar todas las instancias:

```bash
pkill -f infinite.sh
```

## Conclusiones
- `nice` **alto** (ej. 15, 19) => menor prioridad de CPU.
- `nice` **normal** (`0`) => prioridad estandar.
- `nice` **negativo** (ej. `-10`) => mayor prioridad (requiere permisos).
- Se ve claramente como el scheduler reparte CPU distinto segun la prioridad.
