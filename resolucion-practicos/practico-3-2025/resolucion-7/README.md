# Resolucion 7 - Espacios de memoria (incluyendo swap) usados por los procesos

## Objetivo del ejercicio

Determinar los espacios de memoria que estan usando los procesos del sistema, incluyendo swap.

El enunciado sugiere comandos de Linux (`top`, `free`, `smem`, `swapon`), pero en este caso el sistema es **macOS**.  
Entonces se usan equivalentes:

- `top` (macOS) -> estado general de CPU/RAM/VM por snapshot
- `vm_stat` -> contadores de memoria virtual por paginas
- `memory_pressure` -> estado global de presion de memoria
- `sysctl vm.swapusage` -> swap total/usado/libre
- `ps` -> consumo por proceso (RSS/VSZ)

## Entorno medido

- Fecha de medicion: `2026-03-26 23:08:17 -03`
- SO: `Darwin 25.3.0 (arm64)`
- RAM fisica (`hw.memsize`): `8589934592 bytes` = **8 GiB**

## 1) Memoria fisica y swap global

### Comando

```bash
sysctl vm.swapusage
```

### Resultado

- Swap total: `2048.00M`
- Swap usado: `927.94M`
- Swap libre: `1120.06M`

Interpretacion:

- Hay uso real de swap (casi 1 GiB), por lo que parte del working set historico de procesos fue desplazado fuera de RAM.

## 2) Snapshot general de memoria del sistema

### Comando

```bash
top -l 1 -n 20
```

### Fragmento relevante del resultado

- `PhysMem: 7472M used (1399M wired, 3476M compressor), 160M unused.`
- `VM: ... 56073 swapins, 151844 swapouts.`

Interpretacion rapida:

- RAM usada: **7472M**
- RAM sin usar inmediata: **160M**
- `wired` (no pageable): **1399M**
- `compressor`: **3476M** (macOS comprimio memoria para evitar mas swapping)
- Existe actividad de swap I/O (`swapins/swapouts`), consistente con el uso de swap reportado.

## 3) Contadores de memoria virtual (detalle por paginas)

### Comando

```bash
vm_stat
```

### Datos observados

- Page size: `16384 bytes` (16KB en esta maquina)
- `Pages active`: `101164`
- `Pages inactive`: `97108`
- `Pages wired down`: `79420`
- `Pages occupied by compressor`: `203786`
- `Swapins`: `53961`
- `Swapouts`: `137396`
- `Pageins`: `1645025`
- `Pageouts`: `37316`

Interpretacion:

- Hay una fraccion importante de memoria en estado comprimido.
- Hubo swapping significativo (`swapouts` bastante mayor que `swapins`), lo que sugiere presion de memoria previa/sostenida.

## 4) Presion de memoria

### Comando

```bash
memory_pressure
```

### Resultado clave

- `System-wide memory free percentage: 41%`
- swapins/swapouts tambien reportados por esta herramienta.

Interpretacion:

- Aunque `top` muestra poca RAM "unused", macOS mantiene disponibilidad por compresion + cache + pages reclaimables, por eso este porcentaje puede parecer alto.

## 5) Procesos que mas memoria residente consumen (RSS)

### Comando usado

```bash
ps -axo pid,rss,comm | sort -k2 -nr | head -n 15
```

### Top observado (RSS en KB, aprox)

1. PID `3517` - VS Code Helper (Renderer): `392416 KB` (~383 MB)
2. PID `18570` - VS Code Helper (Renderer): `256128 KB` (~250 MB)
3. PID `963` - Google Chrome (main): `196576 KB` (~192 MB)
4. PID `1503` - VS Code Helper (Renderer): `192240 KB` (~188 MB)
5. PID `18581` - VS Code Helper (Plugin): `171744 KB` (~168 MB)
6. PID `44506` - Chrome Helper (Renderer): `161200 KB` (~157 MB)
7. PID `43424` - Chrome Helper (Renderer): `146944 KB` (~143 MB)
8. PID `43671` - Chrome Helper (Renderer): `146880 KB` (~143 MB)

Patron claro:

- Los mayores consumidores residentes en este snapshot son procesos de **VS Code** y **Google Chrome** (varios helpers/renderer por arquitectura multiproceso).

## 6) Equivalencia con comandos sugeridos por la catedra (Linux vs macOS)

- `top` -> existe en ambos (sintaxis distinta)
- `free` (Linux) -> en macOS se reemplaza por `vm_stat` + `top` + `memory_pressure`
- `smem` (Linux) -> en macOS usar `ps`/Activity Monitor (`smem` no viene instalado)
- `swapon` (Linux) -> en macOS usar `sysctl vm.swapusage`

Comprobacion en esta maquina:

- `free`: no instalado
- `smem`: no instalado
- `swapon`: no instalado

## 7) Conclusion final del ejercicio

En este sistema macOS (8 GiB RAM):

1. La RAM esta fuertemente utilizada (top reporta ~7.3 GiB usados).
2. Hay compresion alta de memoria (varios GiB en compressor).
3. Hay swap en uso real (~928 MB).
4. Los procesos que mas memoria residente consumen pertenecen principalmente a VS Code y Chrome.

Por lo tanto, el analisis de espacios de memoria del sistema (RAM activa/inactiva/wired/comprimida + swap) queda completamente determinado con los comandos usados.
