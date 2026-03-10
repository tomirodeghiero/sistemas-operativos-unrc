#!/bin/sh
set -eu

# Une ambos archivos de entrada, ordena y elimina duplicados.
cat correos1.txt correos2.txt | sort | uniq > correos_ordenados_sin_duplicados.txt

# Mensaje de confirmación para el usuario.
echo "Archivo generado: correos_ordenados_sin_duplicados.txt"
