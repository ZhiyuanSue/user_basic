#include <stdio.h>
#include <unistd.h>
#include "stddef.h"

/* Declare wait4 if not in headers */
extern pid_t wait4(pid_t pid, int *status, int options, void *rusage);

/* Helper macros for wait4 status parsing */
#ifndef WIFEXITED
#define WIFEXITED(status) (((status) & 0x7f) == 0)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(status) (((status) & 0xff00) >> 8)
#endif
#ifndef WIFSIGNALED
#define WIFSIGNALED(status) (((status) & 0x7f) != 0 && (((status) & 0x7f) != 0x7f))
#endif
#ifndef WTERMSIG
#define WTERMSIG(status) ((status) & 0x7f)
#endif

#define WNOHANG 1

/* Test getpid and gettid */
void test_getpid_gettid(void)
{
        TEST_START("getpid_gettid");

        pid_t pid = getpid();
        pid_t tid = gettid();

        printf("[INFO] getpid() = %d\n", pid);
        printf("[INFO] gettid() = %d\n", tid);

        if (pid > 0 && tid > 0) {
                printf("[PASS] getpid and gettid returned positive values\n");
        } else {
                printf("[FAIL] getpid or gettid returned invalid value\n");
        }

        TEST_END("getpid_gettid");
}

/* Test fork and wait4 */
void test_fork_wait4(void)
{
        TEST_START("fork_wait4");

        pid_t pid = fork();

        if (pid < 0) {
                printf("[FAIL] fork failed (returned %d)\n", pid);
                TEST_END("fork_wait4");
                return;
        }

        if (pid == 0) {
                /* Child process */
                pid_t my_pid = getpid();
                pid_t my_ppid = getppid();

                /* Child self-check */
                if (my_pid <= 0) {
                        printf("[FAIL] Child got invalid PID %d\n", my_pid);
                        exit(1);
                }

                if (my_ppid <= 0) {
                        printf("[FAIL] Child got invalid PPID %d\n", my_ppid);
                        exit(1);
                }

                printf("[INFO] Child: PID=%d, PPID=%d\n", my_pid, my_ppid);
                printf("[INFO] Child: exiting with code 42\n");
                exit(42);
        } else {
                /* Parent process */
                printf("[INFO] Parent: child PID=%d\n", pid);

                int status;
                pid_t waited = wait4(pid, &status, 0, NULL);

                if (waited != pid) {
                        printf("[FAIL] wait4 returned wrong PID (expected %d, got %d)\n",
                               pid, waited);
                        TEST_END("fork_wait4");
                        return;
                }

                printf("[INFO] Parent: wait4 returned child PID %d\n", waited);
                printf("[INFO] Parent: raw exit status = 0x%x\n", (unsigned int)status);

                if (!WIFEXITED(status)) {
                        if (WIFSIGNALED(status)) {
                                printf("[FAIL] Child terminated by signal %d\n", WTERMSIG(status));
                        } else {
                                printf("[FAIL] Child terminated abnormally (status=0x%x)\n",
                                       (unsigned int)status);
                        }
                        TEST_END("fork_wait4");
                        return;
                }

                int exit_code = WEXITSTATUS(status);
                printf("[INFO] Parent: exit code = %d\n", exit_code);

                if (exit_code == 42) {
                        printf("[PASS] fork and wait4 worked correctly\n");
                } else {
                        printf("[FAIL] wait4 returned wrong exit code (expected 42, got %d)\n",
                               exit_code);
                }
        }

        TEST_END("fork_wait4");
}

