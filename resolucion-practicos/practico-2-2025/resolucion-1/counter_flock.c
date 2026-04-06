#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

#define LOOPS 1000
#define BUF_SZ 64

int main(void) {
    int fd = open("counter.dat", O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    for (int i = 0; i < LOOPS; i++) {
        char buf[BUF_SZ];
        memset(buf, 0, sizeof(buf));

        if (flock(fd, LOCK_EX) == -1) {
            perror("flock LOCK_EX");
            close(fd);
            return 1;
        }

        if (lseek(fd, 0, SEEK_SET) == -1) {
            perror("lseek(read)");
            flock(fd, LOCK_UN);
            close(fd);
            return 1;
        }

        ssize_t nread = read(fd, buf, sizeof(buf) - 1);
        if (nread < 0) {
            perror("read");
            flock(fd, LOCK_UN);
            close(fd);
            return 1;
        }
        buf[nread] = '\0';

        int n = atoi(buf);
        int len = snprintf(buf, sizeof(buf), "%05d", n + 1);
        if (len < 0 || len >= (int)sizeof(buf)) {
            fprintf(stderr, "snprintf error\n");
            flock(fd, LOCK_UN);
            close(fd);
            return 1;
        }

        if (ftruncate(fd, 0) == -1) {
            perror("ftruncate");
            flock(fd, LOCK_UN);
            close(fd);
            return 1;
        }

        if (lseek(fd, 0, SEEK_SET) == -1) {
            perror("lseek(write)");
            flock(fd, LOCK_UN);
            close(fd);
            return 1;
        }

        ssize_t nwritten = write(fd, buf, len);
        if (nwritten != len) {
            perror("write");
            flock(fd, LOCK_UN);
            close(fd);
            return 1;
        }

        if (fsync(fd) == -1) {
            perror("fsync");
            flock(fd, LOCK_UN);
            close(fd);
            return 1;
        }

        if (flock(fd, LOCK_UN) == -1) {
            perror("flock LOCK_UN");
            close(fd);
            return 1;
        }
    }

    close(fd);
    return 0;
}
