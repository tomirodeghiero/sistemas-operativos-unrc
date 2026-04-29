# Resolucion 6 - Practico 4

Ejercicio 6: en un sistema **SMP (Symmetric Multi-Processing)** el scheduler debe lograr al mismo tiempo *processor affinity* y *load balancing*. Explicar por que estas propiedades se contraponen.

## Conceptos teoricos

De las notas del curso (capitulo *Planificacion de uso de CPU*, seccion *Multiprocesadores*):

- En SMP todas las CPUs ejecutan tanto codigo de usuario como del kernel. La cola READY puede ser compartida o particionada por CPU.
- **Load balancing**: el scheduler busca **emparejar la carga de trabajo entre todas las cpus**. Para eso necesita "contabilizar la carga y asignar a las cpus menos cargadas". En cada context switch un proceso puede *migrar de cpu*, es decir, continuar su ejecucion en otra cpu distinta de la que lo venia ejecutando.
- **Processor affinity**: en cambio, el scheduler intenta **minimizar la migracion de procesos entre cpus**. Linux incluso ofrece la syscall `sched_setaffinity` (*hard affinity*) para que un proceso quede ligado a un subconjunto de cpus.

La teoria explicita el motivo: la migracion **afecta al rendimiento porque la memoria cache de la cpu anterior queda invalidada y la cache de la cpu nueva tiene que volver a llenarse**. Mientras esa cache se "calienta" se acumulan *cache miss* y el proceso ejecuta mas lento.

## Por que se contraponen

Las dos propiedades empujan al scheduler en direcciones opuestas.

1. **Load balancing pide migrar**: si una cpu esta sobrecargada (cola READY larga) y otra esta libre, lo *correcto* desde el punto de vista del balance es mover algun proceso `RUNNABLE` de la primera a la segunda. Sin esa migracion, una cpu queda saturada mientras la otra desperdicia ciclos en idle, y el throughput global cae.

2. **Processor affinity pide no migrar**: cada vez que un proceso cambia de cpu se descarta el trabajo de calentamiento de cache (linea por linea, especialmente L1 y L2 que son privadas por core en la mayoria de las arquitecturas modernas). El proceso paga los proximos miles de instrucciones con un *cache miss rate* mucho mayor, hasta que la nueva cache se vuelva a poblar. Tambien se pierde la "huella" en la TLB del core anterior. En consecuencia, *no migrar* preserva la inversion en cache y hace que el proceso ejecute a la maxima velocidad sostenida.

3. **El conflicto en el caso concreto**:

   - Si el scheduler **prioriza affinity**, deja de migrar incluso cuando una cpu esta sobrecargada. La cola crece, los procesos esperan, y aparece **subutilizacion**: una cpu esta al 100% mientras otra esta al 0%, es decir, un sistema de N cpus rinde efectivamente como uno con menos.
   - Si el scheduler **prioriza load balancing**, migra procesos seguido para mantener parejas las cpus. Pero cada migracion paga el costo de cache (y de TLB, y eventualmente de pagetables si la NUMA topology lo amerita). Si las migraciones son frecuentes el sistema gasta tiempo en repoblar caches en lugar de hacer trabajo util. En el limite, los procesos pueden terminar con menor throughput que antes del balanceo, o sea que el "balance" mejoro la metrica de carga pero empeoro el rendimiento real.

## Compromiso practico

La teoria del curso lo nombra: "estos dos objetivos se contraponen, por lo que hay que lograr una solucion de compromiso". Las estrategias clasicas son:

- **Cola por cpu (*per-cpu runqueue*)** mas un *kernel thread* dedicado al balanceo (en Linux: `migration` thread y la rutina `load_balance`). El scheduler local respeta la affinity por defecto; cada cierto tiempo el balanceador estima la carga relativa y migra solo si la diferencia entre cpus excede un umbral.
- **Affinity blanda (*soft affinity*)**: el scheduler trata de no migrar pero puede hacerlo si compensa. Es el comportamiento por defecto en Linux.
- **Affinity dura (*hard affinity*)**: con `sched_setaffinity` el proceso queda restringido a un conjunto fijo de cpus y el balanceador no puede sacarlo de ahi. Util cuando el costo de migrar es muy alto (procesos NUMA-sensitive, tiempo real, base de datos pinneada por core).
- **Migracion *push* vs *pull*** (tambien mencionada en bibliografia clasica): una cpu sobrecargada puede empujar trabajo (push) o una cpu ociosa puede tirar trabajo de otra (pull). En general las dos estrategias coexisten.

En todos los casos el principio es el mismo: **migrar solo cuando el desbalance proyectado supera al costo esperado de la migracion**. Esa es la decision que captura el compromiso entre las dos propiedades.

## Notas adicionales

- En arquitecturas **multi-core** modernas la cache L3 suele ser compartida entre cores del mismo *socket*, asi que migrar dentro del mismo socket es bastante mas barato que entre sockets distintos. Los schedulers como CFS de Linux son conscientes de la *scheduling domain* (jerarquia: hilo SMT, core, socket, NUMA node) y prefieren migrar primero entre dominios baratos antes de tocar los caros. Esto le saca filo al conflicto sin eliminarlo.
- En sistemas **NUMA** el costo de migrar entre nodos suma ademas el de los accesos a memoria remota, asi que la affinity es aun mas importante. La regla "buen balance = buen rendimiento" deja de valer si el balance se logra moviendo memoria entre nodos NUMA.
- En sistemas con **hyperthreading** dos hilos logicos comparten el mismo core fisico (mismas L1 y L2). Mover un proceso entre hilos del mismo core es practicamente gratis en cache pero introduce contencion por las unidades de ejecucion del core. Es otra dimension del mismo compromiso.

En una linea: **affinity protege la cache de cada proceso individual; load balancing protege la utilizacion global del multiprocesador**. Las dos son deseables, pero cada migracion gana balance al costo de afinidad y viceversa, asi que el scheduler tiene que decidir cuanto vale cada migracion antes de hacerla.
