/*=============================================================================
 * efs.c -- Embedded File System: búsqueda por nombre en la tabla embebida
 *=============================================================================
 * Rol dentro de EDOS:
 *   Provee la ÚNICA operación del EFS: buscar un archivo por nombre.
 *   Los archivos en sí están definidos en efsfiles.c (autogenerado por
 *   user/mkefs.sh), con la tabla `efs_files_table[]`.
 *
 * Relación con el resto:
 *   - task.c::load_program() lo usa para localizar el binario "init".
 *   - efsfiles.c define efs_files_table (que declaramos extern acá).
 *
 * Conceptos de SO involucrados:
 *   - Sistema de archivos "de sólo lectura" embebido en la imagen del kernel
 *     (similar a un initramfs en Linux, pero mucho más simple).
 *============================================================================*/

//=============================================================================
// EFS: An embedded (in kernel image) filesystem
//=============================================================================
#include "efs.h"
#include "klib.h"                                       // strcmp

// Definida en efsfiles.c (autogenerada). Está terminada con {0,0,0,0}.
extern struct file efs_files_table[];

/*-----------------------------------------------------------------------------
 * efs_file(file_name):
 *   Búsqueda lineal por nombre. Devuelve puntero al descriptor si lo encuentra
 *   o 0 si no está.
 *
 *   La búsqueda para cuando encuentra un descriptor con name==NULL: es la
 *   marca de "fin de tabla" (última fila = {0,0,0,0}).
 *---------------------------------------------------------------------------*/
struct file* efs_file(char *file_name)
{
    for (int i=0; efs_files_table[i].name; i++) {
        if (strcmp(efs_files_table[i].name, file_name) == 0)
            return efs_files_table + i;                 // Coincidencia.
    }
    return 0;                                           // No está.
}
