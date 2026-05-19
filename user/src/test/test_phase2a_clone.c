#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Phase 2A线程控制syscall测试 - 多架构支持版本
 *
 * 测试内容：
 * 1. set_tid_address (使用正确的syscall号码)
 * 2. set_robust_list (使用正确的syscall号码)
 * 3. fork验证基础进程创建
 * 4. 基础功能验证
 */

/* Clone flags (from kernel headers) */
#ifndef CLONE_VM
#define CLONE_VM             0x00000100
#define CLONE_FS             0x00000200
#define CLONE_FILES          0x00000400
#define CLONE_SIGHAND        0x00000800
#define CLONE_THREAD         0x00010000
#define CLONE_SETTLS         0x00080000
#define CLONE_PARENT_SETTID  0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000
#define CLONE_CHILD_SETTID   0x01000000
#endif

/* Stack size for child threads */
#define STACK_SIZE (1024 * 1024)

/* Global test counters */
static int set_tid_address_test_passed = 0;
static int set_robust_list_test_passed = 0;
static int fork_test_passed = 0;
static int basic_test_passed = 0;

/* Define WIFEXITED if not available (avoid duplicate definition) */
#ifndef WIFEXITED
#define WIFEXITED(status) (((status) & 0x7f) == 0)
#define WEXITSTATUS(status) (((status) >> 8) & 0xff)
#endif

/* Include architecture-specific syscall numbers from user payload lib */
#include "../lib/syscall_ids.h"

/*
 * 测试1：set_tid_address (使用正确的syscall号码)
 */
int test_set_tid_address(void)
{
    printf("=== Test 1: set_tid_address ===\n");

    int tid_pointer = 0;
    long tid;

    /* 使用架构相关的syscall号码 */
    #ifdef __x86_64__
    __asm__ volatile (
        "mov %[syscall], %%rax\n\t"
        "syscall"
        : "=a"(tid)
        : [syscall] "i"(SYS_set_tid_address), "D"(&tid_pointer)
        : "rcx", "r11", "memory"
    );
    #elif defined(__aarch64__)
    __asm__ volatile (
        "mov x8, %2\n\t"
        "mov x0, %1\n\t"
        "svc #0\n\t"
        "mov %0, x0"
        : "=r"(tid)
        : "r"(&tid_pointer), "i"(SYS_set_tid_address)
        : "x8", "x0", "memory"
    );
    #else
    #error "Unsupported architecture for set_tid_address test"
    #endif

    printf("set_tid_address returned TID=%ld\n", tid);
    printf("Expected TID=%d (gettid())\n", gettid());

    if (tid == gettid()) {
        printf("PASS: set_tid_address returned correct TID\n");
        set_tid_address_test_passed = 1;
        return 0;
    } else {
        printf("FAIL: set_tid_address returned wrong TID\n");
        return 1;
    }
}

/*
 * 测试2：set_robust_list (使用正确的syscall号码)
 */
int test_set_robust_list(void)
{
    printf("=== Test 2: set_robust_list ===\n");

    struct robust_list_head {
        void *next;
        int *futex_pending;
        unsigned int futex_pending_owner;
    } list_head;

    memset(&list_head, 0, sizeof(list_head));

    long result;

    /* 使用架构相关的syscall号码 */
    #ifdef __x86_64__
    __asm__ volatile (
        "mov %[syscall], %%rax\n\t"
        "syscall"
        : "=a"(result)
        : [syscall] "i"(SYS_set_robust_list), "D"(&list_head), "S"(sizeof(list_head))
        : "rcx", "r11", "memory"
    );
    #elif defined(__aarch64__)
    __asm__ volatile (
        "mov x8, %3\n\t"
        "mov x0, %1\n\t"
        "mov x1, %2\n\t"
        "svc #0\n\t"
        "mov %0, x0"
        : "=r"(result)
        : "r"(&list_head), "r"(sizeof(list_head)), "i"(SYS_set_robust_list)
        : "x8", "x0", "x1", "memory"
    );
    #else
    #error "Unsupported architecture for set_robust_list test"
    #endif

    printf("set_robust_list returned %ld\n", result);

    if (result == 0) {
        printf("PASS: set_robust_list succeeded\n");
        set_robust_list_test_passed = 1;
        return 0;
    } else {
        printf("FAIL: set_robust_list failed with error %ld\n", result);
        return 1;
    }
}

/*
 * 测试3：使用fork验证基础进程创建
 */
int test_fork_process(void)
{
    printf("=== Test 3: fork-based process creation ===\n");

    pid_t pid = fork();
    if (pid < 0) {
        printf("FAIL: fork() failed\n");
        return 1;
    }

    if (pid == 0) {
        /* Child process */
        printf("Child: Running with PID=%d, TID=%d\n", getpid(), gettid());
        printf("Child: Exiting with code 0\n");
        exit(0);
    } else {
        /* Parent process */
        printf("Parent: Created child PID=%d\n", pid);

        int status;
        pid_t result = waitpid(pid, &status, 0);

        if (result == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("PASS: Child process exited successfully\n");
            fork_test_passed = 1;
        } else {
            printf("FAIL: Child process exit failed\n");
        }

        return (fork_test_passed ? 0 : 1);
    }
}

/*
 * 测试4：基础功能验证
 */
int test_basic_functionality(void)
{
    printf("=== Test 4: Basic functionality check ===\n");

    /* 测试基本的系统调用是否工作 */
    pid_t pid = getpid();
    pid_t tid = gettid();

    printf("Current PID=%d, TID=%d\n", pid, tid);

    if (pid > 0 && tid > 0) {
        printf("PASS: Basic syscalls working\n");
        basic_test_passed = 1;
        return 0;
    } else {
        printf("FAIL: Basic syscalls not working\n");
        return 1;
    }
}

/*
 * 主测试函数
 */
int main(void)
{
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     Phase 2A: Thread Control Syscall Tests                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    int total_tests = 4;
    int passed_tests = 0;

    /* Run tests */
    if (test_basic_functionality() == 0) {
        passed_tests++;
    }
    printf("\n");

    if (test_set_tid_address() == 0) {
        passed_tests++;
    }
    printf("\n");

    if (test_set_robust_list() == 0) {
        passed_tests++;
    }
    printf("\n");

    if (test_fork_process() == 0) {
        passed_tests++;
    }
    printf("\n");

    /* Print summary */
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      Test Summary                           ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %2d                                            ║\n", total_tests);
    printf("║  Passed:       %2d                                            ║\n", passed_tests);
    printf("║  Failed:       %2d                                            ║\n", total_tests - passed_tests);
    printf("║  Success Rate: %2d%%                                           ║\n",
           (passed_tests * 100) / total_tests);
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    return (passed_tests == total_tests) ? 0 : 1;
}