/* Test WNOHANG option */
void test_wait4_wnohang(void)
{
        TEST_START("wait4_wnohang");

        pid_t pid = fork();

        if (pid < 0) {
                printf("[FAIL] fork failed (returned %d)\n", pid);
                TEST_END("wait4_wnohang");
                return;
        }

        if (pid == 0) {
                /* Child: sleep a bit then exit */
                printf("[INFO] Child: sleeping before exit\n");
                for (volatile int i = 0; i < 1000000; i++);
                printf("[INFO] Child: exiting\n");
                exit(0);
        } else {
                /* Parent: try WNOHANG */
                printf("[INFO] Parent: child PID=%d\n", pid);

                int status;
                pid_t waited = wait4(pid, &status, WNOHANG, NULL);

                if (waited == 0) {
                        printf("[INFO] Parent: WNOHANG returned 0 (child not exited yet)\n");
                        printf("[PASS] WNOHANG non-blocking wait works\n");
                } else if (waited == pid) {
                        /* Child already exited */
                        if (WIFEXITED(status)) {
                                printf("[INFO] Parent: child already exited (exit=%d)\n",
                                       WEXITSTATUS(status));
                        } else {
                                printf("[INFO] Parent: child terminated abnormally (status=0x%x)\n",
                                       (unsigned int)status);
                        }
                        printf("[INFO] WNOHANG returned child PID because child already exited\n");
                } else {
                        printf("[FAIL] WNOHANG returned unexpected value: %d\n", waited);
                        TEST_END("wait4_wnohang");
                        return;
                }

                /* Now wait for real to clean up */
                waited = wait4(pid, &status, 0, NULL);
                if (waited == pid) {
                        printf("[INFO] Parent: final wait4 succeeded (exit=%d)\n",
                               WIFEXITED(status) ? WEXITSTATUS(status) : -1);
                } else {
                        printf("[FAIL] final wait4 returned wrong PID (expected %d, got %d)\n",
                               pid, waited);
                }
        }

        TEST_END("wait4_wnohang");
}

/* Test multiple forks */
void test_multiple_forks(void)
{
        TEST_START("multiple_forks");

        pid_t child1 = fork();
        if (child1 == 0) {
                printf("[INFO] Child1: exiting with code 1\n");
                exit(1);
        }

        pid_t child2 = fork();
        if (child2 == 0) {
                printf("[INFO] Child2: exiting with code 2\n");
                exit(2);
        }

        if (child1 > 0 && child2 > 0) {
                printf("[INFO] Parent: child1=%d, child2=%d\n", child1, child2);

                int status;
                pid_t waited;
                int child_count = 0;

                /* Wait for child1 */
                waited = wait4(child1, &status, 0, NULL);
                if (waited == child1) {
                        if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
                                printf("[INFO] Parent: child1 exited with code 1 (PASS)\n");
                                child_count++;
                        } else {
                                printf("[INFO] Parent: child1 unexpected status=0x%x\n",
                                       (unsigned int)status);
                        }
                } else {
                        printf("[FAIL] wait4 for child1 returned wrong PID (expected %d, got %d)\n",
                               child1, waited);
                }

                /* Wait for child2 */
                waited = wait4(child2, &status, 0, NULL);
                if (waited == child2) {
                        if (WIFEXITED(status) && WEXITSTATUS(status) == 2) {
                                printf("[INFO] Parent: child2 exited with code 2 (PASS)\n");
                                child_count++;
                        } else {
                                printf("[INFO] Parent: child2 unexpected status=0x%x\n",
                                       (unsigned int)status);
                        }
                } else {
                        printf("[FAIL] wait4 for child2 returned wrong PID (expected %d, got %d)\n",
                               child2, waited);
                }

                /* Verify both children collected */
                if (child_count == 2) {
                        printf("[PASS] Both children collected successfully\n");
                } else {
                        printf("[FAIL] Only %d children collected (expected 2)\n", child_count);
                }

                /* Final cleanup: ensure no zombies */
                int final_status;
                int zombie_count = 0;
                while ((waited = wait4(-1, &final_status, WNOHANG, NULL)) > 0) {
                        printf("[WARNING] Found unexpected child PID=%d, status=0x%x\n",
                               waited, (unsigned int)final_status);
                        zombie_count++;
                        /* Reap it */
                        waited = wait4(waited, &final_status, 0, NULL);
                }

                if (zombie_count > 0) {
                        printf("[WARNING] Cleaned up %d zombie processes\n", zombie_count);
                }

                if (zombie_count == 0 && child_count == 2) {
                        printf("[PASS] No zombie processes remaining\n");
                }
        }

        TEST_END("multiple_forks");
}

int main(void)
{
        printf("========================================\n");
        printf("PHASE 1 SYSCALL TESTS\n");
        printf("========================================\n");

        test_getpid_gettid();
        printf("\n");

        test_fork_wait4();
        printf("\n");

        test_wait4_wnohang();
        printf("\n");

        test_multiple_forks();
        printf("\n");

        printf("========================================\n");
        printf("ALL TESTS COMPLETED\n");
        printf("========================================\n");

        return 0;
}
