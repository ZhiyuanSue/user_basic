#include "stdio.h"
#include "stdlib.h"

/* Simple target for execve testing */
int main(void)
{
        printf("  I am execve_target!\n");
        printf("  execve success!\n");
        TEST_END(__func__);
        return 0;
}