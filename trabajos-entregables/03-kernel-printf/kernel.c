#include "klib.h"
#include "arch.h"

// boot() call kernel_main() after CPUs initial setup
void kernel_main(void) {
    if (cpuid() == 0) {
        printf("\n\nHello %s\n", "World!");
        printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);
    }
    stop();
}
