#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "stddef.h"

#define AT_FDCWD (-100) // 相对路径

// Stat *kst;
static struct kstat kst;
void test_fstat()
{
        TEST_START(__func__);

        int fd = open("./text.txt", 0);
        if (fd < 0) {
                printf("ERROR: open syscall not implemented or failed (returned %d)\n", fd);
                TEST_FAIL(__func__);
                return;
        }

        int ret = fstat(fd, &kst);
        if (ret < 0) {
                printf("ERROR: fstat syscall not implemented or failed (returned %d)\n", ret);
                close(fd);
                TEST_FAIL(__func__);
                return;
        }

        printf("fstat ret: %d\n", ret);
        printf("fstat: dev: %d, inode: %d, mode: %d, nlink: %d, size: %d, atime: %d, mtime: %d, ctime: %d\n",
               kst.st_dev,
               kst.st_ino,
               kst.st_mode,
               kst.st_nlink,
               kst.st_size,
               kst.st_atime_sec,
               kst.st_mtime_sec,
               kst.st_ctime_sec);

        close(fd);
        TEST_PASS(__func__);
}

int main(void)
{
        test_fstat();
        return 0;
}
