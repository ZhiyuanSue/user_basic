#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"

void test_openat(void)
{
        TEST_START(__func__);

        // Test if openat syscall is implemented
        int fd_dir = open("./mnt", O_DIRECTORY);
        if (fd_dir < 0) {
                printf("ERROR: open syscall not implemented or failed (returned %d)\n", fd_dir);
                TEST_FAIL(__func__);
                return;
        }

        printf("open dir fd: %d\n", fd_dir);

        int fd = openat(fd_dir, "test_openat.txt", O_CREATE | O_RDWR);
        if (fd < 0) {
                printf("ERROR: openat syscall not implemented or failed (returned %d)\n", fd);
                close(fd_dir);
                TEST_FAIL(__func__);
                return;
        }

        printf("openat fd: %d\n", fd);
        printf("openat success.\n");

        close(fd);
        close(fd_dir);

        TEST_PASS(__func__);
}

int main(void)
{
        test_openat();
        return 0;
}
