#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../lib/syscall_ids.h"

/* Wait4 options */
#ifndef WNOHANG
#define WNOHANG    0x00000001  /* Don't block if no child exited */
#endif

#ifndef WUNTRACED
#define WUNTRACED  0x00000002  /* Report stopped children */
#endif

#ifndef WCONTINUED
#define WCONTINUED 0x00000008  /* Report continued children */
#endif

#ifndef WIFEXITED
#define WIFEXITED(status) (((status) & 0x7f) == 0)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#endif

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
                         : : [buf] "r"(str), [len] "r"(len)
                         : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_write;
        register long x0 asm("x0") = 1;
        register const char *x1 asm("x1") = str;
        register long x2 asm("x2") = (long)len;
        __asm__ volatile("svc #0"
                         : : "r"(x8), "r"(x0), "r"(x1), "r"(x2)
                         : "memory");
#endif
}

static char *append_str(char *p, const char *s)
{
        while (*s != '\0') {
                *p++ = *s++;
        }
        return p;
}

static char *append_u32(char *p, unsigned v)
{
        char tmp[10];
        int n = 0;
        int i;

        if (v == 0) {
                *p++ = '0';
                return p;
        }

        while (v > 0) {
                tmp[n++] = (char)('0' + (v % 10));
                v /= 10;
        }

        for (i = n - 1; i >= 0; i--) {
                *p++ = tmp[i];
        }

        return p;
}

static void write_child_run(int idx, int pid)
{
        char buf[96];
        char *p = buf;

        p = append_str(p, "Child ");
        p = append_u32(p, (unsigned)idx);
        p = append_str(p, ": Running with PID=");
        p = append_u32(p, (unsigned)pid);
        p = append_str(p, "\n");
        *p = '\0';
        my_write(buf);
}

static void write_child_exit(int idx, int code)
{
        char buf[96];
        char *p = buf;

        p = append_str(p, "Child ");
        p = append_u32(p, (unsigned)idx);
        p = append_str(p, ": Exiting with code ");
        p = append_u32(p, (unsigned)code);
        p = append_str(p, "\n");
        *p = '\0';
        my_write(buf);
}

static void write_parent_created(int count)
{
        char buf[64];
        char *p = buf;

        p = append_str(p, "Parent: Created ");
        p = append_u32(p, (unsigned)count);
        p = append_str(p, " children\n");
        *p = '\0';
        my_write(buf);
}

static void write_parent_reaped(int pid, int exit_code)
{
        char buf[96];
        char *p = buf;

        p = append_str(p, "Parent: Reaped PID=");
        p = append_u32(p, (unsigned)pid);
        p = append_str(p, ", exit_code=");
        p = append_u32(p, (unsigned)exit_code);
        p = append_str(p, "\n");
        *p = '\0';
        my_write(buf);
}

static void write_parent_all_reaped(int count)
{
        char buf[64];
        char *p = buf;

        p = append_str(p, "Parent: All ");
        p = append_u32(p, (unsigned)count);
        p = append_str(p, " children reaped\n");
        *p = '\0';
        my_write(buf);
}

/*
 * 基础fork+wait4测试
 * 测试父子进程基本通信
 */
int test_fork_wait_basic(void)
{
    printf("=== Basic fork/wait4 test ===\n");

    pid_t pid = fork();
    if (pid < 0) {
        printf("FAIL: fork() failed\n");
        return 1;
    }

    if (pid == 0) {
        // 子进程
        printf("Child: Running with PID=%d\n", getpid());
        printf("Child: Doing some work...\n");
        // 模拟工作
        for (int i = 0; i < 1000000; i++)
                ;
        printf("Child: Exiting with code 42\n");
        exit(42);
    } else {
        // 父进程
        printf("Parent: Created child PID=%d\n", pid);
        printf("Parent: Waiting for child...\n");

        int status;
        pid_t result = wait4(pid, &status, 0, NULL);

        printf("Parent: wait4 returned PID=%d\n", result);
        printf("Parent: Child exit status=%d\n", status);

        if (result == pid) {
            int exit_code = WEXITSTATUS(status);
            printf("Parent: Child exited with code %d\n", exit_code);

            if (exit_code == 42) {
                printf("PASS: Got expected exit code\n");
                return 0;
            } else {
                printf("FAIL: Expected exit code 42, got %d\n", exit_code);
                return 1;
            }
        } else {
            printf("FAIL: wait4 returned wrong PID\n");
            return 1;
        }
    }
}

static void yield_cpu(void)
{
#if defined(__x86_64__)
        __asm__ volatile("mov $24, %%rax\n\t"
                         "syscall"
                         :
                         :
                         : "rax", "rcx", "r11", "memory");
#elif defined(__aarch64__)
        register long x8 asm("x8") = SYS_sched_yield;
        register long x0 asm("x0") = 0;
        __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8) : "memory");
#endif
}

/*
 * WNOHANG测试
 * 测试非阻塞等待语义
 */
