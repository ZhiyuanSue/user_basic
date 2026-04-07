#include "unistd.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

/*
 * Curated syscall test (from oscomp): munmap (+ mmap as setup)
 *
 * Note: depends on file syscalls too (open/write/fstat/close). Keep it in
 * `src/test` but only run it when those syscalls are ready.
 */

static struct kstat kst;

static void test_munmap(void)
{
        TEST_START(__func__);
        char *array;
        const char *str = "  Hello, mmap successfully!";
        int fd;

        fd = open("test_mmap.txt", O_RDWR | O_CREATE);
        write(fd, str, strlen(str));
        fstat(fd, &kst);
        printf("file len: %d\n", kst.st_size);
        array = mmap(NULL,
                     kst.st_size,
                     PROT_WRITE | PROT_READ,
                     MAP_FILE | MAP_SHARED,
                     fd,
                     0);

        if (array == MAP_FAILED) {
                printf("mmap error.\n");
        } else {
                int ret = munmap(array, kst.st_size);
                printf("munmap return: %d\n", ret);
                assert(ret == 0);

                if (ret == 0)
                        printf("munmap successfully!\n");
        }
        close(fd);

        TEST_END(__func__);
}

int main(void)
{
        test_munmap();
        return 0;
}

