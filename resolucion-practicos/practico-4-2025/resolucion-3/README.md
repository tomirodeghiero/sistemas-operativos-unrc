# Resolucion 3 - Modificar `rtc.c` para retornar segundos en binario

## Que pide el ejercicio

Modificar el driver `rtc.c` para que el valor de los segundos no se entregue en formato BCD, sino en binario (valor entero normal de `0` a `59`).

## Idea clave

El RTC clasico puede entregar datos en BCD o en binario segun el bit `DM` del registro `RTC_REG_B`.

1. Si `DM = 0`, el dato viene en BCD y hay que convertirlo.
2. Si `DM = 1`, el dato ya esta en binario y no se convierte.

Conversion BCD -> binario:

```c
bin = ((bcd >> 4) * 10) + (bcd & 0x0F);
```

Ejemplo: `0x45` (BCD) -> `45` (binario).

## Cambio de codigo (patch sugerido)

Este patch asume una implementacion tipica del driver que lee segundos desde `RTC_SECONDS` y usa `RTC_REG_B`.

```diff
--- a/rtc.c
+++ b/rtc.c
@@
-static unsigned char rtc_read_seconds(void)
+static unsigned char bcd_to_bin(unsigned char v)
+{
+    return ((v >> 4) * 10) + (v & 0x0F);
+}
+
+static unsigned char rtc_read_seconds(void)
 {
     unsigned char sec;
+    unsigned char regb;
 
     sec = cmos_read(RTC_SECONDS);
-    return sec; /* antes devolvia BCD */
+
+    regb = cmos_read(RTC_REG_B);
+    if (!(regb & 0x04)) {   /* DM=0 => dato en BCD */
+        sec = bcd_to_bin(sec);
+    }
+
+    return sec; /* ahora siempre binario */
 }
```

## Si tu `read()` devolvia texto

Si el driver devolvia los segundos como texto hexadecimal (`%02x`), cambiar a decimal:

```diff
--- a/rtc.c
+++ b/rtc.c
@@
-len = scnprintf(kbuf, sizeof(kbuf), "%02x\n", sec);
+len = scnprintf(kbuf, sizeof(kbuf), "%u\n", sec);
```

Con ese cambio, al leer desde user space se obtiene un valor decimal normal (`0..59`).

## Compilacion y prueba

```bash
make
sudo insmod rtc.ko
dmesg | tail -n 30
cat /dev/rtc_driver
sudo rmmod rtc
```

## Resultado esperado

1. El modulo compila y carga sin errores.
2. El valor de segundos no aparece en BCD.
3. Si antes veias algo como `0x25`, ahora el valor es `25` decimal.

## Nota

En este repositorio no esta el archivo `rtc.c` del enunciado, por eso se deja el patch listo para aplicar sobre el codigo provisto por la catedra.
