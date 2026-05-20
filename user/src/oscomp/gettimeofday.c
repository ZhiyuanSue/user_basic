#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"

/*
 * 测试通过时的输出：
 * "gettimeofday success."
 * "start:[num], end:[num]"
 * "interval: [num]"	注：数字[num]的值应大于0
 * 测试失败时的输出：
 * "gettimeofday error."
 */
void test_gettimeofday()
{
        TEST_START(__func__);

        int test_ret1 = get_time();
        if (test_ret1 < 0) {
                printf("ERROR: gettimeofday syscall not implemented or failed (returned %d)\n", test_ret1);
                TEST_FAIL(__func__);
                return;
        }

        volatile int i = 12500000; // qemu时钟频率12500000
        while (i > 0)
                i--;

        int test_ret2 = get_time();
        if (test_ret2 < 0) {
                printf("ERROR: gettimeofday syscall not implemented or failed (returned %d)\n", test_ret2);
                TEST_FAIL(__func__);
                return;
        }

        printf("gettimeofday success.\n");
        printf("start:%d, end:%d\n", test_ret1, test_ret2);
        printf("interval: %d\n", test_ret2 - test_ret1);

        TEST_PASS(__func__);
}

int main(void)
{
        test_gettimeofday();
        return 0;
}
