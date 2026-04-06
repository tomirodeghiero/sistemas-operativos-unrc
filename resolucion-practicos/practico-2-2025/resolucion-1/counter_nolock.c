#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define N 10

int main()
{
    int pid = getpid(), i;
    int fd = open("counter.dat", O_RDWR);

    for (i=0; i<1000; i++) {
        char s[N];
        int n;

        lseek(fd, 0, SEEK_SET);
        read(fd, s, N);
        n = atoi(s);
        sprintf(s, "%05d", n+1);
        lseek(fd, 0, SEEK_SET);  // rewind
        write(fd, s, strlen(s));
        fsync(fd);
    }
    close(fd);
    return 0;
}
