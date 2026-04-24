#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

/*
 * Simple mremap test - basic functionality
 * Tests memory remapping with different scenarios
 */

static void test_mremap_shrink(void)
{
        TEST_START("mremap_shrink");

        /* Create initial mapping */
        void* addr = mmap(NULL, 8192, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed\n");
                TEST_END("mremap_shrink");
                return;
        }

        printf("[INFO] Initial mapping: %p, size: 8192\n", addr);

        /* Write pattern to verify data preservation */
        int* ptr = (int*)addr;
        for (int i = 0; i < 2048; i++) {  // 8192 bytes = 2048 ints
                ptr[i] = 0xDEADBEEF + i;
        }

        /* Shrink to 4096 bytes */
        void* new_addr = mremap(addr, 8192, 4096, 0);

        if (new_addr == MAP_FAILED) {
                printf("[FAIL] mremap failed (should be able to shrink)\n");
                munmap(addr, 8192);
                TEST_END("mremap_shrink");
                return;
        }

        if (new_addr != addr) {
                printf("[FAIL] address changed on shrink (expected %p, got %p)\n", addr, new_addr);
                munmap(new_addr, 4096);
                TEST_END("mremap_shrink");
                return;
        }

        /* Verify data is still accessible */
        for (int i = 0; i < 1024; i++) {  // 4096 bytes = 1024 ints
                if (ptr[i] != 0xDEADBEEF + i) {
                        printf("[FAIL] data mismatch after shrink at index %d\n", i);
                        munmap(new_addr, 4096);
                        TEST_END("mremap_shrink");
                        return;
                }
        }

        printf("[PASS] mremap shrink successful, data preserved\n");
        munmap(new_addr, 4096);
        TEST_END("mremap_shrink");
}

static void test_mremap_expand_in_place(void)
{
        TEST_START("mremap_expand_in_place");

        /* Create initial mapping */
        void* addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed\n");
                TEST_END("mremap_expand_in_place");
                return;
        }

        printf("[INFO] Initial mapping: %p, size: 4096\n", addr);

        /* Write initial data */
        int* ptr = (int*)addr;
        ptr[0] = 0x12345678;
        ptr[1023] = 0x87654321;

        /* Try to expand to 8192 bytes */
        void* new_addr = mremap(addr, 4096, 8192, 0);

        if (new_addr == MAP_FAILED) {
                printf("[INFO] mremap expand failed (may be expected if can't grow in place)\n");
                munmap(addr, 4096);
                TEST_END("mremap_expand_in_place");
                return;
        }

        if (new_addr != addr) {
                printf("[INFO] address changed on expand to %p (MAYMOVE not set)\n", new_addr);
                munmap(new_addr, 8192);
                TEST_END("mremap_expand_in_place");
                return;
        }

        /* Verify original data */
        if (ptr[0] != 0x12345678 || ptr[1023] != 0x87654321) {
                printf("[FAIL] data mismatch after expand\n");
                munmap(new_addr, 8192);
                TEST_END("mremap_expand_in_place");
                return;
        }

        /* Write to new area */
        ptr[1024] = 0xAABBCCDD;

        printf("[PASS] mremap expand in place successful\n");
        munmap(new_addr, 8192);
        TEST_END("mremap_expand_in_place");
}

static void test_mremap_expand_with_move(void)
{
        TEST_START("mremap_expand_with_move");

        /* Create initial mapping */
        void* addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed\n");
                TEST_END("mremap_expand_with_move");
                return;
        }

        printf("[INFO] Initial mapping: %p, size: 4096\n", addr);

        /* Write data to verify copy */
        int* ptr = (int*)addr;
        for (int i = 0; i < 1024; i++) {
                ptr[i] = 0x11111111 + i;
        }

        /* Try to expand with MAYMOVE flag */
        void* new_addr = mremap(addr, 4096, 8192, MREMAP_MAYMOVE);

        if (new_addr == MAP_FAILED) {
                printf("[INFO] mremap with MAYMOVE failed (expected if OOM)\n");
                munmap(addr, 4096);
                TEST_END("mremap_expand_with_move");
                return;
        }

        if (new_addr == addr) {
                printf("[INFO] expanded in place even with MAYMOVE\n");
        } else {
                printf("[INFO] moved from %p to %p\n", addr, new_addr);
        }

        /* Verify data was copied */
        int* new_ptr = (int*)new_addr;
        for (int i = 0; i < 1024; i++) {
                if (new_ptr[i] != 0x11111111 + i) {
                        printf("[FAIL] data mismatch after move at index %d\n", i);
                        munmap(new_addr, 8192);
                        TEST_END("mremap_expand_with_move");
                        return;
                }
        }

        printf("[PASS] mremap with MAYMOVE successful, data preserved\n");
        munmap(new_addr, 8192);
        TEST_END("mremap_expand_with_move");
}

static void test_mremap_same_size(void)
{
        TEST_START("mremap_same_size");

        void* addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed\n");
                TEST_END("mremap_same_size");
                return;
        }

        /* Remap to same size */
        void* new_addr = mremap(addr, 4096, 4096, 0);

        if (new_addr == MAP_FAILED) {
                printf("[FAIL] mremap same size failed\n");
                munmap(addr, 4096);
                TEST_END("mremap_same_size");
                return;
        }

        if (new_addr != addr) {
                printf("[INFO] address changed on same-size remap (implementation choice)\n");
        }

        printf("[PASS] mremap same size successful\n");
        munmap(new_addr, 4096);
        TEST_END("mremap_same_size");
}

static void test_mremap_error_handling(void)
{
        TEST_START("mremap_error_handling");

        /* Test 1: Invalid old address (not page aligned) */
        void* bad_addr = (void*)0x1234;
        void* result = mremap(bad_addr, 4096, 8192, 0);
        if (result != MAP_FAILED) {
                printf("[FAIL] should fail on unaligned address\n");
                TEST_END("mremap_error_handling");
                return;
        }
        printf("[PASS] correctly failed on unaligned address\n");

        /* Test 2: Zero old size */
        result = mremap((void*)0x1000, 0, 4096, 0);
        if (result != MAP_FAILED) {
                printf("[FAIL] should fail on zero old size\n");
                TEST_END("mremap_error_handling");
                return;
        }
        printf("[PASS] correctly failed on zero old size\n");

        /* Test 3: Zero new size */
        void* addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (addr == MAP_FAILED) {
                printf("[SKIP] mmap failed\n");
                TEST_END("mremap_error_handling");
                return;
        }

        result = mremap(addr, 4096, 0, 0);
        if (result != MAP_FAILED) {
                printf("[FAIL] should fail on zero new size\n");
                munmap(addr, 4096);
                TEST_END("mremap_error_handling");
                return;
        }
        printf("[PASS] correctly failed on zero new size\n");
        munmap(addr, 4096);

        TEST_END("mremap_error_handling");
}

int main(void)
{
        printf("========================================\n");
        printf("SIMPLE MREMAP TEST SUITE\n");
        printf("========================================\n");

        test_mremap_shrink();
        test_mremap_expand_in_place();
        test_mremap_expand_with_move();
        test_mremap_same_size();
        test_mremap_error_handling();

        printf("========================================\n");
        printf("MREMAP TEST SUITE FINISHED\n");
        printf("========================================\n");

        return 0;
}
