#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define PGSIZE   4096
#define filename "mmapfile.txt"

/******************************************************************************
 * This function create a file (or open if it exist) with the following content
 * +--------------------------+ 0
 * | Hello world in page 1    |
 * | padding (zeroes)         |
 * +--------------------------+ 4096
 * | I'm in second page<EOF>  |
 * +--------------------------+
 *
 * So, it will be mapped in memory in two contiguous pages as shown below
 *
 * +--------------------------+ <-- data
 * | Hello world in page 1    |
 * | padding (zeroes)         |
 * +--------------------------+ <-- data + 4096
 * | I'm in second page       |
 * | padding (zeroes)         |
 * +--------------------------+ <-- data + 8192
 */
int createfile(void)
{
    int f = open(filename, O_RDONLY);

    if (f < 0) {
        const char *str1 = "Hello world in page 1";
        const char *str2 = "I'm in second page";
        const unsigned char zero = 0;
        int l = strlen(str1);
        int i;

        f = open(filename, O_RDWR | O_CREAT, 0666);
        write(f, str1, l);
        for (i=l; i<PGSIZE; i++)
            write(f, &zero, 1);
        write(f, str2, strlen(str2)+1);
        close(f);
        f = open(filename, O_RDONLY);
    }
    return f;
}

int main(void)
{
    int f = createfile();
    char *data = mmap(0, PGSIZE * 2, PROT_READ, MAP_PRIVATE, f, 0);
    if (data) {
        printf("file contents mapped at %p\n", data);
        printf("%s\n", data);
        printf("%s\n", data + 4096);

        /* access beyond EOF but less than two pages limit */
        /* It is a valid logical address. Why? */
        printf("%s\n", data + 7000);
        munmap(data, PGSIZE * 2);
    } else {
        printf("mmap error\n");
    }
    close(f);
    return 0;
}
