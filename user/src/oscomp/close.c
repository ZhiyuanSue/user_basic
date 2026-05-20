#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/*
 * 测试成功则输出：
 * "  close success."
 * 测试失败则输出：
 * "  close error."
 */

void test_close(void)
{
        TEST_START(__func__);

        int fd = open("test_close.txt", O_CREATE | O_RDWR);
        if (fd < 0) {
                printf("ERROR: open syscall not implemented or failed (returned %d)\n", fd);
                TEST_FAIL(__func__);
                return;
        }

        const char *str = "  close error.\n";
        int str_len = strlen(str);

        int written = write(fd, str, str_len);
        if (written < 0) {
                printf("ERROR: write syscall failed (returned %d)\n", written);
                close(fd);
                TEST_FAIL(__func__);
                return;
        }

        int rt = close(fd);
        if (rt != 0) {
                printf("ERROR: close syscall failed (returned %d)\n", rt);
                TEST_FAIL(__func__);
                return;
        }

        printf("  close %d success.\n", fd);
        TEST_PASS(__func__);
}

int main(void)
{
        test_close();
        return 0;
}
