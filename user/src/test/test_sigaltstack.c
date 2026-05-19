#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Test sigaltstack functionality
 *
 * This test verifies that:
 * 1. sigaltstack syscall works correctly
 * 2. Alternate stack can be set and queried
 * 3. Signal handlers can use SA_ONSTACK flag
 */

/* Include architecture-specific syscall numbers from user payload lib */
#include "../lib/syscall_ids.h"

/* Basic types */
typedef unsigned long long u64;
typedef long long i64;

/* Standard signal definitions */
#define SIGUSR1		10
#define SIGUSR2		12

/* Signal handler constants */
#define SIG_DFL		((void (*)(int))0)  /* Default handling */
#define SIG_IGN		((void (*)(int))1)  /* Ignore signal */

/* Signal set structure */
typedef struct {
    unsigned long sig[1];  /* Simplified: just one element */
} sigset_t;

/* sigaction structure for kernel */
typedef struct {
    void (*handler)(int);
    unsigned long flags;
    sigset_t mask;
} sigaction_t;

/* Alternate signal stack */
typedef struct {
    void *ss_sp;
    int ss_flags;
    unsigned long ss_size;
} stack_t;

/* Stack flags */
#define SS_ONSTACK	0x00000001
#define SS_DISABLE	0x00000002

/* Signal action flags */
#define SA_ONSTACK	0x08000000

/* Minimum stack size */
#define MINSIGSTKSZ	2048
#define SIGSTKSZ	8192

/* Test signal stack */
static char signal_stack[SIGSTKSZ];
static volatile int signal_stack_used = 0;

/* Signal handler that should execute on alternate stack */
void signal_handler(int sig)
{
    /* Check if we're on the signal stack */
    char local_var;
    char *sp = &local_var;

    /* Check if SP is within the signal stack range */
    if (sp >= signal_stack && sp < signal_stack + SIGSTKSZ) {
        signal_stack_used = 1;
    }
}

/* Helper function for write syscall - multi-architecture */
static void my_write(const char *str)
{
#if defined(__x86_64__)
    __asm__ volatile (
        "mov $1, %%rax\n\t"
        "mov $1, %%rdi\n\t"
        "syscall"
        : : "S"(str)
        : "rax", "rdi", "rcx", "r11", "memory"
    );
#elif defined(__aarch64__)
    register long x8 asm("x8") = 64;   /* SYS_write */
    register long x0 asm("x0") = 1;    /* stdout */
    register const char *x1 asm("x1") = str;
    __asm__ volatile (
        "svc #0"
        : : "r"(x8), "r"(x0), "r"(x1)
        : "memory"
    );
#endif
}

/* Syscall wrappers */
static long my_rt_sigaction(int sig, void (*handler)(int), unsigned long flags)
{
    long result;
    u64 act_ptr = 0;
    sigaction_t new_action;

    if (handler != SIG_DFL && handler != SIG_IGN) {
        /* Custom handler */
        new_action.handler = handler;
        new_action.flags = flags;
        new_action.mask.sig[0] = 0;
        act_ptr = (u64)&new_action;
    }

#ifdef __x86_64__
    __asm__ volatile (
        "mov %[syscall], %%rax\n\t"
        "mov $8, %%r10\n\t"        /* sizeof(sigset_t) */
        "syscall"
        : "=a"(result)
        : [syscall] "i"(SYS_rt_sigaction), "D"(sig), "S"(act_ptr),
          "d"(0), "r"(8)
        : "rcx", "r11", "r10", "memory"
    );
#elif defined(__aarch64__)
    __asm__ volatile (
        "mov x8, %[syscall]\n\t"
        "svc #0"
        : "=r"(result)
        : [syscall] "i"(SYS_rt_sigaction), "r"((long)sig), "r"(act_ptr),
          "r"(0), "r"(8)
        : "x0", "x1", "x2", "x3", "x4", "x8", "memory"
    );
#endif

    return result;
}

static long my_sigaltstack(stack_t *ss, stack_t *old_ss)
{
    long result;

#ifdef __x86_64__
    __asm__ volatile (
        "mov %[syscall], %%rax\n\t"
        "syscall"
        : "=a"(result)
        : [syscall] "i"(SYS_sigaltstack), "D"((u64)ss), "S"((u64)old_ss)
        : "rcx", "r11", "memory"
    );
#elif defined(__aarch64__)
    __asm__ volatile (
        "mov x8, %[syscall]\n\t"
        "svc #0"
        : "=r"(result)
        : [syscall] "i"(SYS_sigaltstack), "r"((u64)ss), "r"((u64)old_ss)
        : "x0", "x1", "x8", "memory"
    );
#endif

    return result;
}

