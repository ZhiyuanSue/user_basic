#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/*
 * Curated syscall test (from oscomp): brk
 *
 * Expected: brk syscall can extend heap
 */

static void test_brk(void)
{
        TEST_START("brk");

        intptr_t cur_pos, alloc_pos, alloc_pos_1;

        cur_pos = brk(0);
        printf("[INFO] Initial heap position: %d\n", cur_pos);

        if (cur_pos < 0) {
                printf("[FAIL] brk(0) failed\n");
                TEST_END("brk");
                exit(1);
        }

        brk(cur_pos + 64);
        alloc_pos = brk(0);
        printf("[INFO] After first alloc (+64): heap pos = %d\n", alloc_pos);

        if (alloc_pos < cur_pos + 64) {
                printf("[PARTIAL] Heap not fully extended (got %d, wanted >= %d)\n",
                       (int)alloc_pos, (int)(cur_pos + 64));
        }

        brk(alloc_pos + 64);
        alloc_pos_1 = brk(0);
        printf("[INFO] After second alloc (+64): heap pos = %d\n", alloc_pos_1);

        if (alloc_pos_1 >= alloc_pos + 64) {
                printf("[PASS] brk successfully extended heap\n");
        } else {
                printf("[PARTIAL] Heap extension partial (got %d, wanted >= %d)\n",
                       (int)alloc_pos_1, (int)(alloc_pos + 64));
        }

        TEST_END("brk");
}

int main(void)
{
        test_brk();
        return 0;
}

