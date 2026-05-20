#ifndef _UAPI_SIGACTION_H_
#define _UAPI_SIGACTION_H_

/*
 * Linux uapi struct sigaction layout (x86_64 / aarch64 lp64).
 * Must match include/linux_compat/signal/signal_types.h in the kernel tree.
 */
typedef struct {
        void (*sa_handler)(int);
        unsigned long sa_flags;
        void (*sa_restorer)(void);
        unsigned long sa_mask[64 / (8 * sizeof(unsigned long))];
} linux_uapi_sigaction_t;

#define LINUX_UAPI_SIGSET_SIZE sizeof(((linux_uapi_sigaction_t *)0)->sa_mask)

#endif /* _UAPI_SIGACTION_H_ */
