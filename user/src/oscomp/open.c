#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"

void test_open()
{
        TEST_START(__func__);

        // O_RDONLY = 0, O_WRONLY = 1
        int fd = open("./text.txt", 0);
        if (fd < 0) {
                printf("ERROR: open syscall not implemented or failed (returned %d)\n", fd);
                TEST_FAIL(__func__);
                return;
        }

        char buf[256];
        int size = read(fd, buf, 256);
        if (size < 0) {
                printf("ERROR: read syscall not implemented or failed (returned %d)\n", size);
                close(fd);
                TEST_FAIL(__func__);
                return;
        }

        write(STDOUT, buf, size);
        close(fd);

        TEST_PASS(__func__);
}

int main(void)
{
        test_open();
        return 0;
}
