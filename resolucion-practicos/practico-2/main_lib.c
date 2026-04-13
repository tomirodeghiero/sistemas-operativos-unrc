/* main_lib.c */
#include <stdio.h>

extern char *hello(void);
extern char *hello2(void);

int main(void)
{
    printf("%s\n", hello());
    printf("%s\n", hello2());
    return 0;
}
