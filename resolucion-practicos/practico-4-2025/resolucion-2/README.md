# Resolucion 2 - Compilar y experimentar con el driver Linux RTC

Para este ejercicio asumo que el driver dado corresponde a un modulo de kernel Linux (LKM) llamado `rtc_driver.c`, con su `Makefile` de Kbuild.

## Objetivo

1. Compilar el modulo.
2. Cargarlo en kernel.
3. Verificar su inicializacion y funcionamiento.
4. Descargarlo y validar limpieza de recursos.

## Estructura minima esperada

```text
rtc_driver/
  rtc_driver.c
  Makefile
```

## Makefile (Kbuild)

```make
obj-m += rtc_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

## Compilacion

```bash
cd rtc_driver
make
```

Resultado esperado:

1. Se genera `rtc_driver.ko`.
2. No hay errores de compilacion.
3. Puede haber warnings menores segun version de kernel, pero no debe fallar el enlace.

## Carga del modulo y verificacion

```bash
sudo insmod rtc_driver.ko
lsmod | grep rtc_driver
dmesg | tail -n 30
```

Validacion esperada:

1. `lsmod` muestra el modulo cargado.
2. En `dmesg` aparece el mensaje de init del driver (por ejemplo, que registro el dispositivo o que obtuvo la hora RTC).

## Experimentos recomendados

1. Cargar y descargar multiples veces para verificar estabilidad:

```bash
for i in 1 2 3; do
  sudo insmod rtc_driver.ko
  dmesg | tail -n 5
  sudo rmmod rtc_driver
  dmesg | tail -n 5
done
```

2. Si el modulo crea un dispositivo de caracteres (ejemplo `/dev/rtc_driver`), probar lectura:

```bash
ls -l /dev | grep rtc
sudo cat /dev/rtc_driver
```

3. Si expone parametros de modulo, probar valores distintos:

```bash
sudo insmod rtc_driver.ko debug=1
dmesg | tail -n 30
sudo rmmod rtc_driver
```

## Descarga y limpieza

```bash
sudo rmmod rtc_driver
lsmod | grep rtc_driver
dmesg | tail -n 30
make clean
```

Resultado esperado:

1. El modulo deja de aparecer en `lsmod`.
2. En `dmesg` aparece el mensaje de salida (`module_exit`).
3. No quedan archivos temporales de compilacion luego de `make clean`.

## Analisis tecnico breve

Este ejercicio permite verificar el ciclo completo de un driver LKM:

1. Compilacion fuera del arbol del kernel.
2. Carga dinamica con `insmod` y ejecucion de `module_init`.
3. Interaccion con el subsistema RTC o dispositivo expuesto por el modulo.
4. Descarga con `rmmod` y ejecucion de `module_exit`.

Ademas, confirma conceptos clave de E/S vistos en teoria: separacion entre espacio de usuario y kernel, registro de interfaces del driver y depuracion via `dmesg`.

## Nota de entorno

Este procedimiento debe ejecutarse en GNU/Linux con headers del kernel instalados (`/lib/modules/$(uname -r)/build`). En este entorno de trabajo no se puede cargar modulos Linux porque no estamos corriendo un kernel Linux de laboratorio.
