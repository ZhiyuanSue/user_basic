#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

/*
 * oscomp execve — loads /tests/test_echo from initramfs.
 * Success stdout:
 *   I am test_echo.
 *   execve success.
 */
void test_execve(void)
{
        TEST_START(__func__);
        char *newargv[] = {"test_echo", NULL};
        char *newenviron[] = {NULL};
        execve("/tests/test_echo", newargv, newenviron);
        printf("  execve error.\n");
        // TEST_END(__func__);
}

int main(void)
{
        test_execve();
        return 0;
}