static long my_kill(pid_t pid, int sig)
{
    long result;

#ifdef __x86_64__
    __asm__ volatile (
        "mov %[syscall], %%rax\n\t"
        "syscall"
        : "=a"(result)
        : [syscall] "i"(SYS_kill), "D"((long)pid), "S"((long)sig)
        : "rcx", "r11", "memory"
    );
#elif defined(__aarch64__)
    __asm__ volatile (
        "mov x8, %[syscall]\n\t"
        "svc #0"
        : "=r"(result)
        : [syscall] "i"(SYS_kill), "r"((long)pid), "r"((long)sig)
        : "x0", "x1", "x8", "memory"
    );
#endif

    return result;
}

/* Test 1: Basic sigaltstack functionality */
void test_sigaltstack_basic(void)
{
    stack_t ss, old_ss;
    long ret;

    my_write("=== Test 1: Basic sigaltstack ===\n");

    /* Set up alternate stack */
    ss.ss_sp = signal_stack;
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;

    ret = my_sigaltstack(&ss, &old_ss);
    if (ret != 0) {
        my_write("FAIL: sigaltstack returned error\n");
        return;
    }

    /* Verify old stack was disabled */
    if (old_ss.ss_flags & SS_DISABLE) {
        my_write("PASS: Old stack was disabled as expected\n");
    } else {
        my_write("FAIL: Old stack should have been disabled\n");
        return;
    }

    /* Query current stack */
    ret = my_sigaltstack(NULL, &old_ss);
    if (ret != 0) {
        my_write("FAIL: sigaltstack query returned error\n");
        return;
    }

    /* Verify stack was set correctly */
    if (old_ss.ss_sp == signal_stack && old_ss.ss_size == SIGSTKSZ) {
        my_write("PASS: Alternate stack set correctly\n");
    } else {
        my_write("FAIL: Alternate stack not set correctly\n");
    }
}

/* Test 2: Signal handler with SA_ONSTACK */
void test_sa_onstack(void)
{
    long ret;

    my_write("=== Test 2: SA_ONSTACK flag ===\n");

    /* Reset flag */
    signal_stack_used = 0;

    /* Set up signal handler with SA_ONSTACK */
    ret = my_rt_sigaction(SIGUSR1, signal_handler, SA_ONSTACK);
    if (ret != 0) {
        my_write("FAIL: rt_sigaction returned error\n");
        return;
    }

    /* Send signal to ourselves */
    ret = my_kill(getpid(), SIGUSR1);
    if (ret != 0) {
        my_write("FAIL: kill returned error\n");
        return;
    }

    /* Check if signal handler used alternate stack */
    if (signal_stack_used) {
        my_write("PASS: Signal handler used alternate stack\n");
    } else {
        my_write("FAIL: Signal handler did not use alternate stack\n");
    }
}

/* Test 3: Disable alternate stack */
void test_disable_altstack(void)
{
    stack_t ss, old_ss;
    long ret;

    my_write("=== Test 3: Disable alternate stack ===\n");

    /* Disable alternate stack */
    ss.ss_sp = NULL;
    ss.ss_size = 0;
    ss.ss_flags = SS_DISABLE;

    ret = my_sigaltstack(&ss, &old_ss);
    if (ret != 0) {
        my_write("FAIL: sigaltstack disable returned error\n");
        return;
    }

    /* Query current stack */
    ret = my_sigaltstack(NULL, &old_ss);
    if (ret != 0) {
        my_write("FAIL: sigaltstack query returned error\n");
        return;
    }

    /* Verify stack is disabled */
    if (old_ss.ss_flags & SS_DISABLE) {
        my_write("PASS: Alternate stack disabled successfully\n");
    } else {
        my_write("FAIL: Alternate stack should be disabled\n");
    }
}

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║        Signal Stack (sigaltstack) Test                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    /* Run tests */
    test_sigaltstack_basic();
    my_write("\n");

    test_sa_onstack();
    my_write("\n");

    test_disable_altstack();
    my_write("\n");

    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║           Signal Stack Tests Completed                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    return 0;
}