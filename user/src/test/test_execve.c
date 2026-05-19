#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

/*
 * Phase 3a execve test
 *
 * This test verifies basic execve functionality:
 * - Simple execve with embedded ELF
 * - argv passing
 * - Process replacement
 */

void test_execve_basic(void)
{
        TEST_START(__func__);

        printf("  Testing basic execve...\n");

        /* Prepare argv for test_execve_simple (simple test in test directory) */
        char *newargv[] = {"test_execve_simple", NULL};
        char *newenviron[] = {NULL};

        /* This should replace current process with test_execve_simple */
        execve("test_execve_simple", newargv, newenviron);

        /* If we reach here, execve failed */
        printf("  ERROR: execve returned unexpectedly!\n");
        TEST_END(__func__);
}

int main(void)
{
        test_execve_basic();
        return 0;
}