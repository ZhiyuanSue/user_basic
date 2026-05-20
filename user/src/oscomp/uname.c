#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct utsname {
        char sysname[65];
        char nodename[65];
        char release[65];
        char version[65];
        char machine[65];
        char domainname[65];
};

struct utsname un;

void test_uname()
{
        TEST_START(__func__);

        int test_ret = uname(&un);
        if (test_ret < 0) {
                printf("ERROR: uname syscall not implemented or failed (returned %d)\n", test_ret);
                TEST_FAIL(__func__);
                return;
        }

        printf("Uname: %s %s %s %s %s %s\n",
               un.sysname,
               un.nodename,
               un.release,
               un.version,
               un.machine,
               un.domainname);

        TEST_PASS(__func__);
}

int main(void)
{
        test_uname();
        return 0;
}
