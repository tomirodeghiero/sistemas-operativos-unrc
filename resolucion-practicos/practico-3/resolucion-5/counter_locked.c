#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/file.h>

#define N 10

int main()
{
    int pid = getpid(), i;
    int fd = open("counter.dat", O_RDWR);

    (void)pid;

    for (i=0; i<1000; i++) {
        char s[N];
        int n;

        flock(fd, LOCK_EX);

        lseek(fd, 0, SEEK_SET);
        read(fd, s, N);
        n = atoi(s);
        sprintf(s, "%d", n+1);
        lseek(fd, 0, SEEK_SET);
        write(fd, s, strlen(s));
        fsync(fd);

        flock(fd, LOCK_UN);
    }
    close(fd);
    return 0;
}
