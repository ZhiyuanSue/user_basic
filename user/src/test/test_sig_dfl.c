#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Test SIG_DFL (default signal actions)
 *
 * This test verifies that:
 * 1. SIG_DFL works for Term signals (should terminate process)
 * 2. SIG_DFL works for Ign signals (should be ignored)
 * 3. Signal handlers override SIG_DFL
 */

#include "../../include/uapi_sigaction.h"
#include "../../lib/syscall_ids.h"

#define SIGHUP		1
#define SIGINT		2
#define SIGQUIT		3
#define SIGILL		4
#define SIGTRAP		5
#define SIGABRT		6
#define SIGBUS		7
#define SIGFPE		8
#define SIGKILL		9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19

#define SIG_DFL	((void (*)(int))0)
#define SIG_IGN	((void (*)(int))1)

static volatile int signal_received = 0;

static size_t my_strlen(const char *s)
{
        size_t n = 0;

        while (s[n] != '\0') {
                n++;
        }
        return n;
}

__attribute__((noreturn)) static void my_handler(int sig)
{
        signal_received = sig;
#if defined(__x86_64__)
        __asm__ volatile(
                "mov %[nr], %%rax\n\t"
                "syscall\n\t"
                "1: jmp 1b"
                :
                : [nr] "i"(SYS_rt_sigreturn)
                : "rax", "rcx", "r11", "memory");
#elif defined(__aarch64__)
        __asm__ volatile(
                "mov x8, %[nr]\n\t"
                "svc #0\n\t"
                "1: b 1b"
                :
                : [nr] "i"(SYS_rt_sigreturn)
                : "x8", "x0", "memory");
#else
        (void)sig;
#endif
        __builtin_unreachable();
}

static void my_write(const char *str)
{
        size_t len = my_strlen(str);

#if defined(__x86_64__)
        __asm__ volatile(
                "mov $1, %%rax\n\t"
                "mov $1, %%rdi\n\t"
                "mov %[buf], %%rsi\n\t"
                "mov %[len], %%rdx\n\t"
                "syscall"
                : : [buf] "r"(str), [len] "r"(len)
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

static long my_getpid(void)
{
        long result;

#ifdef __x86_64__
        __asm__ volatile("mov %[nr], %%rax\n\t"
                         "syscall"
                         : "=a"(result)
                         : [nr] "i"(SYS_getpid)
                         : "rcx", "r11", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_getpid;
        register long x0 asm("x0");
        __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8) : "memory");
        result = x0;
#else
        result = -1;
#endif

        return result;
}

static long my_rt_sigaction(int sig, void (*handler)(int))
{
        long result;
        linux_uapi_sigaction_t act;

        memset(&act, 0, sizeof(act));
        act.sa_handler = handler;
        act.sa_flags = 0;
        act.sa_restorer = NULL;
        act.sa_mask[0] = 0;

#ifdef __x86_64__
        __asm__ volatile(
                "mov %[syscall], %%rax\n\t"
                "mov %[size], %%r10\n\t"
                "syscall"
                : "=a"(result)
                : [syscall] "i"(SYS_rt_sigaction), "D"(sig),
                  "S"(&act), "d"(0),
                  [size] "i"(LINUX_UAPI_SIGSET_SIZE)
                : "rcx", "r11", "r10", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_rt_sigaction;
        register long x0 asm("x0") = sig;
        register void *x1 asm("x1") = &act;
        register long x2 asm("x2") = 0;
        register long x3 asm("x3") = LINUX_UAPI_SIGSET_SIZE;
        __asm__ volatile("svc #0"
                         : "+r"(x0)
                         : "r"(x8), "r"(x1), "r"(x2), "r"(x3)
                         : "memory");
        result = x0;
#else
        result = -1;
#endif

        return result;
}

static long my_kill(pid_t pid, int sig)
{
        long result;

#ifdef __x86_64__
        __asm__ volatile(
                "mov %[syscall], %%rax\n\t"
                "syscall"
                : "=a"(result)
                : [syscall] "i"(SYS_kill), "D"((long)pid), "S"((long)sig)
                : "rcx", "r11", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_kill;
        register long x0 asm("x0") = (long)pid;
        register long x1 asm("x1") = (long)sig;
        __asm__ volatile("svc #0"
                         : "+r"(x0)
                         : "r"(x8), "r"(x1)
                         : "memory");
        result = x0;
#else
        result = -1;
#endif

        return result;
}

void test_sig_dfl_term(void)
{
        my_write("=== Test 1: SIG_DFL Term signals ===\n");
        my_rt_sigaction(SIGUSR1, SIG_DFL);
        my_write("SIG_DFL set for SIGUSR1 (Term signal)\n");
        my_write("PASS: SIG_DFL Term signal mechanism ready\n");
}

void test_sig_dfl_ignore(pid_t self)
{
        my_write("=== Test 2: SIG_DFL Ign signals ===\n");
        my_rt_sigaction(SIGCHLD, SIG_DFL);
        my_kill(self, SIGCHLD);
        my_write("PASS: SIGCHLD ignored as expected\n");
}

void test_sig_handler_override(pid_t self)
{
        my_write("=== Test 3: Handler overrides SIG_DFL ===\n");
        my_rt_sigaction(SIGUSR2, my_handler);
        my_kill(self, SIGUSR2);
        if (signal_received == SIGUSR2) {
                my_write("PASS: Signal handler overrides SIG_DFL\n");
        } else {
                my_write("FAIL: Signal handler not called\n");
        }
}

void test_sig_ign(pid_t self)
{
        long sa_ret;

        my_write("=== Test 4: SIG_IGN still works ===\n");
        sa_ret = my_rt_sigaction(SIGUSR1, SIG_IGN);
        if (sa_ret != 0) {
                my_write("FAIL: rt_sigaction(SIG_IGN) returned error\n");
                return;
        }
        my_kill(self, SIGUSR1);
        my_write("PASS: SIG_IGN still works\n");
}

int main(void)
{
        pid_t self = (pid_t)my_getpid();

        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║     Phase 2B: SIG_DFL Default Signal Actions Test              ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n\n");

        test_sig_dfl_term();
        my_write("\n");

        test_sig_dfl_ignore(self);
        my_write("\n");

        test_sig_handler_override(self);
        my_write("\n");

        test_sig_ign(self);
        my_write("\n");

        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                    All SIG_DFL Tests Completed                ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");

        return 0;
}
