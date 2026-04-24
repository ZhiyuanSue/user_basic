#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * Curated syscall test (from oscomp): getpid
 *
 * Expected: prints process ID and exits successfully
 */

static void test_getpid(void)
{
        TEST_START("getpid");

        int pid = getpid();

        if (pid < 0) {
                printf("[FAIL] getpid returned negative value: %d\n", pid);
                TEST_END("getpid");
                exit(1);
        }

        printf("[INFO] getpid() = %d\n", pid);
        printf("[PASS] getpid returned valid PID\n");

        TEST_END("getpid");
}

int main(void)
{
        test_getpid();
        return 0;
}

