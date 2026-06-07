#include <stdio.h>
#include <unistd.h>

#include "../../include/uapi_sigaction.h"
#include "../../lib/syscall_ids.h"

/*
 * Test sigaltstack: install alt stack, deliver with SA_ONSTACK, disable.
 * Layout/syscall wrappers match test_signal_delivery.c (known good on both arches).
 */

#define SIGUSR1 10

#define SS_ONSTACK  0x00000001
#define SS_DISABLE  0x00000002
#define SA_ONSTACK  0x08000000

#define SIGSTKSZ 8192

typedef struct {
        void *ss_sp;
        int ss_flags;
        unsigned long ss_size;
} stack_t;

static char signal_stack[SIGSTKSZ];
static volatile int signal_stack_used;
static linux_uapi_sigaction_t sigaltstack_act;

static size_t my_strlen(const char *s)
{
        size_t n = 0;

        while (s[n] != '\0') {
                n++;
        }
        return n;
}

static void my_write(const char *str)
{
        size_t len = my_strlen(str);

#if defined(__x86_64__)
        __asm__ volatile("mov $1, %%rax\n\t"
                         "mov $1, %%rdi\n\t"
                         "mov %[buf], %%rsi\n\t"
                         "mov %[len], %%rdx\n\t"
                         "syscall"
                         :
                         : [buf] "r"(str), [len] "r"(len)
                         : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_write;
        register long x0 asm("x0") = 1;
        register const char *x1 asm("x1") = str;
        register long x2 asm("x2") = (long)len;

        __asm__ volatile("svc #0"
                         :
                         : "r"(x8), "r"(x0), "r"(x1), "r"(x2)
                         : "memory");
#endif
}

__attribute__((noreturn)) static void signal_handler(int sig)
{
        char local_var;
        char *sp = &local_var;

        (void)sig;

        if (sp >= signal_stack && sp < signal_stack + SIGSTKSZ) {
                signal_stack_used = 1;
        }

#if defined(__x86_64__)
        __asm__ volatile("mov %[nr], %%rax\n\t"
                         "syscall\n\t"
                         "1: jmp 1b"
                         :
                         : [nr] "i"(SYS_rt_sigreturn)
                         : "rax", "rcx", "r11", "memory");
#elif defined(__aarch64__)
        __asm__ volatile("mov x8, %[nr]\n\t"
                         "svc #0\n\t"
                         "1: b 1b"
                         :
                         : [nr] "i"(SYS_rt_sigreturn)
                         : "x8", "x0", "memory");
#endif
        __builtin_unreachable();
}

static long my_rt_sigaction(int sig, void (*handler)(int), unsigned long flags)
{
        long result;

        sigaltstack_act.sa_handler = handler;
        sigaltstack_act.sa_flags = flags;
        sigaltstack_act.sa_restorer = NULL;
        sigaltstack_act.sa_mask[0] = 0;

#if defined(__x86_64__)
        __asm__ volatile("mov %[syscall], %%rax\n\t"
                         "mov %[size], %%r10\n\t"
                         "syscall"
                         : "=a"(result)
                         : [syscall] "i"(SYS_rt_sigaction), "D"(sig),
                           "S"(&sigaltstack_act), "d"(0),
                           [size] "i"(LINUX_UAPI_SIGSET_SIZE)
                         : "rcx", "r11", "r10", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_rt_sigaction;
        register long x0 asm("x0") = sig;
        register void *x1 asm("x1") = &sigaltstack_act;
        register long x2 asm("x2") = 0;
        register long x3 asm("x3") = LINUX_UAPI_SIGSET_SIZE;

        __asm__ volatile("svc #0"
                         : "=r"(result)
                         : "r"(x8), "r"(x0), "r"(x1), "r"(x2), "r"(x3)
                         : "memory");
#endif
        return result;
}

static long my_sigaltstack(stack_t *ss, stack_t *old_ss)
{
        long result;

#if defined(__x86_64__)
        __asm__ volatile("mov %[syscall], %%rax\n\t"
                         "syscall"
                         : "=a"(result)
                         : [syscall] "i"(SYS_sigaltstack), "D"((unsigned long)ss),
                           "S"((unsigned long)old_ss)
                         : "rcx", "r11", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_sigaltstack;
        register unsigned long x0 asm("x0") = (unsigned long)ss;
        register unsigned long x1 asm("x1") = (unsigned long)old_ss;

        __asm__ volatile("svc #0"
                         : "=r"(result)
                         : "r"(x8), "r"(x0), "r"(x1)
                         : "memory");
#endif
        return result;
}

static long my_kill(pid_t pid, int sig)
{
        long result;

#if defined(__x86_64__)
        __asm__ volatile("mov %[syscall], %%rax\n\t"
                         "syscall"
                         : "=a"(result)
                         : [syscall] "i"(SYS_kill), "D"((long)pid), "S"((long)sig)
                         : "rcx", "r11", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_kill;
        register long x0 asm("x0") = (long)pid;
        register long x1 asm("x1") = (long)sig;

        __asm__ volatile("svc #0"
                         : "=r"(result)
                         : "r"(x8), "r"(x0), "r"(x1)
                         : "memory");
#endif
        return result;
}

static void test_sigaltstack_basic(void)
{
        stack_t ss, old_ss;
        long ret;

        my_write("=== Test 1: Basic sigaltstack ===\n");

        ss.ss_sp = signal_stack;
        ss.ss_size = SIGSTKSZ;
        ss.ss_flags = 0;

        ret = my_sigaltstack(&ss, &old_ss);
        if (ret != 0) {
                my_write("FAIL: sigaltstack returned error\n");
                return;
        }

        if (old_ss.ss_flags & SS_DISABLE) {
                my_write("PASS: Old stack was disabled as expected\n");
        } else {
                my_write("FAIL: Old stack should have been disabled\n");
                return;
        }

        ret = my_sigaltstack(NULL, &old_ss);
        if (ret != 0) {
                my_write("FAIL: sigaltstack query returned error\n");
                return;
        }

        if (old_ss.ss_sp == signal_stack && old_ss.ss_size == SIGSTKSZ) {
                my_write("PASS: Alternate stack set correctly\n");
        } else {
                my_write("FAIL: Alternate stack not set correctly\n");
        }
}

static void test_sa_onstack(void)
{
        long ret;
        pid_t my_pid = getpid();

        my_write("=== Test 2: SA_ONSTACK flag ===\n");

        signal_stack_used = 0;

        ret = my_rt_sigaction(SIGUSR1, signal_handler, SA_ONSTACK);
        if (ret != 0) {
                my_write("FAIL: rt_sigaction returned error\n");
                return;
        }

        ret = my_kill(my_pid, SIGUSR1);
        if (ret != 0) {
                my_write("FAIL: kill returned error\n");
                return;
        }

        if (signal_stack_used) {
                my_write("PASS: Signal handler used alternate stack\n");
        } else {
                my_write("FAIL: Signal handler did not use alternate stack\n");
        }
}

static void test_disable_altstack(void)
{
        stack_t ss, old_ss;
        long ret;

        my_write("=== Test 3: Disable alternate stack ===\n");

        ss.ss_sp = NULL;
        ss.ss_size = 0;
        ss.ss_flags = SS_DISABLE;

        ret = my_sigaltstack(&ss, &old_ss);
        if (ret != 0) {
                my_write("FAIL: sigaltstack disable returned error\n");
                return;
        }

        ret = my_sigaltstack(NULL, &old_ss);
        if (ret != 0) {
                my_write("FAIL: sigaltstack query returned error\n");
                return;
        }

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
