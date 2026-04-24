#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

/*
 * Simple mmap test - no filesystem dependencies
 * This should be the first test to run since it only tests basic mmap/munmap
 */

static void test_mmap_anonymous_basic(void)
{
        TEST_START("mmap_anonymous_basic");

        void *addr;

        /* Test 1: Simple anonymous mapping - NULL address */
        addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[FAIL] mmap returned MAP_FAILED\n");
                TEST_END("mmap_anonymous_basic");
                exit(1);
        }

        printf("[INFO] mmap(NULL, 4096) returned: %p\n", addr);

        /* Test 2: Write to memory */
        int *ptr = (int *)addr;
        *ptr = 0x12345678;
        printf("[INFO] Wrote 0x%x to %p, read back: 0x%x\n", 0x12345678, ptr, *ptr);

        if (*ptr != 0x12345678) {
                printf("[FAIL] Data mismatch\n");
                munmap(addr, 4096);
                TEST_END("mmap_anonymous_basic");
                exit(1);
        }

        /* Test 3: Clean up */
        int ret = munmap(addr, 4096);
        if (ret != 0) {
                printf("[FAIL] munmap returned %d (expected 0)\n", ret);
                TEST_END("mmap_anonymous_basic");
                exit(1);
        }

        printf("[PASS] Basic mmap/munmap test\n");
        TEST_END("mmap_anonymous_basic");
}

static void test_mmap_multiple_pages(void)
{
        TEST_START("mmap_multiple_pages");

        /* Test mapping multiple pages */
        void *addr = mmap(NULL, 8192, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[FAIL] mmap(8192) returned MAP_FAILED\n");
                TEST_END("mmap_multiple_pages");
                exit(1);
        }

        printf("[INFO] mmap(NULL, 8192) returned: %p\n", addr);

        /* Write to both pages */
        int *ptr = (int *)addr;
        ptr[0] = 0x11111111;
        ptr[1024] = 0x22222222;  /* Second page */

        printf("[INFO] Wrote 0x%x to first page, 0x%x to second page\n", ptr[0], ptr[1024]);

        /* Clean up */
        int ret = munmap(addr, 8192);
        if (ret != 0) {
                printf("[FAIL] munmap returned %d\n", ret);
                TEST_END("mmap_multiple_pages");
                exit(1);
        }

        printf("[PASS] Multiple pages test\n");
        TEST_END("mmap_multiple_pages");
}

static void test_mmap_alignment(void)
{
        TEST_START("mmap_alignment");

        /* Test that mmap returns page-aligned addresses */
        void *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[FAIL] mmap returned MAP_FAILED\n");
                TEST_END("mmap_alignment");
                exit(1);
        }

        if ((uintptr_t)addr & 0xFFF) {
                printf("[FAIL] mmap returned non-page-aligned address: %p\n", addr);
                munmap(addr, 4096);
                TEST_END("mmap_alignment");
                exit(1);
        }

        printf("[PASS] mmap returned page-aligned address: %p\n", addr);
        munmap(addr, 4096);
        TEST_END("mmap_alignment");
}

int main(void)
{
        printf("========================================\n");
        printf("SIMPLE MMAP TEST SUITE\n");
        printf("========================================\n");

        test_mmap_anonymous_basic();
        test_mmap_multiple_pages();
        test_mmap_alignment();

        printf("========================================\n");
        printf("ALL MMAP TESTS PASSED\n");
        printf("========================================\n");

        return 0;
}
