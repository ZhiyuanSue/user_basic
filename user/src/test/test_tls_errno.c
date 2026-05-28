#include <errno.h>
#include <stdio.h>
#include <unistd.h>

/*
 * TLS / errno smoke test for user_payload + musl-gcc.
 *
 * Requires:
 * - __init_user_tls() in __start_main (sets %fs / TPIDR)
 * - kernel sys_arch_prctl (x86_64)
 * - syscall wrappers that set errno on failure
 *
 * Without TLS init, %fs:0 is unset; musl-gcc then yields errno at ~0xfffffffc.
 */

int main(void)
{
        int *ep = __errno_location();

        printf("=== TLS errno test ===\n");
        printf("__errno_location() = %p\n", (void *)ep);

        if (!ep) {
                printf("FAIL: null errno pointer\n");
                return 1;
        }

        *ep = 42;
        if (errno != 42) {
                printf("FAIL: errno macro does not see TLS write (got %d)\n",
                       errno);
                return 1;
        }

        /* Provoke EINVAL from kernel (bad fd read). */
        errno = 0;
        ssize_t n = read(-1, ep, sizeof(*ep));
        if (n != -1) {
                printf("FAIL: read(-1) should fail (got %ld)\n", (long)n);
                return 1;
        }
        if (errno == 0) {
                printf("FAIL: errno still 0 after failed read\n");
                return 1;
        }
        printf("read(-1) errno=%d (expected non-zero)\n", errno);

        printf("PASS: TLS errno basic\n");
        return 0;
}
