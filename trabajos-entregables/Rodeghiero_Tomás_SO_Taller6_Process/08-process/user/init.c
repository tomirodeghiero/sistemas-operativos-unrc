#include "edoslib.h"

int main(void)
{
    printf("EDOS init process: pid=%d!\n", getpid());
    console_puts("Init going to sleep for 4 ticks...\n");
    sleep(4);
    console_puts("init awake! Finishing...\n");
    return 0;
}
