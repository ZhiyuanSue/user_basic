#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

static volatile int signal_handler_called = 0;
static volatile int received_signal = 0;

/* File-scope act: avoid deep-stack &act landing below the 8-page user stack. */
static linux_uapi_sigaction_t sig_delivery_act;

static size_t my_strlen(const char *s)
{
        size_t n = 0;
        while (s[n] != '\0') {
                n++;
        }
        return n;
}

__attribute__((noreturn)) void my_signal_handler(int sig)
{
        signal_handler_called = 1;
        received_signal = sig;
#if defined(__x86_64__)
        {
                static const char msg[] = "[SIGNAL HANDLER] Received signal\n";
                size_t len = my_strlen(msg);
                (void)sig;
                __asm__ volatile(
                        "mov $1, %%rax\n\t"
                        "mov $1, %%rdi\n\t"
                        "mov %[buf], %%rsi\n\t"
                        "mov %[len], %%rdx\n\t"
                        "syscall"
                        : : [buf] "r"(msg), [len] "r"(len)
                        : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
                /* rt_sigreturn — must not fall through to compiler "ret". */
                __asm__ volatile(
                        "mov %[nr], %%rax\n\t"
                        "syscall\n\t"
                        "1: jmp 1b"
                        :
                        : [nr] "i"(SYS_rt_sigreturn)
                        : "rax", "rcx", "r11", "memory");
        }
#elif defined(__aarch64__)
        {
                static const char msg[] = "[SIGNAL HANDLER] Received signal\n";
                size_t len = my_strlen(msg);
                register long x8 asm("x8") = 64;
                register long x0 asm("x0") = 1;
                register const char *x1 asm("x1") = msg;
                register long x2 asm("x2") = (long)len;
                (void)sig;
                __asm__ volatile("svc #0"
                                 : : "r"(x8), "r"(x0), "r"(x1), "r"(x2)
                                 : "memory");
                __asm__ volatile(
                        "mov x8, %[nr]\n\t"
                        "svc #0\n\t"
                        "1: b 1b"
                        :
                        : [nr] "i"(SYS_rt_sigreturn)
                        : "x8", "x0", "memory");
        }
#else
#error "Unsupported architecture"
#endif
        __builtin_unreachable();
}

static long my_rt_sigaction(int sig, void (*handler)(int))
{
        long result;

        sig_delivery_act.sa_handler = handler;
        sig_delivery_act.sa_flags = 0;
        sig_delivery_act.sa_restorer = NULL;
        sig_delivery_act.sa_mask[0] = 0;

#ifdef __x86_64__
        __asm__ volatile(
                "mov %[syscall], %%rax\n\t"
                "mov %[size], %%r10\n\t"
                "syscall"
                : "=a"(result)
                : [syscall] "i"(SYS_rt_sigaction), "D"(sig),
                  "S"(&sig_delivery_act), "d"(0),
                  [size] "i"(LINUX_UAPI_SIGSET_SIZE)
                : "rcx", "r11", "r10", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_rt_sigaction;
        register long x0 asm("x0") = sig;
        register void *x1 asm("x1") = &sig_delivery_act;
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
        __asm__ volatile(
                "mov x8, %[syscall]\n\t"
                "svc #0"
                : "=r"(result)
                : [syscall] "i"(SYS_kill), "r"((long)pid), "r"((long)sig)
                : "x0", "x1", "x8", "memory");
#else
        result = -1;
#endif

        return result;
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
        register long x8 asm("x8") = 64;
        register long x0 asm("x0") = 1;
        register const char *x1 asm("x1") = str;
        register long x2 asm("x2") = (long)len;
        __asm__ volatile("svc #0"
                         :
                         : "r"(x8), "r"(x0), "r"(x1), "r"(x2)
                         : "memory");
#endif
}

int test_signal_delivery(void)
{
        my_write("=== Test: Signal Delivery ===\n");
        my_write("Setting up signal handler for SIGUSR1...\n");

        long sig_result = my_rt_sigaction(SIGUSR1, my_signal_handler);
        if (sig_result != 0) {
                printf("FAIL: rt_sigaction returned %d\n", (int)sig_result);
                return 1;
        }

        my_write("Signal handler installed successfully\n");

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

        my_write("Sending SIGUSR1 to PID: ");
        my_write(pid_str);

        long kill_result = my_kill(my_pid, SIGUSR1);
        if (kill_result != 0) {
                printf("FAIL: kill() returned %d\n", (int)kill_result);
                return 1;
        }

        my_write("kill() returned success\n");

        if (signal_handler_called) {
                my_write("PASS: Signal handler was called!\n");
                return 0;
        }

        my_write("FAIL: Signal handler was NOT called\n");
        return 1;
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
