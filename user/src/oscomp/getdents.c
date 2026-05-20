#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

char buf[512];
void test_getdents(void)
{
        TEST_START(__func__);

        int fd, nread;
        struct linux_dirent64 *dirp64;
        dirp64 = buf;

        fd = open(".", O_RDONLY);
        if (fd < 0) {
                printf("ERROR: open syscall not implemented or failed (returned %d)\n", fd);
                TEST_FAIL(__func__);
                return;
        }

        printf("open fd:%d\n", fd);

        nread = getdents(fd, dirp64, 512);
        if (nread < 0) {
                printf("ERROR: getdents syscall not implemented or failed (returned %d)\n", nread);
                close(fd);
                TEST_FAIL(__func__);
                return;
        }

        printf("getdents fd:%d\n", nread);
        printf("getdents success.\n%s\n", dirp64->d_name);

        close(fd);
        TEST_PASS(__func__);
}

int main(void)
{
        test_getdents();
        return 0;
}
