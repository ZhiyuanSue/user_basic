#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

void test_mkdir(void)
{
        TEST_START(__func__);

        int rt, fd;

        rt = mkdir("test_mkdir", 0666);
        if (rt < 0) {
                printf("ERROR: mkdir syscall not implemented or failed (returned %d)\n", rt);
                TEST_FAIL(__func__);
                return;
        }

        printf("mkdir ret: %d\n", rt);

        fd = open("test_mkdir", O_RDONLY | O_DIRECTORY);
        if (fd < 0) {
                printf("ERROR: open syscall failed for created directory (returned %d)\n", fd);
                TEST_FAIL(__func__);
                return;
        }

        printf("  mkdir success.\n");
        close(fd);

        TEST_PASS(__func__);
}

int main(void)
{
        test_mkdir();
        return 0;
}
