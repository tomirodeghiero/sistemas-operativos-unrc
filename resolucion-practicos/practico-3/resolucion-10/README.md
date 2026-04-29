# Resolucion 10 - Practico 3

Ejercicio 10: mostrar de manera semi-formal que la solucion de Peterson, con dos procesos, garantiza **exclusion mutua** y **progreso**.

## El algoritmo

El algoritmo de Peterson (1981) resuelve el problema de la seccion critica para dos procesos `P0` y `P1` usando solo lectura y escritura sobre dos variables compartidas: un arreglo de banderas y una variable `turn` que arbitra empates.

```c
// Variables compartidas, inicializadas en falso / 0
bool flag[2] = { false, false };
int  turn   = 0;          // o 1, da igual

// Codigo que ejecuta el proceso i (j = 1 - i es el otro)
void p_i(void) {
    while (true) {
        flag[i] = true;            // (1) anuncio interes
        turn    = j;               // (2) cedo el turno al otro
        while (flag[j] && turn == j)   // (3) busy wait
            ;
        // ----- seccion critica -----
        flag[i] = false;           // (4) salgo de la SC
        // ----- seccion no critica -----
    }
}
```

Hipotesis del modelo (lo que tomamos como dado en la teoria):

- Las lecturas y escrituras a `flag[i]` y a `turn` son atomicas (cada operacion individual es indivisible).
- Solo `P_i` escribe en `flag[i]`, pero ambos lo leen. `turn` la escriben los dos.
- No se asume orden estricto de ejecucion entre `P0` y `P1`: el scheduler puede intercalar instrucciones arbitrariamente.

## Vocabulario que usamos en la demostracion

- "Estoy en la SC" significa que el contador de programa esta despues del paso (3) y antes del paso (4) (es decir, el `while` de espera ya termino y todavia no se libero la bandera).
- "Esperando" significa que el contador de programa esta dentro del paso (3), girando en busy wait.
- "Quiero entrar" significa que ya ejecute (1) (`flag[i] = true`), pero todavia no termine (3).

## Parte A. Exclusion mutua

> **Afirmacion**: en ningun estado alcanzable del sistema, ambos procesos pueden estar simultaneamente dentro de la seccion critica.

Vamos a demostrarlo por contradiccion.

**Supongamos** que en algun instante `t`, `P0` y `P1` estan ambos en la SC. Entonces, justo antes de entrar:

- `P0` salio de su `while` de espera (3), por lo que la guarda fallo. Es decir:

  $$\text{NOT}(\text{flag}[1] = \text{true} \land \text{turn} = 1) \quad \Rightarrow \quad \text{flag}[1] = \text{false} \;\lor\; \text{turn} = 0 \quad (\star)$$

- Simetricamente, `P1` salio de su `while` de espera, por lo que:

  $$\text{flag}[0] = \text{false} \;\lor\; \text{turn} = 1 \quad (\star\star)$$

Ahora bien, ambos procesos estan **dentro** de la SC, lo que implica que ambos ya ejecutaron sus respectivos pasos (1):

$$\text{flag}[0] = \text{true} \quad \text{y} \quad \text{flag}[1] = \text{true}.$$

(Ninguno limpia su flag hasta el paso (4), que esta despues de la SC. Y solo `P_i` escribe en `flag[i]`.)

Reemplazando en `(*)` y `(**)`:

- De `(*)`: como `flag[1] = true`, la primera disyuncion falla, asi que tiene que ser `turn = 0`.
- De `(**)`: como `flag[0] = true`, la primera disyuncion falla, asi que tiene que ser `turn = 1`.

Por lo tanto `turn = 0` y `turn = 1` simultaneamente. Contradiccion.

> **Conclusion**: no hay un instante en el que ambos procesos esten dentro de la SC. La exclusion mutua se cumple. $\blacksquare$

### Por que el orden (1) -> (2) -> (3) es esencial

Si se invirtieran los pasos (1) y (2) (primero `turn = j`, despues `flag[i] = true`), la prueba se rompe: un proceso podria leer la flag del otro antes de que el otro haya anunciado interes, y ambos podrian colarse simultaneamente. La idea central es: **escribir mi flag primero asegura que, si el otro entra al `while` despues de mi `turn = j`, vea mi flag en `true`**.

## Parte B. Progreso

