# Resolucion 2 - Practico 3

Ejercicio 2: usar el script `runseconds.sh` provisto por la catedra (ciclo busy-waiting) y controlar su estado con señales.

## Archivo fuente

- `runseconds.sh`: ciclo en shell que consume CPU durante 120 segundos usando la variable especial `SECONDS` de bash y el no-op `:`. Al terminar imprime `Done!`.

Contenido:

```bash
#!/bin/bash
# Run for 120 seconds
end=$((SECONDS + 120))

while [ $SECONDS -lt $end ]; do
    : # this command does nothing
done

echo "Done!"
```

Dar permisos de ejecucion:

```bash
chmod +x runseconds.sh
```

Nota: el enunciado menciona 60 segundos pero el script de la catedra fija 120. El comportamiento observable es el mismo; solo cambia la ventana de tiempo para inspeccionar el proceso.

## 2.1 Lanzar como proceso de fondo

```bash
./runseconds.sh &
PID=$!
echo "$PID"
```

El `&` delega la ejecucion al background. `$!` guarda el PID del ultimo proceso lanzado en background. Se lo copia a `PID` para reutilizarlo en los pasos siguientes.

Nota zsh: si pegas `echo "PID=$!"` directamente y zsh te muestra el prompt `dquote>`, es porque la comilla de cierre se perdio en el copy-paste (por ejemplo si el markdown convirtio `"` en comillas tipograficas `"` / `"`). Presiona `"` y Enter para cerrar la cita y abortar el comando, o Ctrl-C para cancelar. La version con variable `PID=$!` de arriba evita ese problema porque no pone `$!` dentro de comillas.

## 2.2 Verificar estado RUNNING

En macOS la columna `cmd` se llama `command`. Comando compatible con Linux y macOS:

```bash
ps -o pid,stat,command -p $PID
```

Salida esperada (columna `STAT` con `R` o `R+`):

```text
  PID STAT CMD
12345 R    bash ./runseconds.sh
```

El signo `+` indica que el proceso pertenece al grupo de foreground de la terminal. Como esta en background suele mostrar solo `R` o `S`.

Como el ciclo evalua `$SECONDS` en cada iteracion (shell builtin, no syscall), el proceso alterna muy rapido entre `R` (computando) y breves microbloqueos. En la practica `ps` lo reporta casi siempre como `R`.

## 2.3 Pasar a STOPPED con kill

Enviar `SIGSTOP` (senal 19 en Linux, no se puede ignorar ni bloquear):

```bash
kill -STOP $PID
ps -o pid,stat,command -p $PID
```

Salida esperada (estado `T`):

```text
  PID STAT COMMAND
12345 T    bash ./runseconds.sh
```

Mientras el proceso esta `T`, la variable interna `SECONDS` **no avanza desde el punto de vista del shell**, por lo que el timeout de 120 s se efectiviza sobre el tiempo corrido, no sobre el real. Al detenerlo 30 s y reanudarlo, va a durar 120 s mas desde ese instante.

## 2.4 Continuar con SIGCONT

```bash
kill -CONT $PID
ps -o pid,stat,command -p $PID
```

El estado vuelve a `R` (o `R+`). `SIGCONT` es la unica senal que reanuda a un proceso detenido con `SIGSTOP`.

## 2.5 Esperar o terminar

Opcion A: esperar que termine por timeout del script (cuando imprime `Done!`):

```bash
wait $PID
```

Opcion B: forzarlo:

```bash
kill $PID        # envia SIGTERM (pide terminar limpio)
# o si no responde:
kill -KILL $PID  # SIGKILL (no se puede ignorar)
```

## Resumen de senales usadas

| Senal     | Numero Linux | Efecto                                  |
|-----------|--------------|-----------------------------------------|
| `SIGSTOP` | 19           | Detiene el proceso (no interceptable)   |
| `SIGCONT` | 18           | Reanuda un proceso detenido             |
| `SIGTERM` | 15           | Pide terminar de forma ordenada         |
| `SIGKILL` | 9            | Termina de inmediato (no interceptable) |
