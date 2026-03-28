# Resolucion 5 - Pasos de la syscall `fs_open("/docs/mycv.txt")`

## Que pide el ejercicio

Describir, en un sistema basado en inodos, que pasos realiza una syscall del tipo:

```c
inode *fs_open("/docs/mycv.txt")
```

La syscall debe resolver el `full path` y devolver el inodo del archivo objetivo.

## Supuesto de modelo

Asumo un kernel tipo UNIX/xv6 simplificado:

1. Cada proceso tiene su tabla de descriptores.
2. El VFS/FS usa inodos y directorios con entradas `nombre -> inodo`.
3. El path es absoluto porque empieza con `/`.
4. La llamada retorna un puntero/referencia al inodo en memoria (o error).

## Flujo completo de `fs_open("/docs/mycv.txt")`

## 1) Entrada a kernel y validacion inicial

1. El proceso de usuario invoca la syscall (trap al kernel).
2. El kernel copia el string desde espacio de usuario a un buffer seguro de kernel (`copyin`).
3. Verifica:
4. que el puntero no sea invalido.
5. que termine en `\0`.
6. que la longitud no exceda `PATH_MAX`.
7. Si algo falla, retorna error (`-EFAULT`, `-ENAMETOOLONG`, etc.).

## 2) Parseo del path

Con `"/docs/mycv.txt"`:

1. El parser detecta path absoluto por el prefijo `/`.
2. Se ignoran barras repetidas si existen.
3. Componentes resultantes: `["docs", "mycv.txt"]`.

Si el path fuera solo `/`, la respuesta seria el inodo raiz.

## 3) Seleccion del inodo inicial

Como es absoluto:

1. `current = inode_raiz`.
2. Se incrementa su referencia (por ejemplo `iget`/`idup`).
3. Se toma lock del inodo cuando se lo consulta (`ilock`), segun el diseño del FS.

## 4) Resolucion iterativa de componentes

## 4.1 Resolver `docs`

1. Verificar que `current` (raiz) sea directorio.
2. Leer entradas del directorio (sus bloques de datos).
3. Buscar entrada con nombre exacto `"docs"`.
4. Si no existe -> error `-ENOENT`.
5. Si existe -> obtener `inum_docs`.
6. Cargar inodo de `docs` desde tabla de inodos.
7. Liberar referencia/lock del inodo anterior (raiz) cuando corresponde.
8. `current = inode_docs`.

## 4.2 Resolver `mycv.txt`

1. Verificar que `current` (`docs`) sea directorio.
2. Leer entradas y buscar `"mycv.txt"`.
3. Si no existe -> `-ENOENT`.
4. Si existe -> obtener `inum_mycv`.
5. Cargar ese inodo y pasar a ser `current`.

Al finalizar componentes, `current` debe ser el inodo del objeto objetivo.

## 5) Validaciones finales del objeto abierto

1. Verificar permisos de acceso segun credenciales del proceso:
2. permiso de ejecucion/busqueda en directorios recorridos.
3. permiso de lectura/escritura sobre `mycv.txt` segun modo de apertura.
4. Verificar tipo si la syscall espera archivo regular (si fuese obligatorio).
5. Si falla permisos -> `-EACCES`.

## 6) Crear estado de archivo abierto (si aplica)

Aunque el enunciado muestra retorno de `inode*`, en un SO real normalmente se hace:

1. Reservar una entrada en la tabla global de archivos abiertos (`struct file`).
2. Asociar el inodo encontrado.
3. Inicializar offset (`0`), flags (`O_RDONLY`, etc.), contador de referencias.
4. Asignar un descriptor en la tabla del proceso.
5. Retornar `fd`.

En la version pedida por el ejercicio, el retorno conceptual es el `inode*` de `mycv.txt`.

## 7) Manejo de errores y limpieza

En cualquier fallo intermedio:

1. Soltar locks tomados.
2. Decrementar referencias de inodos temporales.
3. Liberar estructuras auxiliares.
4. Retornar codigo de error consistente.

Esto evita fugas de referencias y deadlocks.

## Pseudocodigo compacto

```c
inode *fs_open(const char *upath) {
    char kpath[PATH_MAX];
    char *comp[MAX_DEPTH];
    int n;
    inode *cur, *next;

    if (copy_path_from_user(kpath, upath) < 0) return ERR_PTR(-EFAULT);
    n = split_path(kpath, comp);
    if (n < 0) return ERR_PTR(-EINVAL);

    cur = iget(ROOT_INUM);              // path absoluto
    for (int i = 0; i < n; i++) {
        ilock(cur);
        if (cur->type != T_DIR) { iunlockput(cur); return ERR_PTR(-ENOTDIR); }
        int inum = dir_lookup(cur, comp[i]);   // busca nombre en directorio
        iunlock(cur);
        if (inum < 0) { iput(cur); return ERR_PTR(-ENOENT); }
        next = iget(inum);
        iput(cur);
        cur = next;
    }

    ilock(cur);
    if (!permite_acceso(cur, current_cred())) { iunlockput(cur); return ERR_PTR(-EACCES); }
    iunlock(cur);
    return cur; // inodo de /docs/mycv.txt
}
```

## Trazado concreto para `/docs/mycv.txt`

1. Inicio en inodo raiz (`/`).
2. `lookup("docs")` en `/` -> obtiene inodo de `docs`.
3. `lookup("mycv.txt")` en `docs` -> obtiene inodo de `mycv.txt`.
4. Verifica permisos sobre `mycv.txt`.
5. Retorna referencia al inodo final.

## Complejidad (idea)

Si cada directorio tiene `m` entradas y el path tiene `k` componentes:

1. Busqueda lineal simple: `O(k * m)` en promedio.
2. Con indice por directorio (htree/btree), mejora la busqueda por componente.

## Conclusiones

La syscall no "salta" directo al archivo: recorre el path componente por componente desde raiz, validando tipo de objeto, permisos y existencia en cada paso.  
Solo cuando termina ese recorrido de forma consistente puede devolver el inodo de `mycv.txt`.
