#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

/*
 * Phase 3 execve test (initramfs / VFS)
 *
 * Target ELF: /tests/test_execve_simple (see RendezvOS rootfs/tests/)
 */

void test_execve_basic(void)
{
        TEST_START(__func__);

        printf("  Testing basic execve...\n");

        char *newargv[] = {"test_execve_simple", NULL};
        char *newenviron[] = {NULL};

        execve("/tests/test_execve_simple", newargv, newenviron);

        /* If we reach here, execve failed */
        printf("  ERROR: execve returned unexpectedly!\n");
        TEST_END(__func__);
}

int main(void)
{
        test_execve_basic();
        return 0;
}