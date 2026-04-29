# Resolucion 2 - Practico 3

Ejercicio 2: usar el script `runseconds.sh` provisto por la catedra (un ciclo de busy-waiting de 120 segundos) y manipular su estado con senales.

## El script

`runseconds.sh` es un loop en bash que consume cpu durante 120 segundos. Aprovecha la variable especial `SECONDS` (que cuenta el tiempo desde que arranco el shell) y el no-op `:` para girar sin hacer nada util:

```bash
#!/bin/bash
# Run for 120 seconds
end=$((SECONDS + 120))

while [ $SECONDS -lt $end ]; do
    : # this command does nothing
done

echo "Done!"
```

Para que sea ejecutable:

```bash
chmod +x runseconds.sh
```

> Nota al margen: el enunciado habla de 60 segundos pero el script de la catedra fija 120. No cambia la dinamica del ejercicio, solo da una ventana mas amplia para inspeccionar al proceso.

## 2.1 Lanzar como proceso de fondo

```bash
./runseconds.sh &
PID=$!
echo "$PID"
```

El `&` manda el comando al background. La variable `$!` guarda el PID del ultimo proceso lanzado en background, que copio a `PID` para reusarlo despues.

> Tip zsh: si pegas algo como `echo "PID=$!"` y zsh te muestra el prompt `dquote>`, es porque la comilla de cierre se perdio en el copy-paste (algunas veces el markdown convierte `"` en comillas tipograficas `"`/`"`). Se cierra escribiendo `"` y Enter, o se aborta con Ctrl-C. Usar `PID=$!` sin comillas evita ese problema de raiz.

## 2.2 Verificar que esta RUNNING

En macOS la columna `cmd` se llama `command`, asi que conviene pedir el formato explicito para que sea portable:

```bash
ps -o pid,stat,command -p $PID
```

Salida esperada:

```text
  PID STAT CMD
12345 R    bash ./runseconds.sh
```

El estado es `R` (o `R+` cuando el proceso pertenece al grupo de foreground). Como el ciclo evalua `$SECONDS` como builtin del shell, no como syscall, el proceso esta computando casi todo el tiempo y `ps` lo reporta como `R`.

## 2.3 Pasarlo a STOPPED con `kill`

`SIGSTOP` (senal 19 en Linux) detiene al proceso y no puede ser ignorada ni bloqueada:

```bash
kill -STOP $PID
ps -o pid,stat,command -p $PID
```

Ahora el `STAT` cambia a `T`:

```text
  PID STAT COMMAND
12345 T    bash ./runseconds.sh
```

Un detalle interesante: mientras esta en `T`, la variable interna `SECONDS` no avanza desde el punto de vista del shell. Si lo dejo detenido 30 segundos y despues lo reanudo, el script va a durar 120 segundos mas a partir de ese instante, no menos. Es decir, la espera se mide en tiempo de cpu corrido, no en tiempo real.

## 2.4 Continuar la ejecucion

La unica senal que reanuda un proceso detenido por `SIGSTOP` es `SIGCONT`:

```bash
kill -CONT $PID
ps -o pid,stat,command -p $PID
```

El estado vuelve a `R` (o `R+`).

## 2.5 Esperar o terminar

Si quiero esperar a que termine solo (cuando imprime `Done!`), uso:

```bash
wait $PID
```

Si quiero forzarlo, primero le mando `SIGTERM` (terminacion ordenada) y, si no responde, `SIGKILL`:

```bash
kill $PID         # SIGTERM
kill -KILL $PID   # SIGKILL si lo anterior no alcanzo
```

## Recapitulacion de senales usadas

| Senal     | Numero Linux | Efecto                                   |
|-----------|--------------|------------------------------------------|
| `SIGSTOP` | 19           | Detiene el proceso (no se puede ignorar) |
| `SIGCONT` | 18           | Reanuda un proceso detenido              |
| `SIGTERM` | 15           | Pide terminar de forma ordenada          |
| `SIGKILL` | 9            | Termina de inmediato (no se puede ignorar) |

El par `SIGSTOP`/`SIGCONT` es justamente la herramienta que el shell usa internamente cuando uno aprieta Ctrl-Z (`SIGTSTP`) y despues escribe `fg` o `bg` para reactivar el job.