int test_wnohang(void)
{
    printf("=== WNOHANG test ===\n");

    pid_t pid = fork();
    if (pid < 0) {
        printf("FAIL: fork() failed\n");
        return 1;
    }

    if (pid == 0) {
        /*
         * Yield first so the parent can reach WNOHANG while we are still
         * alive. Avoid huge busy-loops: on SMP + QEMU they pin vCPUs and
         * virtual time/timer IRQs stall (looks like a hang until keyboard).
         */
        printf("Child: Sleeping for a while...\n");
        for (int i = 0; i < 256; i++) {
                yield_cpu();
        }
        printf("Child: Exiting\n");
        exit(0);
    } else {
        int status;
        pid_t result;

        /* Parent: WNOHANG immediately after fork (before slow printf). */
        result = wait4(pid, &status, WNOHANG, NULL);

        printf("Parent: Created child PID=%d\n", pid);

        if (result == 0) {
            printf("Parent: WNOHANG returned 0 (child still running)\n");
            printf("PASS: WNOHANG works correctly\n");

            printf("Parent: Now waiting for child to finish...\n");
            result = wait4(pid, &status, 0, NULL);
            if (result == pid) {
                printf("Parent: Child exited normally\n");
                return 0;
            }
            printf("FAIL: wait4 failed\n");
            return 1;
        }

        if (result == pid) {
            printf("Parent: WNOHANG reaped already-exited child (exit=%d)\n",
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1);
            printf("PASS: WNOHANG reaped exited child\n");
            return 0;
        }

        printf("FAIL: WNOHANG returned unexpected value: %d\n", (int)result);
        return 1;
    }
}

/*
 * 多子进程测试
 * 测试等待任意子进程 (pid == -1)
 */
static void multiple_child_entry(int idx)
{
        const int code = (idx + 1) * 10;

        write_child_run(idx, (int)getpid());
        for (int j = 0; j < (idx + 1) * 64; j++) {
                yield_cpu();
        }
        write_child_exit(idx, code);
        exit(code);
}

int test_multiple_children(void)
{
    printf("=== Multiple children test ===\n");

    const int NUM_CHILDREN = 3;
    pid_t children[NUM_CHILDREN];
    int got_exit_10 = 0;
    int got_exit_20 = 0;
    int got_exit_30 = 0;

    for (int i = 0; i < NUM_CHILDREN; i++) {
        children[i] = -1;
    }

#define SPAWN_ONE_CHILD(idx)                                                   \
        do {                                                                   \
                pid_t pid = fork();                                            \
                if (pid == 0) {                                                \
                        multiple_child_entry(idx);                             \
                        __builtin_unreachable();                               \
                }                                                              \
                if (pid < 0) {                                                 \
                        printf("FAIL: fork() failed for child %d\n", (idx));  \
                        return 1;                                              \
                }                                                              \
                children[(idx)] = pid;                                           \
        } while (0)

    SPAWN_ONE_CHILD(0);
    SPAWN_ONE_CHILD(1);
    SPAWN_ONE_CHILD(2);

#undef SPAWN_ONE_CHILD

    write_parent_created(NUM_CHILDREN);
    my_write("Parent: Waiting for children (any order)...\n");

    int children_reaped = 0;
    while (children_reaped < NUM_CHILDREN) {
        int status;
        pid_t pid = wait4(-1, &status, 0, NULL);

        if (pid > 0) {
            int exit_code = WEXITSTATUS(status);

            switch (exit_code) {
            case 10:
                    got_exit_10 = 1;
                    break;
            case 20:
                    got_exit_20 = 1;
                    break;
            case 30:
                    got_exit_30 = 1;
                    break;
            default:
                    printf("FAIL: unexpected exit_code %d from pid %d\n",
                           exit_code, (int)pid);
                    return 1;
            }

            write_parent_reaped((int)pid, exit_code);
            children_reaped++;
        } else {
            printf("FAIL: wait4(-1) failed\n");
            return 1;
        }
    }

    if (!got_exit_10 || !got_exit_20 || !got_exit_30) {
            printf("FAIL: missing exit codes (10=%d 20=%d 30=%d)\n",
                   got_exit_10, got_exit_20, got_exit_30);
            return 1;
    }

    write_parent_all_reaped(NUM_CHILDREN);
    my_write("PASS: Multiple children test succeeded\n");
    return 0;
}

/*
 * 主测试入口
 */
int main(void)
{
    printf("Starting fork/wait4 tests...\n\n");

    int failures = 0;

    if (test_fork_wait_basic() != 0) {
        printf("\n*** test_fork_wait_basic FAILED ***\n\n");
        failures++;
    } else {
        printf("\n*** test_fork_wait_basic PASSED ***\n\n");
    }

    if (test_wnohang() != 0) {
        printf("\n*** test_wnohang FAILED ***\n\n");
        failures++;
    } else {
        printf("\n*** test_wnohang PASSED ***\n\n");
    }

    if (test_multiple_children() != 0) {
        printf("\n*** test_multiple_children FAILED ***\n\n");
        failures++;
    } else {
        printf("\n*** test_multiple_children PASSED ***\n\n");
    }

    printf("=== Test Summary ===\n");
    printf("Total tests: 3\n");
    printf("Passed: %d\n", 3 - failures);
    printf("Failed: %d\n", failures);

    return failures > 0 ? 1 : 0;
}
