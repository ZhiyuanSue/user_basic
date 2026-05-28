/*
 * TLS bring-up for user_payload + musl-gcc (-nostdlib custom ulib).
 *
 * musl-gcc __errno_location (x86_64):
 *   mov %fs:0, %rax ; add $-4, %rax
 * So %fs:0 must point at a qword holding (&errno + 4).
 *
 * Do NOT use (uintptr_t)&__errno_value here: musl expands that to "add %fs:0"
 * before FS is set, causing a user NULL access at 0x0 inside this function.
 */
#include "tls.h"

#include "stddef.h"
#include "syscall_ids.h"

/* Linker symbol in lib/arch/.../user.ld; RIP-relative only, no %%fs. */
extern char __tbss_start[];

#if defined(__x86_64__)
static uintptr_t __user_tls_head;

static long sys_arch_prctl_set_fs(unsigned long fs_base)
{
        long ret;
        __asm__ volatile("syscall"
                         : "=a"(ret)
                         : "a"((long)SYS_arch_prctl),
                           "D"((long)0x1002),
                           "S"((long)fs_base)
                         : "rcx", "r11", "memory");
        return ret;
}
#endif

void __init_user_tls(void)
{
#if defined(__x86_64__)
        /*
         * __errno_value is the first (only) .tbss object today; musl layout needs
         * word at %%fs:0 holds __tbss_start+4 (musl errno slot layout).
         */
        __user_tls_head = (uintptr_t)__tbss_start + 4;
        (void)sys_arch_prctl_set_fs((unsigned long)(uintptr_t)&__user_tls_head);
#elif defined(__aarch64__)
        __asm__ volatile("msr tpidr_el0, %0" ::"r"((uintptr_t)__tbss_start)
                         : "memory");
#else
        (void)0;
#endif
}
