# Este script está pensado para ejecutarse como: sh procesar_correos_sin_interprete.sh
# (no tiene shebang a propósito por el enunciado del ejercicio).

# Une ambos archivos, ordena y quita repetidos.
cat correos1.txt correos2.txt | sort | uniq > correos_ordenados_sin_duplicados.txt

# Informa el archivo de salida generado.
echo "Archivo generado: correos_ordenados_sin_duplicados.txt"
