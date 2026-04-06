# Practico 2 - Ejercicio 8

## Enunciado

Implementar semaforos en `edos-steps` y probarlos con tareas que representen productor-consumidor.

## Que implemente

Propuse una API clasica de semaforos:

- `sem_init(sem, value)`
- `sem_wait(sem)` (operacion P/down)
- `sem_signal(sem)` (operacion V/up)

La implementacion usa:

- un `spinlock` para proteger el valor del semaforo,
- una `wait_queue` para bloquear/despertar tareas cuando no hay recursos.

## Archivo de codigo

- `semaforos_productor_consumidor_ej8.c`

El archivo incluye:

1. Implementacion del semaforo para mini-kernel.
2. Demo productor-consumidor con buffer circular.

## Idea del productor-consumidor

Se usan tres semaforos:

- `empty = N` (espacios libres en buffer)
- `full = 0` (items disponibles)
- `mutex = 1` (exclusion mutua sobre el buffer)

### Productor

1. `sem_wait(empty)`
2. `sem_wait(mutex)`
3. inserta item en buffer
4. `sem_signal(mutex)`
5. `sem_signal(full)`

### Consumidor

1. `sem_wait(full)`
2. `sem_wait(mutex)`
3. extrae item del buffer
4. `sem_signal(mutex)`
5. `sem_signal(empty)`

## Que se espera al probar

- No hay race al tocar el buffer (por `mutex`).
- El productor se bloquea si el buffer esta lleno.
- El consumidor se bloquea si el buffer esta vacio.
- Los valores producidos/consumidos quedan consistentes.

## Nota de integracion

Los nombres de primitivas de kernel pueden variar segun tu `edos-steps` (por ejemplo `sleep`, `wakeup`, `yield`, `task_create`, etc.).
Si hace falta, solo hay que mapear esos nombres a los del proyecto real.

## Como compilar y ejecutar

### Opcion A: chequeo rapido del archivo C (sin linkear kernel)
Esto sirve para validar sintaxis/headers del archivo:

```bash
cd resolucion-practicos/practico-2
gcc -std=c11 -Wall -Wextra -c semaforos_productor_consumidor_ej8.c -o semaforos_productor_consumidor_ej8.o
```

Tambien funciona sin `-o`:

```bash
gcc -std=c11 -Wall -Wextra -c semaforos_productor_consumidor_ej8.c
```

Nota: si usas `-o`, **tenes que** poner el nombre del archivo de salida.  
Ejemplo incorrecto (falla): `... -o`

Importante: en esta opcion solo se compila a objeto (`-c`), no se ejecuta, porque las funciones del kernel (`sleep_on`, `task_create`, `kprintf`, etc.) se resuelven al linkear dentro de `edos-steps`.

### Opcion B: integrarlo y ejecutarlo en edos-steps (prueba real)
1. Copiar el archivo al codigo del kernel (o agregarlo al arbol del proyecto).
2. Incluirlo en el build del kernel (Makefile/CMake del proyecto).
3. Llamar `sem_demo_start()` desde la tarea/proceso init del kernel.
4. Compilar y correr `edos-steps` con el flujo normal de tu proyecto.

Ejemplo generico:

```bash
# dentro de tu repo de edos-steps
make
make run
```

Al ejecutar, deberias ver logs tipo:
- `[PROD x] produce ...`
- `[CONS y] consume ...`

Eso confirma que las tareas productor/consumidor estan sincronizadas por semaforos.

## Nota sobre el error "incomplete type struct spinlock"

Si aparece en el editor/compilador un error como:

`incomplete type "struct spinlock" is not allowed`

es porque el archivo esta usando `struct spinlock` y `struct wait_queue` como campos dentro de `struct semaphore`, pero tu entorno no esta viendo sus definiciones concretas.

En `semaforos_productor_consumidor_ej8.c` se encuentran dos caminos:

1. **Compilacion aislada / ejemplo didactico**: usa placeholders de tipos para evitar ese error.
2. **Integracion real con edos-steps**: definir `EDOS_USE_REAL_SYNC_TYPES` e incluir los headers reales del kernel (ajustar paths en el archivo).

La referencia esta al inicio del archivo, en la seccion de tipos de sincronizacion.
