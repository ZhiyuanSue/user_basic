#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"

struct tms {
        long tms_utime;
        long tms_stime;
        long tms_cutime;
        long tms_cstime;
};

struct tms mytimes;

void test_times()
{
        TEST_START(__func__);

        clock_t test_ret = times(&mytimes);
        if (test_ret < 0) {
                printf("ERROR: times syscall not implemented or failed (returned %d)\n", test_ret);
                TEST_FAIL(__func__);
                return;
        }

        printf("mytimes success\n{tms_utime:%d, tms_stime:%d, tms_cutime:%d, tms_cstime:%d}\n",
               mytimes.tms_utime,
               mytimes.tms_stime,
               mytimes.tms_cutime,
               mytimes.tms_cstime);

        TEST_PASS(__func__);
}

int main(void)
{
        test_times();
        return 0;
}
