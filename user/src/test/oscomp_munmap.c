#include "unistd.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

/*
 * Curated syscall test (from oscomp): munmap (+ mmap as setup)
 *
 * Note: depends on file syscalls too (open/write/fstat/close).
 * This test requires filesystem support.
 */

static struct kstat kst;

static void test_munmap(void)
{
        TEST_START("munmap");

        char *array;
        const char *str = "  Hello, mmap successfully!";
        int fd;

        printf("[INFO] This test requires filesystem support\n");

        fd = open("test_mmap.txt", O_RDWR | O_CREATE);
        if (fd < 0) {
                printf("[SKIP] Cannot open file (filesystem not available)\n");
                TEST_END("munmap");
                return;
        }

        write(fd, str, strlen(str));
        fstat(fd, &kst);
        printf("[INFO] File size: %d\n", kst.st_size);

        array = mmap(NULL,
                     kst.st_size,
                     PROT_WRITE | PROT_READ,
                     MAP_FILE | MAP_SHARED,
                     fd,
                     0);

        if (array == MAP_FAILED) {
                printf("[FAIL] mmap failed\n");
                close(fd);
                TEST_END("munmap");
                return;
        }

        int ret = munmap(array, kst.st_size);
        printf("[INFO] munmap returned: %d\n", ret);

        if (ret != 0) {
                printf("[FAIL] munmap failed (returned %d, expected 0)\n", ret);
        } else {
                printf("[PASS] munmap successful\n");
        }

        close(fd);

        TEST_END("munmap");
}

int main(void)
{
        printf("========================================\n");
        printf("MUNMAP TEST SUITE\n");
        printf("========================================\n");

        test_munmap();

        printf("========================================\n");
        printf("Test completed\n");
        printf("========================================\n");

        return 0;
}

