# Resolucion 8 - `mmapfile.c` (adaptado y ejecutado en macOS)

## Contexto

El enunciado original dice "en Linux", pero `mmap`/`munmap`/`MAP_SHARED`/`MAP_PRIVATE`
son interfaces POSIX disponibles tambien en macOS.

En esta carpeta ya estaba el archivo provisto:

- `mmapfile.c` (de la catedra)

y sobre esa base se resolvio:

1. Compilar y ejecutar el original.
2. Explicar por que `data + 7000` no da segmentation fault.
3. Crear version RW + `MAP_SHARED` para persistir cambios al liberar el mapping.
4. Verificar tamano final del archivo.

## Archivos usados

- `mmapfile.c` (original, sin cambios)
- `mmapfile_shared.c` (version modificada para inciso b)
- `mmapfile.txt` (archivo de prueba generado)

## a) Compilar y ejecutar `mmapfile.c` + explicacion de `data + 7000`

### Compilacion y ejecucion

```bash
clang -std=c17 -Wall -Wextra -O0 -g mmapfile.c -o mmapfile
./mmapfile
ls -l mmapfile.txt
```

### Salida obtenida

```text
file contents mapped at 0x100408000
Hello world in page 1
I'm in second page

-rw-r--r--@ 1 tomasrodeghiero  staff  4115 Mar 26 23:17 mmapfile.txt
```

### Por que `data + 7000` no causa segmentation fault

Puntos clave:

1. El mapping se hace con longitud `PGSIZE * 2 = 8192` bytes.
2. El archivo mide `4115` bytes (un poco mas de 4096).
3. La direccion `data + 7000` esta dentro del rango mapeado `[data, data+8192)`.

Entonces la direccion es valida para el VMA del proceso.

Ademas, como el EOF cae dentro de la segunda pagina mapeada:

- bytes de archivo reales en pagina 2: desde 4096 hasta 4114
- resto de esa pagina (hasta 8191): el kernel los expone como ceros para lectura

Por eso no hay segfault en ese acceso.

Nota:

- En muchos sistemas, tocar paginas completamente fuera del objeto mapeado puede terminar en `SIGBUS`.
- En este caso puntual, `7000` cae en la pagina parcialmente respaldada por archivo (la segunda), no en una tercera pagina inexistente.

## b) Version lectura/escritura con persistencia automatica al liberar mapping

Se creo `mmapfile_shared.c` con estos cambios respecto al original:

1. Abrir archivo con `O_RDWR`.
2. Mapear con `PROT_READ | PROT_WRITE`.
3. Usar `MAP_SHARED` (no `MAP_PRIVATE`).
4. Modificar contenido en memoria mapeada.
5. Llamar `munmap` y `close`.
6. Reabrir y verificar que el archivo quedo modificado.

### Comandos

```bash
clang -std=c17 -Wall -Wextra -O0 -g mmapfile_shared.c -o mmapfile_shared
./mmapfile_shared
ls -l mmapfile.txt
```

### Salida obtenida

```text
Antes de modificar:
  page1: Hello world in page 1
  page2: I'm in second page
Despues de munmap + close:
  page1: HELLO from page 1 (shared)
  page2: I'm in second page [MODIFIED]
  tamano archivo: 4115 bytes
-rw-r--r--@ 1 tomasrodeghiero  staff  4115 Mar 26 23:17 mmapfile.txt
```

### Como hace esto el SO

Con `MAP_SHARED`:

1. Las escrituras del proceso van a paginas de memoria asociadas al page cache del archivo.
2. Esas paginas quedan marcadas como "dirty".
3. Al `munmap`/`close` (o por writeback periodico), el kernel sincroniza esas paginas dirty al backing file.

O sea, no hicimos `write()` explicito; el SO propaga cambios desde memoria mapeada al archivo.

## c) Verificar tamano del archivo al finalizar

Resultado medido:

- Tamaño final de `mmapfile.txt`: **4115 bytes**

No cambia porque en la version modificada solo alteramos bytes dentro del rango ya existente del archivo.  
No hicimos `ftruncate` para extenderlo.

## Conclusiones finales

1. El acceso a `data + 7000` no falla porque cae dentro de un mapping valido de 2 paginas.
2. Con `MAP_SHARED` + `PROT_WRITE`, los cambios hechos en memoria quedan persistidos en el archivo al liberar mapping/cerrar.
3. El tamaño del archivo permanece en 4115 bytes si no se lo extiende explicitamente.
