# Resolucion 4 - Practico 3

Ejercicio 4: experimento de planificacion de CPU con dos procesos CPU-bound de distinta prioridad.

## Archivo fuente

- `busyloop.sh`: script shell con ciclo infinito (`while :; do :; done`). Consume 100% de CPU.

Dar permisos:

```bash
chmod +x busyloop.sh
```

## 4.1 Instancia normal + observacion con top

Terminal 1:

```bash
./busyloop.sh
```

Terminal 2:

```bash
top
# dentro de top: tecla P para ordenar por %CPU, q para salir
```

Se observa `bash busyloop.sh` usando cerca del 100% de un core con valor `NI` (niceness) = `0` y `PR` (prioridad del scheduler) por defecto (tipicamente 20).

## 4.2 Segunda instancia con menor prioridad (nice +15)

Terminal 3:

```bash
nice -n 15 bash ./busyloop.sh
```

Terminal 2 (top):

- Ambos procesos aparecen compitiendo por CPU.
- El nuevo muestra `NI=15` y una prioridad `PR` mas alta numericamente (menos prioridad).
- Si hay un solo core libre, el proceso con nice 0 consume la mayor parte del tiempo de CPU. El de nice 15 recibe mucho menos.

### Cambios de contexto

```bash
cat /proc/<PID>/sched | grep nr_switches
```

`nr_switches` cuenta los context switches totales desde que el proceso existe (voluntarios + involuntarios).

Pregunta: **el proceso con menor prioridad (nice 15) deberia ejecutar mas o menos cambios de contexto?**

Respuesta: **mas**. Con el scheduler CFS de Linux (Completely Fair Scheduler), el proceso con mayor nice recibe fracciones de CPU mas cortas y es desalojado mas frecuentemente a favor del de mayor prioridad. Como el desalojo forzado del scheduler es un context switch involuntario, el contador `nr_switches` crece mas rapido en el proceso de baja prioridad.

Monitoreo en vivo (actualiza cada 1s):

```bash
watch -n1 "cat /proc/<PID_NICE15>/sched | grep nr_switches"
```

Se puede comparar con el nice=0 abriendo otro watch en paralelo.

## 4.3 Cambiar prioridad en ejecucion con renice

```bash
renice -n 5 -p <PID>       # sube el nice (baja la prioridad)
renice -n -5 -p <PID>      # baja el nice (sube la prioridad, requiere root)
```

Verificar con `top` o:

```bash
ps -o pid,ni,pri,comm -p <PID>
```

## 4.4 Correr con mayor prioridad (nice -10)

Los nice negativos reducen solo root puede asignarlos:

```bash
sudo nice -n -10 bash ./busyloop.sh
```

Rango de niceness en Linux: `-20` (mayor prioridad) a `+19` (menor prioridad). El enunciado menciona el rango `-19 .. 20` que es equivalente pero la implementacion real es `-20 .. 19` (ver `man setpriority`).

Observacion esperada en `top`:

- El proceso con `NI=-10` captura casi todo el tiempo de CPU frente a otros con nice 0.
- Los otros procesos CPU-bound aparecen esencialmente pausados en intervalos cortos.

## Limpieza

Terminar todas las instancias:

```bash
pkill -f busyloop.sh
```

## Nota macOS

`/proc` no existe en macOS. Equivalencias:

- Prioridad: `nice`/`renice` funcionan igual.
- Context switches por proceso: no hay interfaz directa. Se puede inspeccionar con `dtrace` (requiere SIP desactivado) o Instruments.app.

Este experimento esta pensado para GNU/Linux.
