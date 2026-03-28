#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int g_init = 123;                  // Segmento de datos inicializados
int g_bss;                         // Segmento BSS (no inicializado)
static int g_static = 77;          // Datos inicializados (static)
const int g_const = 99;            // Constante global (normalmente DATA_CONST/rodata)
char g_array[] = "datos_globales"; // Datos inicializados
static const char g_msg[] = "SO-UNRC"; // Solo lectura

static void code_marker(void) {
  // Funcion vacia para tomar una direccion en la zona de codigo.
}

typedef struct {
  const char *name;
  void *addr;
} region_addr_t;

static int cmp_addr(const void *a, const void *b) {
  const region_addr_t *ra = (const region_addr_t *)a;
  const region_addr_t *rb = (const region_addr_t *)b;
  uintptr_t aa = (uintptr_t)ra->addr;
  uintptr_t bb = (uintptr_t)rb->addr;
  if (aa < bb)
    return -1;
  if (aa > bb)
    return 1;
  return 0;
}

int main(int argc, char **argv) {
  int stack_local = 42;   // Variable automatica (stack)
  static int s_local = 9; // Static local: segmento de datos
  const char *literal = "literal_en_rodata";

  int *heap_int = malloc(sizeof(int));
  char *heap_buf = malloc(128);
  if (heap_int == NULL || heap_buf == NULL) {
    perror("malloc");
    free(heap_int);
    free(heap_buf);
    return 1;
  }
  *heap_int = 7;
  heap_buf[0] = 'A';
  heap_buf[1] = '\0';

  region_addr_t regions[] = {
      {"codigo: main", (void *)&main},
      {"codigo: code_marker", (void *)&code_marker},
      {"datos: g_init", (void *)&g_init},
      {"bss: g_bss", (void *)&g_bss},
      {"datos: g_static", (void *)&g_static},
      {"const/global: g_const", (void *)&g_const},
      {"datos: g_array", (void *)g_array},
      {"rodata: g_msg", (void *)g_msg},
      {"rodata: literal", (void *)literal},
      {"datos: static local s_local", (void *)&s_local},
      {"heap: malloc sizeof(int)", (void *)heap_int},
      {"heap: malloc 128", (void *)heap_buf},
      {"stack: local stack_local", (void *)&stack_local},
  };

  size_t n = sizeof(regions) / sizeof(regions[0]);
  qsort(regions, n, sizeof(regions[0]), cmp_addr);

  printf("PID: %d\n", getpid());
  printf("Direcciones virtuales (logicas) ordenadas de menor a mayor:\n\n");
  for (size_t i = 0; i < n; i++) {
    printf("%-30s -> %p\n", regions[i].name, regions[i].addr);
  }

  printf("\nNotas:\n");
  printf("- ASLR hace que cambien entre ejecuciones.\n");
  printf("- En macOS, la stack suele aparecer en direcciones altas.\n");
  printf("- Texto/codigo, datos, heap y stack quedan en zonas separadas.\n");

  if (argc > 1 && strcmp(argv[1], "--pause") == 0) {
    printf("\nPausando 20s para inspeccionar con vmmap...\n");
    fflush(stdout);
    sleep(20);
  }

  free(heap_int);
  free(heap_buf);
  return 0;
}
