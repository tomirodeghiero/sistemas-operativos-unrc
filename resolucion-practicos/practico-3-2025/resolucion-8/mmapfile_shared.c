#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define PGSIZE   4096
#define filename "mmapfile.txt"

int createfile(void)
{
    int f = open(filename, O_RDONLY);

    if (f < 0) {
        const char *str1 = "Hello world in page 1";
        const char *str2 = "I'm in second page";
        const unsigned char zero = 0;
        int l = (int)strlen(str1);
        int i;

        f = open(filename, O_RDWR | O_CREAT, 0666);
        write(f, str1, l);
        for (i = l; i < PGSIZE; i++)
            write(f, &zero, 1);
        write(f, str2, strlen(str2) + 1);
        close(f);
        f = open(filename, O_RDONLY);
    }
    return f;
}

int main(void)
{
    // Garantiza que el archivo exista con el layout original
    int fr = createfile();
    close(fr);

    int f = open(filename, O_RDWR);
    if (f < 0) {
        perror("open");
        return 1;
    }

    char *data = mmap(0, PGSIZE * 2, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(f);
        return 1;
    }

    printf("Antes de modificar:\n");
    printf("  page1: %s\n", data);
    printf("  page2: %s\n", data + PGSIZE);

    // Modificamos contenido dentro del rango real del archivo
    strcpy(data, "HELLO from page 1 (shared)");
    strcpy(data + PGSIZE, "I'm in second page [MODIFIED]");

    // Sin msync: dejamos que el flush ocurra al liberar mapping/cerrar.
    if (munmap(data, PGSIZE * 2) < 0) {
        perror("munmap");
        close(f);
        return 1;
    }
    close(f);

    // Verificacion leyendo archivo nuevamente
    f = open(filename, O_RDONLY);
    if (f < 0) {
        perror("open verify");
        return 1;
    }

    struct stat st;
    if (fstat(f, &st) < 0) {
        perror("fstat");
        close(f);
        return 1;
    }

    data = mmap(0, PGSIZE * 2, PROT_READ, MAP_PRIVATE, f, 0);
    if (data == MAP_FAILED) {
        perror("mmap verify");
        close(f);
        return 1;
    }

    printf("Despues de munmap + close:\n");
    printf("  page1: %s\n", data);
    printf("  page2: %s\n", data + PGSIZE);
    printf("  tamano archivo: %lld bytes\n", (long long)st.st_size);

    munmap(data, PGSIZE * 2);
    close(f);
    return 0;
}
