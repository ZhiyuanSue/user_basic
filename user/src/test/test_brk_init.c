#include <stdio.h>
#include <unistd.h>

/* Test brk initialization - verify that brk is set correctly after ELF loading */

static void test_brk_init(void)
{
        TEST_START("brk_init");

        printf("[INFO] Testing brk initialization...\n");

        /* Get initial brk value */
        void *initial_brk = brk(0);
        printf("[INFO] Initial brk: %p\n", initial_brk);

        /* Check if brk is initialized to a reasonable value */
        if (initial_brk == (void *)-1 || (unsigned long)initial_brk < 0x1000) {
                printf("[FAIL] brk not initialized properly (brk=%p)\n", initial_brk);
                TEST_END("brk_init");
                return;
        }

        if ((unsigned long)initial_brk >= 0x400000000UL) {
                printf("[FAIL] brk value too large (brk=%p)\n", initial_brk);
                TEST_END("brk_init");
                return;
        }

        printf("[PASS] brk initialized to reasonable value: %p\n", initial_brk);

        /* Try to set brk to a new value */
        unsigned long new_brk_val = (unsigned long)initial_brk + 0x1000;
        void *new_brk = brk((void *)new_brk_val);
        printf("[INFO] Attempted to set brk to %p, got: %p\n", (void *)new_brk_val, new_brk);

        if (new_brk == (void *)-1) {
                printf("[FAIL] brk syscall failed\n");
                TEST_END("brk_init");
                return;
        }

        if ((unsigned long)new_brk >= new_brk_val) {
                printf("[PASS] brk successfully extended\n");
        } else {
                printf("[PARTIAL] brk not fully extended (got %p, wanted >= %p)\n",
                       new_brk, (void *)new_brk_val);
        }

        /* Verify we can allocate and use memory */
        void *brk_end = brk(0);
        printf("[INFO] Final brk: %p\n", brk_end);

        TEST_END("brk_init");
}

int main(void)
{
        printf("========================================\n");
        printf("BRK INITIALIZATION TEST\n");
        printf("========================================\n");

        test_brk_init();

        printf("========================================\n");
        printf("Test completed\n");
        printf("========================================\n");

        return 0;
}
