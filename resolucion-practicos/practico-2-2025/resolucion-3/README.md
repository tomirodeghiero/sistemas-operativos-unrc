# Practico 2 - Ejercicio 3

## Enunciado
Proponer una implementacion de `fork()` para tareas en `edos-steps` (mini-kernel). En particular, explicar como se logra que en el padre retorne el `pid` del hijo y en el hijo retorne `0`.

## Archivo de codigo C
Se incluye una implementacion propuesta en:

- `fork_syscall_ej3.c`

## Idea general
`fork()` crea un proceso hijo que es una copia del padre:

- mismo codigo y datos de usuario (en su propio espacio de memoria),
- mismo contexto de ejecucion en el punto de retorno del syscall,
- distinto `pid`.

Luego, ambos vuelven a modo usuario en la misma instruccion siguiente al `fork()`, pero con distinto valor de retorno:

- padre: `fork()` devuelve `pid_hijo`
- hijo: `fork()` devuelve `0`

## Estructuras asumidas (tipo edos/xv6)
Se asume algo equivalente a:

- `struct task` / `task_struct` con `pid`, `state`, `parent`, `vm`, `kstack`, `trapframe`, `context`.
- tabla de tareas global.
- estados al menos: `RUNNABLE`, `RUNNING`, `SLEEPING`.

## Propuesta de implementacion

### 1. Buscar descriptor libre para hijo
Reservar una entrada libre en la tabla de tareas.

- Si no hay lugar: retornar error (`-1`).
- Asignar `pid` nuevo al hijo.

### 2. Copiar metadatos del padre
Inicializar en el hijo:

- `parent = current`
- info de scheduling basica
- copia de recursos segun politica (por ejemplo FDs compartidos o duplicados)

### 3. Duplicar memoria de usuario
Crear nuevo espacio de direcciones para el hijo y copiar la imagen del padre:

- text/data/heap/stack usuario
- tabla de paginas propia del hijo

Si falla, liberar lo reservado y retornar error.

### 4. Copiar trapframe/contexto
Copiar el trapframe del padre al hijo.

Esto es clave: ambos van a "retomar" como si volvieran del mismo syscall.

### 5. Ajustar valores de retorno
Aca esta el punto central del ejercicio:

- en el trapframe del **hijo**: `a0 = 0`
- en el trapframe del **padre**: `a0 = pid_hijo`

En RISC-V, el valor de retorno de syscall va en `a0`. Por eso se modifica ese registro salvado antes del retorno a user mode.

### 6. Marcar hijo como RUNNABLE
Una vez consistente, pasar hijo a `RUNNABLE` para que scheduler lo pueda elegir.

### 7. Retorno de `sys_fork()`
El handler de syscall retorna al padre (en kernel) con exito, y luego al volver a user mode:

- padre ve `a0 = pid_hijo`
- hijo, cuando sea planificado y retorne de trap, ve `a0 = 0`

## Pseudocodigo
```c
int sys_fork(void) {
    struct task *p = current;
    struct task *c = alloc_task();
    if (c == NULL) return -1;

    c->pid = alloc_pid();
    c->parent = p;

    if (vm_copy(&c->vm, &p->vm) < 0) {
        free_task(c);
        return -1;
    }

    // Copia contexto de retorno de syscall
    *(c->trapframe) = *(p->trapframe);

    // Valor de retorno en hijo
    c->trapframe->a0 = 0;

    // Valor de retorno en padre
    p->trapframe->a0 = c->pid;

    // Contexto inicial del hijo para volver por trapret/userret
    init_context_for_user_return(c);

    c->state = RUNNABLE;
    return c->pid;
}
```

## Por que funciona padre=pid_hijo e hijo=0
Porque **no depende de dos `return` en C**, sino de lo que queda cargado en el trapframe de cada tarea antes de hacer el retorno a modo usuario.

`fork()` se ejecuta una vez en kernel, pero prepara dos contextos de retorno distintos:

- contexto del padre con `a0 = pid_hijo`
- contexto del hijo con `a0 = 0`

Cuando cada uno haga `trapret/sret`, ese valor aparece como retorno del `fork()` en user space.

## Nota de robustez
En una implementacion real tambien hay que contemplar:

- rollback completo si falla copiar memoria o recursos,
- sincronizacion sobre tabla de tareas/pid,
- herencia correcta de FDs y referencias,
- politicas de copy-on-write (si existiera optimizacion).

## Conclusion
La implementacion de `fork()` en un mini-kernel se resuelve copiando estado del padre, creando espacio de memoria propio para el hijo y, sobre todo, escribiendo distintos valores en `a0` del trapframe de cada proceso. Eso garantiza el comportamiento clasico: padre recibe `pid` del hijo e hijo recibe `0`.