> **Afirmacion**: si la SC esta libre y al menos un proceso quiere entrar, alguno entrara en tiempo finito. Ademas, el unico proceso que puede impedirle el ingreso a otro es uno que tambien quiere entrar.

Recordatorio del enunciado de progreso (Peterson, Dijkstra): si nadie esta dentro de la SC, la decision de cual entra solo puede involucrar a procesos que estan compitiendo por entrar (no la pueden bloquear procesos en la seccion no critica), y no puede postergarse indefinidamente.

**Caso 1**: `P0` quiere entrar y `P1` esta en la seccion no critica.

`P1` no esta en (1)-(3): por lo tanto `flag[1] = false`. La guarda del `while` de `P0` evalua `flag[1] && turn == 1` -> `false && ...` -> falso. `P0` sale del `while` y entra a la SC. `P1` no influye.

**Caso 2**: ambos procesos quieren entrar (los dos ya ejecutaron (1), ambos `flag` quedaron en `true`).

Vamos a mostrar que **alguno** sale del `while` y entra a la SC. Notar primero que `turn` es atomica y unica: el ultimo proceso que escribio sobre ella define su valor. Sean dos casos por el valor final de `turn` antes de que alguno entre al `while`:

- Si `turn = 1`, entonces la guarda de `P0` es `flag[1] && turn == 1` -> verdadera, gira. La de `P1` es `flag[0] && turn == 0` -> falsa. **`P1` entra**.
- Si `turn = 0`, simetricamente, **`P0` entra**.

En cualquiera de los dos subcasos hay un ganador, y como `turn` ya no se vuelve a tocar mientras quienquiera que tenga el turno este esperando o dentro de la SC, el ganador no es derrotado por nuevas escrituras.

**Caso 3**: el proceso que esta en la SC ya termino su SC.

Cuando un proceso sale de la SC ejecuta (4) (`flag[i] = false`). En el ciclo siguiente, la guarda del `while` del otro ve `flag[i] = false`, falla, y el otro entra. Asi, ningun proceso queda atrapado por uno que ya termino la SC.

> **Conclusion**: si la SC esta libre y hay procesos que quieren entrar, alguno entra; y solo procesos que tambien quieren entrar pueden retrasar la decision (por un tiempo finito). El algoritmo cumple progreso. $\blacksquare$

### Comentario adicional: espera acotada

Un tercer requerimiento del problema de la SC, **espera acotada**, dice que existe un limite en la cantidad de veces que otros procesos pueden entrar a la SC antes que un proceso que ya pidio entrada lo logre. En el caso de dos procesos Peterson la cota es trivial: a lo sumo el otro proceso entra **una sola vez** entre que yo expreso interes y entrar yo. La razon: cuando `P_j` sale de la SC, vuelve a competir y ejecuta nuevamente (2), `turn = i`. A partir de ahi mi `while` ya falla y entro yo. Esto generaliza la prueba de progreso a un argumento de equidad fuerte para 2 procesos.

## Hipotesis criticas y limites

La demostracion descansa en dos supuestos:

1. **Atomicidad** de las lecturas y escrituras a `flag[]` y `turn`. Si la palabra no se escribe atomicamente, podriamos leer un valor "a medio actualizar" y la prueba falla.
2. **Modelo de memoria sequencially consistent**. Si la maquina puede reordenar las escrituras (1) y (2) (lo que pasa en x86 / ARM / RISC-V con buffers de escritura), la prueba se rompe igual: el proceso 1 podria observar `turn = j` antes que `flag[i] = true` y "colarse" en la SC. Por eso, en hardware moderno, **una implementacion real de Peterson requiere barreras de memoria** (`MFENCE` en x86, `fence rw,rw` en RISC-V) entre los pasos (1) y (2). Sin esas barreras, el algoritmo es semi-formalmente correcto pero practicamente roto.

Estos dos supuestos son justamente el motivo por el que en sistemas reales las exclusiones mutuas se construyen sobre instrucciones atomicas de hardware (`test_and_set`, `compare_and_swap`, `LR/SC`) en lugar de sobre Peterson puro.

## En una linea

- **Exclusion mutua**: si los dos estuvieran en la SC, `turn` deberia valer `0` y `1` a la vez. Imposible.
- **Progreso**: cuando los dos compiten, gana el que NO escribio `turn` ultimo; cuando solo compite uno, entra de inmediato porque la flag del otro esta en `false`.
