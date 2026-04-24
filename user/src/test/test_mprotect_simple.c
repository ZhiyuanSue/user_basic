#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

/*
 * Simple mprotect test - basic functionality without signal handling
 * Tests mprotect syscall invocation and error handling
 */

static void test_mprotect_basic_call(void)
{
        TEST_START("mprotect_basic_call");

        /* Test 1: mprotect on valid mmap region */
        void *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed, cannot test mprotect\n");
                TEST_END("mprotect_basic_call");
                return;
        }

        printf("[INFO] mmap returned: %p\n", addr);

        /* Write to memory to ensure it's accessible */
        int *ptr = (int *)addr;
        *ptr = 0x12345678;
        printf("[INFO] Wrote 0x%x to %p\n", *ptr, ptr);

        /* Test mprotect: change to read-only */
        int ret = mprotect(addr, 4096, PROT_READ);
        if (ret != 0) {
                printf("[FAIL] mprotect returned %d (expected 0)\n", ret);
                munmap(addr, 4096);
                TEST_END("mprotect_basic_call");
                return;
        }

        printf("[PASS] mprotect successfully changed permissions to READ\n");

        /* Clean up */
        munmap(addr, 4096);
        TEST_END("mprotect_basic_call");
}

static void test_mprotect_error_handling(void)
{
        TEST_START("mprotect_error_handling");

        /* Test 1: mprotect on unaligned address */
        char *unaligned_addr = (char *)0x12345678 + 1; /* Not page-aligned */
        int ret = mprotect(unaligned_addr, 4096, PROT_READ);
        if (ret == 0) {
                printf("[FAIL] mprotect should fail on unaligned address\n");
                TEST_END("mprotect_error_handling");
                return;
        }
        printf("[PASS] mprotect correctly failed on unaligned address (returned %d)\n", ret);

        /* Test 2: mprotect on NULL address */
        ret = mprotect(NULL, 4096, PROT_READ);
        if (ret == 0) {
                printf("[FAIL] mprotect should fail on NULL address\n");
                TEST_END("mprotect_error_handling");
                return;
        }
        printf("[PASS] mprotect correctly failed on NULL address (returned %d)\n", ret);

        /* Test 3: mprotect with zero length (should succeed) */
        void *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed, cannot test zero-length mprotect\n");
                TEST_END("mprotect_error_handling");
                return;
        }

        ret = mprotect(addr, 0, PROT_READ);
        if (ret != 0) {
                printf("[INFO] mprotect with zero length returned %d (implementation choice)\n", ret);
        } else {
                printf("[INFO] mprotect with zero length succeeded (implementation choice)\n");
        }

        munmap(addr, 4096);
        TEST_END("mprotect_error_handling");
}

static void test_mprotect_permission_combinations(void)
{
        TEST_START("mprotect_permission_combinations");

        void *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed, cannot test permission changes\n");
                TEST_END("mprotect_permission_combinations");
                return;
        }

        printf("[INFO] Testing permission changes on %p\n", addr);

        /* Test 1: RW → R */
        int ret = mprotect(addr, 4096, PROT_READ);
        if (ret != 0) {
                printf("[FAIL] RW → R failed (returned %d)\n", ret);
                munmap(addr, 4096);
                TEST_END("mprotect_permission_combinations");
                return;
        }
        printf("[PASS] RW → R successful\n");

        /* Test 2: R → RW */
        ret = mprotect(addr, 4096, PROT_READ | PROT_WRITE);
        if (ret != 0) {
                printf("[FAIL] R → RW failed (returned %d)\n", ret);
                munmap(addr, 4096);
                TEST_END("mprotect_permission_combinations");
                return;
        }
        printf("[PASS] R → RW successful\n");

        /* Test 3: RW → RWX */
        ret = mprotect(addr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
        if (ret != 0) {
                printf("[FAIL] RW → RWX failed (returned %d)\n", ret);
                munmap(addr, 4096);
                TEST_END("mprotect_permission_combinations");
                return;
        }
        printf("[PASS] RW → RWX successful\n");

        /* Test 4: RWX → R */
        ret = mprotect(addr, 4096, PROT_READ);
        if (ret != 0) {
                printf("[FAIL] RWX → R failed (returned %d)\n", ret);
                munmap(addr, 4096);
                TEST_END("mprotect_permission_combinations");
                return;
        }
        printf("[PASS] RWX → R successful\n");

        /* Clean up */
        munmap(addr, 4096);
        TEST_END("mprotect_permission_combinations");
}

static void test_mprotect_multiple_pages(void)
{
        TEST_START("mprotect_multiple_pages");

        /* Test mprotect on multiple pages */
        void *addr = mmap(NULL, 8192, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed, cannot test multi-page mprotect\n");
                TEST_END("mprotect_multiple_pages");
                return;
        }

        printf("[INFO] Testing mprotect on 2 pages at %p\n", addr);

        /* Write to both pages */
        int *ptr = (int *)addr;
        ptr[0] = 0x11111111;
        ptr[1024] = 0x22222222;
        printf("[INFO] Wrote 0x%x to first page, 0x%x to second page\n", ptr[0], ptr[1024]);

        /* Change permissions on both pages */
        int ret = mprotect(addr, 8192, PROT_READ);
        if (ret != 0) {
                printf("[FAIL] mprotect on 2 pages failed (returned %d)\n", ret);
                munmap(addr, 8192);
                TEST_END("mprotect_multiple_pages");
                return;
        }

        printf("[PASS] mprotect on 2 pages successful\n");

        /* Clean up */
        munmap(addr, 8192);
        TEST_END("mprotect_multiple_pages");
}

int main(void)
{
        printf("========================================\n");
        printf("SIMPLE MPROTECT TEST SUITE\n");
        printf("========================================\n");

        test_mprotect_basic_call();
        test_mprotect_error_handling();
        test_mprotect_permission_combinations();
        test_mprotect_multiple_pages();

        printf("========================================\n");
        printf("MPROTECT TEST SUITE FINISHED\n");
        printf("NOTE: Some tests verify syscall invocation only\n");
        printf("      Full permission verification requires signal handling\n");
        printf("========================================\n");

        return 0;
}
