# Resolucion 2 - Direcciones logicas de codigo, datos, heap y stack (macOS)

El ejercicio pide un programa de usuario que permita analizar en que direcciones
virtuales (logicas) caen las zonas de memoria del proceso.

En esta resolucion, el programa imprime direcciones de:

- codigo (`main`, `code_marker`)
- datos inicializados (`g_init`, `g_static`, `g_array`)
- BSS (`g_bss`)
- constantes/solo lectura (`g_const`, literales)
- heap (`malloc`)
- stack (variable local automatica)

## Archivo fuente

- `mapa_memoria_macos.c`

## Compilacion (Unix en macOS)

```bash
clang -std=c17 -Wall -Wextra -O0 -g mapa_memoria_macos.c -o mapa_memoria_macos
```

## Ejecucion

```bash
./mapa_memoria_macos
```

Opcional, para inspeccion con `vmmap`:

```bash
./mapa_memoria_macos --pause
vmmap <PID_QUE_IMPRIME_EL_PROGRAMA>
```

## Que deberias observar

- Las funciones (codigo) suelen quedar juntas en una zona baja/intermedia.
- Datos/BSS/constantes aparecen en regiones cercanas pero separadas por segmento.
- El heap (`malloc`) cae en una region distinta a datos globales.
- La stack suele estar en direcciones mas altas.
- Las direcciones cambian en cada corrida por ASLR.
