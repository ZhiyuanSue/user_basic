#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Phase 2B: Signal Delivery Test
 *
 * This test verifies that:
 * 1. We can set up signal handlers via rt_sigaction
 * 2. We can send signals to ourselves via kill
 * 3. Signal handlers are actually called (delivery mechanism works)
 */

/* Include architecture-specific syscall numbers from user payload lib */
#include "../lib/syscall_ids.h"

/* Standard signal definitions (from Linux signal.h) */
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

/* Global test counters */
static volatile int signal_handler_called = 0;
static volatile int received_signal = 0;

/* Signal handler function */
void my_signal_handler(int sig)
{
    signal_handler_called = 1;
    received_signal = sig;
    /* Write syscall for output - multi-architecture support */
#if defined(__x86_64__)
    __asm__ volatile (
        "mov $1, %%rax\n\t"   /* SYS_write */
        "mov $1, %%rdi\n\t"   /* stdout */
        "syscall"
        : : "S"("[SIGNAL HANDLER] Received signal"), "d"(sig)
        : "rax", "rdi", "rcx", "r11", "memory"
    );
#elif defined(__aarch64__)
    register long x8 asm("x8") = 64;   /* SYS_write for aarch64 */
    register long x0 asm("x0") = 1;    /* stdout */
    register const char *x1 asm("x1") = "[SIGNAL HANDLER] Received signal";
    register long x2 asm("x2") = sig;
    __asm__ volatile (
        "svc #0"
        : : "r"(x8), "r"(x0), "r"(x1), "r"(x2)
        : "memory"
    );
#else
#error "Unsupported architecture"
#endif
}

/* Syscall wrappers */
static long my_rt_sigaction(int sig, void (*handler)(int))
{
    struct {
        unsigned long sig[64 / (8 * sizeof(unsigned long))];
    } mask = {0}; /* Initialize to zero */

    long result;

    #ifdef __x86_64__
    size_t sigsetsize = sizeof(mask);
    __asm__ volatile (
        "mov %[syscall], %%rax\n\t"
        "mov %[size], %%r10\n\t"
        "syscall"
        : "=a"(result)
        : [syscall] "i"(SYS_rt_sigaction), "D"(sig), "S"(handler),
          "d"(0), [size] "r"(sigsetsize)
        : "rcx", "r11", "r10", "memory"
    );
    #elif defined(__aarch64__)
    /* AArch64 uses different registers for syscall parameters */
    __asm__ volatile (
        "mov x8, %[syscall]\n\t"
        "svc #0"
        : "=r"(result)
        : [syscall] "i"(SYS_rt_sigaction), "r"(sig), "r"(handler),
          "r"(0), "r"(0)
        : "x0", "x1", "x2", "x3", "x4", "x8", "memory"
    );
    #else
    result = -1; /* Not implemented */
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
    #else
    result = -1; /* Not implemented */
    #endif

    return result;
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

int test_signal_delivery(void)
{
    /* Write syscall for output - multi-architecture support */
    my_write("=== Test: Signal Delivery ===\n");

    /* Set up signal handler */
    my_write("Setting up signal handler for SIGUSR1...\n");

    long sig_result = my_rt_sigaction(SIGUSR1, my_signal_handler);
    if (sig_result != 0) {
        my_write("FAIL: Could not set signal handler\n");
        return 1;
    }

    my_write("Signal handler installed successfully\n");

    /* Get PID */
    pid_t my_pid = getpid();
    char pid_str[20];
    int pid_len = 0;
    int temp = my_pid;
    if (temp == 0) {
        pid_str[pid_len++] = '0';
    } else {
        char buf[20];
        int buf_len = 0;
        while (temp > 0) {
            buf[buf_len++] = '0' + (temp % 10);
            temp /= 10;
        }
        while (buf_len > 0) {
            pid_str[pid_len++] = buf[--buf_len];
        }
    }
    pid_str[pid_len] = '\n';

    /* Send signal to ourselves */
    my_write("Sending SIGUSR1 to PID: ");
    my_write(pid_str);

    long kill_result = my_kill(my_pid, SIGUSR1);
    if (kill_result != 0) {
        my_write("FAIL: kill() returned error\n");
        return 1;
    }

    my_write("kill() returned success\n");

    /* Check if signal handler was called */
    if (signal_handler_called) {
        my_write("PASS: Signal handler was called!\n");
        return 0;
    } else {
        my_write("FAIL: Signal handler was NOT called\n");
        return 1;
    }
}

int main(void)
{
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     Phase 2B: Signal Delivery Test                            ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    int test_result = test_signal_delivery();

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      Test Result                             ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    if (test_result == 0) {
        printf("║  ✅ PASSED: Signal delivery mechanism works!                ║\n");
    } else {
        printf("║  ❌ FAILED: Signal delivery mechanism not working          ║\n");
    }

    printf("╚══════════════════════════════════════════════════════════════╝\n");

    return test_result;
}