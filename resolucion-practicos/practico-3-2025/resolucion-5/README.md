# Resolucion 5 - Memoria compartida de 2 paginas (4KB)

## Datos del ejercicio

- Tamano de pagina: `4KB = 0x1000`
- Region compartida: `2 paginas = 8KB`
- Proceso 1 mapea desde `0xA000`
- Proceso 2 mapea desde `0xF000`

Voy a nombrar los marcos fisicos compartidos como:

- `S0` para la primera pagina compartida
- `S1` para la segunda pagina compartida

Lo importante es que **ambos procesos apunten a los mismos marcos fisicos `S0` y `S1`**.

## 1) Calculo de paginas virtuales involucradas

Como cada pagina es de `0x1000`, el numero de pagina virtual (VPN) es:

`VPN = VA / 0x1000` (equivale a correr 12 bits a derecha)

### Proceso 1

- Inicio: `0xA000` -> `VPN = 0xA`
- Segunda pagina: `0xB000` -> `VPN = 0xB`

Rango compartido en P1:

- `0xA000 - 0xAFFF` (primera pagina)
- `0xB000 - 0xBFFF` (segunda pagina)

### Proceso 2

- Inicio: `0xF000` -> `VPN = 0xF`
- Segunda pagina: `0x10000` -> `VPN = 0x10`

Rango compartido en P2:

- `0xF000 - 0xFFFF` (primera pagina)
- `0x10000 - 0x10FFF` (segunda pagina)

## 2) Layout virtual de cada proceso (diagrama)

### Proceso 1 (vista parcial)

```text
VA bajas                                                VA altas
... | 0x9000-0x9FFF | 0xA000-0xAFFF | 0xB000-0xBFFF | 0xC000-... |
... |     otro      |   SHARED p0   |   SHARED p1   |    otro     |
                    |-> marco S0    |-> marco S1    |
```

### Proceso 2 (vista parcial)

```text
VA bajas                                                     VA altas
... | 0xE000-0xEFFF | 0xF000-0xFFFF | 0x10000-0x10FFF | 0x11000-... |
... |     otro      |   SHARED p0   |    SHARED p1    |    otro      |
                    |-> marco S0    |-> marco S1      |
```

Observacion clave: la region compartida cae en diferentes direcciones virtuales en cada proceso, pero ambas traducen a los mismos marcos fisicos.

## 3) Tablas de paginas (solo entradas relevantes)

### Tabla de paginas del Proceso 1

```text
VPN    -> PFN    Flags
0xA    -> S0     P=1, RW=1, US=1, SH=1
0xB    -> S1     P=1, RW=1, US=1, SH=1
...    -> ...    ...
```

### Tabla de paginas del Proceso 2

```text
VPN    -> PFN    Flags
0xF    -> S0     P=1, RW=1, US=1, SH=1
0x10   -> S1     P=1, RW=1, US=1, SH=1
...    -> ...    ...
```

(`SH=1` lo uso como etiqueta conceptual de "compartida"; en hardware real depende de politica/MMU/SO.)

## 4) Diagrama de mapeo conjunto (VA -> PA)

```text
Proceso 1                           Memoria fisica compartida
VA 0xA000-0xAFFF  ---------------->  marco S0 (4KB)
VA 0xB000-0xBFFF  ---------------->  marco S1 (4KB)

Proceso 2
VA 0xF000-0xFFFF  ---------------->  marco S0 (4KB)
VA 0x10000-0x10FFF --------------->  marco S1 (4KB)
```

## 5) Traduccion de direcciones (regla)

Si el desplazamiento dentro de pagina es `d` (`0 <= d < 0x1000`):

- En P1:
  - `VA = 0xA000 + d` -> `PA = base(S0) + d`
  - `VA = 0xB000 + d` -> `PA = base(S1) + d`
- En P2:
  - `VA = 0xF000 + d` -> `PA = base(S0) + d`
  - `VA = 0x10000 + d` -> `PA = base(S1) + d`

Eso prueba que escribiendo en la zona compartida de uno, el otro proceso ve los mismos datos.

## Resumen final

- Region compartida total: 2 paginas (8KB)
- P1 usa `VPN 0xA` y `0xB`
- P2 usa `VPN 0xF` y `0x10`
- Ambas tablas apuntan a los mismos marcos fisicos `S0` y `S1`
- Mismo contenido fisico, distinta ubicacion virtual en cada proceso.
