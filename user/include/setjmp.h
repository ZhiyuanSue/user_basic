#ifndef _SETJMP_H_
#define _SETJMP_H_

#include <stddef.h>

/* Buffer for saving register state */
#define _JBLEN  64
typedef struct {
        unsigned long long buf[_JBLEN];
} jmp_buf[1];

/* sigjmp_buf is same as jmp_buf but saves signal mask */
typedef jmp_buf sigjmp_buf;

/* Function declarations */
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

/* Signal versions that save/restore signal mask */
int sigsetjmp(sigjmp_buf env, int savemask);
void siglongjmp(sigjmp_buf env, int val);

#endif // _SETJMP_H_
