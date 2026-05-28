#ifndef _USER_TLS_H_
#define _USER_TLS_H_

/*
 * Initialize this thread's TLS block (compiler __thread / musl-gcc).
 * Must run once per new thread before using errno or other TLS variables.
 * Called from __start_main; clone children with CLONE_SETTLS rely on the kernel.
 */
void __init_user_tls(void);

#endif
