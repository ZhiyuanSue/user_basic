#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"

/*
 * 测试通过时输出：
 * "getcwd OK."
 * 测试失败时输出：
 * "getcwd ERROR."
 */
void test_getcwd(void)
{
        TEST_START(__func__);

        char buf[128] = {0};
        char *cwd = getcwd(buf, 128);

        if (cwd == NULL) {
                printf("ERROR: getcwd syscall not implemented or failed\n");
                TEST_FAIL(__func__);
                return;
        }

        printf("getcwd: %s successfully!\n", buf);
        TEST_PASS(__func__);
}

int main(void)
{
        test_getcwd();
        return 0;
}
