#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>

/* 测试 unlink
 * 测试通过，应有如下输出：
 * "  unlink success!"
 * 测试失败，应有如下输出：
 * "  unlink error!"
 */

int test_unlink()
{
        TEST_START(__func__);

        char *fname = "./test_unlink";
        int fd, ret;

        fd = open(fname, O_CREATE | O_WRONLY);
        if (fd < 0) {
                printf("ERROR: open syscall not implemented or failed (returned %d)\n", fd);
                TEST_FAIL(__func__);
                return;
        }
        close(fd);

        // unlink test
        ret = unlink(fname);
        if (ret < 0) {
                printf("ERROR: unlink syscall not implemented or failed (returned %d)\n", ret);
                TEST_FAIL(__func__);
                return;
        }

        fd = open(fname, O_RDONLY);
        if (fd < 0) {
                printf("  unlink success!\n");
                TEST_PASS(__func__);
        } else {
                printf("  unlink error!\n");
                close(fd);
                TEST_FAIL(__func__);
        }
        // It's Ok if you don't delete the inode and data blocks.
}

int main(void)
{
        test_unlink();
        return 0;
}
