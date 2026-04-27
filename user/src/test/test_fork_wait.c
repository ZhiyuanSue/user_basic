#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
        // 子进程
        printf("Child: Sleeping for a while...\n");
        // 模拟长时间运行
        for (volatile int i = 0; i < 100000000; i++)
                ;
        printf("Child: Exiting\n");
        exit(0);
    } else {
        // 父进程
        printf("Parent: Created child PID=%d\n", pid);

        // 立即检查，子进程应该还在运行
        int status;
        pid_t result = wait4(pid, &status, WNOHANG, NULL);

        if (result == 0) {
            printf("Parent: WNOHANG returned 0 (child still running)\n");
            printf("PASS: WNOHANG works correctly\n");
        } else {
            printf("FAIL: WNOHANG should return 0 when child running\n");
            return 1;
        }

        // 现在真正等待子进程
        printf("Parent: Now waiting for child to finish...\n");
        result = wait4(pid, &status, 0, NULL);

        if (result == pid) {
            printf("Parent: Child exited normally\n");
            return 0;
        } else {
            printf("FAIL: wait4 failed\n");
            return 1;
        }
    }
}

/*
 * 多子进程测试
 * 测试等待任意子进程 (pid == -1)
 */
int test_multiple_children(void)
{
    printf("=== Multiple children test ===\n");

    const int NUM_CHILDREN = 3;
    pid_t children[NUM_CHILDREN];

    // 创建多个子进程
    for (int i = 0; i < NUM_CHILDREN; i++) {
        children[i] = fork();
        if (children[i] < 0) {
            printf("FAIL: fork() failed for child %d\n", i);
            return 1;
        }

        if (children[i] == 0) {
            // 子进程
            printf("Child %d: Running with PID=%d\n", i, getpid());
            // 每个子进程有不同的工作时间
            volatile int delay = (i + 1) * 10000000;
            for (volatile int j = 0; j < delay; j++)
                ;
            printf("Child %d: Exiting with code %d\n", i, (i + 1) * 10);
            exit((i + 1) * 10);
        }
    }

    // 父进程等待所有子进程（顺序不确定）
    printf("Parent: Created %d children\n", NUM_CHILDREN);
    printf("Parent: Waiting for children (any order)...\n");

    int children_reaped = 0;
    while (children_reaped < NUM_CHILDREN) {
        int status;
        pid_t pid = wait4(-1, &status, 0, NULL);

        if (pid > 0) {
            int exit_code = WEXITSTATUS(status);
            printf("Parent: Reaped PID=%d, exit_code=%d\n", pid, exit_code);
            children_reaped++;
        } else {
            printf("FAIL: wait4(-1) failed\n");
            return 1;
        }
    }

    printf("Parent: All %d children reaped\n", NUM_CHILDREN);
    printf("PASS: Multiple children test succeeded\n");
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
