# Resolucion 4 - Practico 3

Ejercicio 4: experimento de planificacion de cpu en GNU-Linux con dos procesos cpu-bound de distinta prioridad.

## El programa de carga

Para tener algo que use 100% de cpu sin hacer nada util alcanza con un loop infinito. Lo dejamos en `busyloop.sh`:

```bash
#!/bin/bash
while :; do :; done
```

Permisos de ejecucion:

```bash
chmod +x busyloop.sh
```

## 4.1 Una instancia normal observada con `top`

Lo mas simple: en una terminal lo lanzo, y en otra lo miro con `top`.

Terminal 1:

```bash
./busyloop.sh
```

Terminal 2:

```bash
top
# dentro de top: tecla P ordena por %CPU; q sale
```

Lo que se ve es `bash busyloop.sh` consumiendo cerca del 100% de un core, con `NI` (niceness) en `0` y la prioridad `PR` por defecto (tipicamente 20). Si la maquina es multinucleo, otros procesos pueden seguir corriendo sin notar la carga; si es de un solo core, el resto del sistema se siente lento.

## 4.2 Segunda instancia con menor prioridad (nice +15)

Ahora arranco una segunda copia con menos prioridad:

```bash
nice -n 15 bash ./busyloop.sh
```

En `top` se observan las dos instancias compitiendo. La nueva muestra `NI=15` y un `PR` numericamente mas alto (recordar que en Linux un PR mas alto significa **menos** prioridad). Si solo hay un core disponible, el proceso con `nice=0` se queda con la mayor parte del cpu y el de `nice=15` recibe muchisimo menos tiempo de ejecucion.

### Cambios de contexto

Para mirar los context switches por proceso, Linux expone el archivo `/proc/<PID>/sched`. La linea relevante es `nr_switches`, que cuenta todos los switches (voluntarios + involuntarios) desde que el proceso existe:

```bash
cat /proc/<PID>/sched | grep nr_switches
```

La pregunta del enunciado: **el proceso con menor prioridad (nice 15), corre mas o menos cambios de contexto?** La respuesta es **mas**. La intuicion es la siguiente: el CFS de Linux le asigna a cada proceso un *time slice* proporcional a su peso (que depende del nice). El proceso con nice 15 tiene un peso mucho menor que el de nice 0, por lo que su quantum es mas chico y termina siendo desalojado mas seguido a favor del otro. Como cada desalojo es un context switch involuntario, el contador `nr_switches` del proceso de baja prioridad crece mas rapido por unidad de tiempo.

Para verlo en vivo conviene un `watch`:

```bash
watch -n1 "cat /proc/<PID_NICE15>/sched | grep nr_switches"
```

Y otro `watch` en paralelo apuntando al PID de nice 0 para comparar.

## 4.3 Cambiar prioridad en caliente con `renice`

```bash
renice -n 5 -p <PID>       # sube el nice (baja la prioridad)
renice -n -5 -p <PID>      # baja el nice (sube la prioridad, requiere root)
```

Para confirmar que el cambio se aplico:

```bash
ps -o pid,ni,pri,comm -p <PID>
```

Si justo antes estaba comparando los `nr_switches` de las dos instancias, deberia verse el cambio reflejado en la pendiente del contador despues del `renice`.

## 4.4 Mayor prioridad (nice -10) y por que pide sudo

```bash
sudo nice -n -10 bash ./busyloop.sh
```

Bajar el nice a un valor negativo aumenta la prioridad por encima de la del resto de los procesos del usuario, y por eso requiere root. La razon es de **seguridad y equidad**: si cualquier usuario pudiera mejorarse a si mismo la prioridad, podria desplazar a procesos del sistema y monopolizar la cpu, transformando una limitacion del scheduler en una herramienta de denegacion de servicio. Por eso la convencion historica de UNIX es que solo root puede asignar nice negativos. Lo que si puede hacer un usuario sin privilegios es **subir** el nice de un proceso suyo (cederle prioridad a otros), nunca lo opuesto.

Lo que se observa en `top` con la instancia en nice -10: captura casi todo el tiempo de cpu disponible, mientras los demas procesos cpu-bound aparecen casi pausados.

> Detalle del rango: el enunciado menciona prioridades de `-19` a `20`, que es la convencion clasica. La implementacion real en Linux es `-20` a `+19` (ver `man setpriority`). Es equivalente, solo que corrida un valor.

## Limpieza

Para terminar todas las instancias en una:

```bash
pkill -f busyloop.sh
```

## Sobre macOS

`/proc` no existe en macOS, asi que la parte de `nr_switches` no se puede reproducir tal cual. Como equivalencias:

- `nice` y `renice` funcionan igual.
- Para ver context switches por proceso hay que recurrir a `dtrace` (con SIP desactivado) o a Instruments.app.

El experimento en si esta pensado para GNU/Linux y se hace mas comodo ahi.

## Conexion con la teoria

La teoria (Notas 5-6, *Cambios de contexto*) define el `quantum` o `time slice` como el periodo maximo de uso continuo de la CPU asignado a una tarea. Cuando el timer dispara una IRQ y el handler detecta que se agoto el quantum, llama a `yield()` y produce un *context switch* a otra tarea `RUNNABLE`. Cada uno de esos desalojos es un *cambio de contexto involuntario*, contabilizado en `nr_switches` del archivo `/proc/<PID>/sched`.

El experimento muestra empiricamente la regla central del scheduler de Linux (CFS, *Completely Fair Scheduler*): el quantum asignado a cada tarea es proporcional a su peso, y el peso es una funcion decreciente del valor de `nice`. Concretamente, el peso aproximado es $1024 \cdot 1.25^{-\text{nice}}$, de modo que cada incremento de 1 en el `nice` reduce el peso en un 20\%. Un proceso con `nice=15` tiene un peso aproximadamente 15 veces menor que uno con `nice=0`, y por lo tanto:

- recibe time slices mas pequenos -> es desalojado mas seguido -> `nr_switches` crece mas rapido por unidad de tiempo (paradojicamente, el "menos prioritario" hace MAS context switches);
- pero la cantidad **total** de CPU que recibe es menor, no mayor.

Por eso requiere `sudo` la asignacion de `nice` negativos: el scheduler es un recurso de equidad global, y permitir que un usuario ordinario suba su prioridad por encima de los procesos de sistema rompe la suposicion de que los servicios criticos del kernel (gestion de red, swapping, etc.) van a poder seguir progresando ante carga adversaria. La regla de UNIX --solo root baja el nice; cualquier usuario puede subirlo-- es un caso particular de la politica clasica: *cada usuario puede ceder sus propios recursos pero no apropiarse de los ajenos*.
