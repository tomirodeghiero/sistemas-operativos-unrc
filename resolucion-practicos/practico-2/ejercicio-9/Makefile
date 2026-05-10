# Practico 2 - Ejercicio 9
# Makefile para generar myprog a partir de main.c y hello.c (originales)
# que viven en el directorio del practico (un nivel arriba).
#
# Uso (desde este directorio):
#   make           -> construye myprog
#   make clean     -> elimina objetos y ejecutable

# VPATH le dice a make donde buscar los archivos fuente cuando no estan
# en el directorio actual.
VPATH = ..

# Variables del compilador
CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic

# El objetivo por defecto es el primero declarado: myprog.
myprog: main.o hello.o
	$(CC) $(CFLAGS) -o $@ $^

# Reglas explicitas para los objetos (didactico, equivale a las
# reglas implicitas de make).
main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

hello.o: hello.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f main.o hello.o myprog
