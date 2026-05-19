#ifndef _SIGNAL_H_
#define _SIGNAL_H_

/* Signal numbers */
#define SIGHUP    1
#define SIGINT    2
#define SIGQUIT   3
#define SIGILL    4
#define SIGTRAP   5
#define SIGABRT   6
#define SIGBUS    7
#define SIGFPE    8
#define SIGKILL   9
#define SIGUSR1   10
#define SIGSEGV   11
#define SIGUSR2   12
#define SIGPIPE   13
#define SIGALRM   14
#define SIGTERM   15
#define SIGSTKFLT 16
#define SIGCHLD   17
#define SIGCONT   18
#define SIGSTOP   19
#define SIGTSTP   20
#define SIGTTIN   21
#define SIGTTOU   22

/* Type for signal handler */
typedef void (*sig_t)(int);

/* Signal constants */
#define SIG_DFL  ((sig_t)0)   /* Default signal handling */
#define SIG_IGN  ((sig_t)1)   /* Ignore signal */
#define SIG_ERR  ((sig_t)-1)  /* Error return */

/* Function declarations */
sig_t signal(int sig, sig_t func);
int kill(pid_t pid, int sig);
int raise(int sig);

/* Setjmp/longjmp for signal handling */
#include <setjmp.h>

#endif // _SIGNAL_H_
