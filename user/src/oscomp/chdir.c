#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

static char buffer[30];
void test_chdir(void)
{
        TEST_START(__func__);

        int ret = mkdir("test_chdir", 0666);
        if (ret < 0) {
                printf("ERROR: mkdir syscall not implemented or failed (returned %d)\n", ret);
                TEST_FAIL(__func__);
                return;
        }

        ret = chdir("test_chdir");
        if (ret < 0) {
                printf("ERROR: chdir syscall not implemented or failed (returned %d)\n", ret);
                TEST_FAIL(__func__);
                return;
        }

        printf("chdir ret: %d\n", ret);

        char* cwd = getcwd(buffer, 30);
        if (cwd == NULL) {
                printf("ERROR: getcwd syscall not implemented or failed\n");
                TEST_FAIL(__func__);
                return;
        }

        printf("  current working dir : %s\n", buffer);
        TEST_PASS(__func__);
}

int main(void)
{
        test_chdir();
        return 0;
}
