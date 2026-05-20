#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void test_write()
{
        TEST_START(__func__);

        const char *str = "Hello operating system contest.\n";
        int str_len = strlen(str);

        int written = write(STDOUT, str, str_len);
        if (written < 0) {
                printf("ERROR: write syscall not implemented or failed (returned %d)\n", written);
                TEST_FAIL(__func__);
                return;
        }

        if (written != str_len) {
                printf("ERROR: write syscall incomplete write (wrote %d, expected %d)\n", written, str_len);
                TEST_FAIL(__func__);
                return;
        }

        TEST_PASS(__func__);
}

int main(void)
{
        test_write();
        return 0;
}
