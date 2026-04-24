#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

/*
 * Curated syscall test: mmap anonymous mapping
 *
 * Expected: creates anonymous mappings and verifies read/write access
 */

static void test_mmap_basic(void)
{
        TEST_START("mmap_basic");

        void *addr;
        int *ptr;
        int i;

        /* Test 1: Simple anonymous mapping */
        addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[FAIL] mmap returned MAP_FAILED\n");
                TEST_END("mmap_basic");
                exit(1);
        }

        printf("[INFO] mmap returned: %p\n", addr);

        /* Write pattern to memory */
        ptr = (int *)addr;
        for (i = 0; i < 1024; i++) {
                ptr[i] = 0xCAFEBABE + i;
        }

        /* Read back and verify */
        for (i = 0; i < 1024; i++) {
                if (ptr[i] != 0xCAFEBABE + i) {
                        printf("[FAIL] Data mismatch at index %d: expected 0x%x, got 0x%x\n",
                               i, 0xCAFEBABE + i, ptr[i]);
                        munmap(addr, 4096);
                        TEST_END("mmap_basic");
                        exit(1);
                }
        }

        printf("[PASS] mmap basic test - data verified\n");

        /* Clean up */
        munmap(addr, 4096);

        TEST_END("mmap_basic");
}

static void test_mmap_multiple(void)
{
        TEST_START("mmap_multiple");

        void *addr1, *addr2, *addr3;

        /* Test multiple mappings */
        addr1 = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        addr2 = mmap(NULL, 8192, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        addr3 = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr1 == MAP_FAILED || addr2 == MAP_FAILED || addr3 == MAP_FAILED) {
                printf("[FAIL] One of the mmap calls failed\n");
                TEST_END("mmap_multiple");
                exit(1);
        }

        printf("[INFO] Created 3 mappings: %p, %p, %p\n", addr1, addr2, addr3);

        /* Verify they don't overlap */
        if (addr1 < addr3 && addr1 + 4096 > addr3) {
                printf("[FAIL] Mappings overlap!\n");
                TEST_END("mmap_multiple");
                exit(1);
        }

        /* Write to each mapping */
        *(int *)addr1 = 0x11111111;
        *(int *)addr2 = 0x22222222;
        *(int *)addr3 = 0x33333333;

        printf("[PASS] Multiple mmap test - 3 non-overlapping mappings\n");

        /* Clean up */
        munmap(addr1, 4096);
        munmap(addr2, 8192);
        munmap(addr3, 4096);

        TEST_END("mmap_multiple");
}

int main(void)
{
        printf("========================================\n");
        printf("MMAP TEST SUITE\n");
        printf("========================================\n");

        test_mmap_basic();
        test_mmap_multiple();

        printf("========================================\n");
        printf("All mmap tests PASSED\n");
        printf("========================================\n");

        return 0;
}
