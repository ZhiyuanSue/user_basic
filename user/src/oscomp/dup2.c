#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/*
 * 测试通过时应输出：
 * "  from fd 100"
 */
void test_dup2()
{
        TEST_START(__func__);

        int fd = dup2(STDOUT, 100);
        if (fd < 0) {
                printf("ERROR: dup2 syscall not implemented or failed (returned %d)\n", fd);
                TEST_FAIL(__func__);
                return;
        }

        const char *str = "  from fd 100\n";
        int written = write(100, str, strlen(str));
        if (written < 0) {
                printf("ERROR: write syscall failed (returned %d)\n", written);
                TEST_FAIL(__func__);
                return;
        }

        TEST_PASS(__func__);
}

int main(void)
{
        test_dup2();
        return 0;
}
