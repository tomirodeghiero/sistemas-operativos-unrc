/*=============================================================================
 * efs.h -- API del "Embedded File System" (filesystem embebido en la imagen)
 *=============================================================================
 * Rol dentro de EDOS:
 *   EDOS no tiene disco. Los programas de usuario (por ejemplo, `init`) y sus
 *   archivos de datos vienen COMPILADOS DENTRO del binario del kernel como
 *   arrays de bytes. Este módulo permite buscarlos por nombre.
 *
 *   El script user/mkefs.sh genera automáticamente `efsfiles.c` con un array
 *   por cada archivo y la tabla `efs_files_table[]`.
 *
 * Relación con el resto:
 *   - efs.c        implementa la búsqueda (efs_file).
 *   - efsfiles.c   contiene los datos (generado, no editar a mano).
 *   - task.c       lo usa en load_program() para cargar el `init`.
 *
 * Conceptos de SO involucrados:
 *   - Filesystem "read-only" en memoria (tipo initramfs/embedded).
 *   - Cargador de programas (loader): copia bytes del EFS a las páginas del
 *     proceso y las mapea en su espacio virtual.
 *============================================================================*/

// Filesystem embebido (efs)

#pragma once

// Tipos de archivo en el EFS ---------------------------------------------------
#define EFS_FILE_PROGRAM    0        // Binario ejecutable (código + datos).
#define EFS_FILE_DATA       1        // Archivo de datos plano (ej: README).

// Descriptor de un archivo embebido -------------------------------------------
struct file {
    char            *name;    // Nombre del archivo (ej: "init").
    unsigned char   type;     // EFS_FILE_PROGRAM o EFS_FILE_DATA.
    unsigned char   *data;    // Puntero al array de bytes del contenido.
    unsigned int    length;   // Tamaño en bytes.
};

// Busca un archivo por nombre en la tabla efs_files_table[].
// Devuelve puntero al descriptor si existe, o 0 si no.
extern struct file* efs_file(char *file_name);
