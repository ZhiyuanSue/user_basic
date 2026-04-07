#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * Curated syscall test (from oscomp): getpid
 *
 * Expected: prints "getpid success." and a non-negative pid.
 */

static void test_getpid(void)
{
        TEST_START(__func__);
        int pid = getpid();
        assert(pid >= 0);
        printf("getpid success.\npid = %d\n", pid);
        TEST_END(__func__);
}

int main(void)
{
        test_getpid();
        return 0;
}

