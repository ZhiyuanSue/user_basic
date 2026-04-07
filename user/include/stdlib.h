#ifndef __STDLIB_H__
#define __STDLIB_H__

/* For assert_eq diagnostics. */
#include <stdio.h>

void panic(char *);

#define WEXITSTATUS(s) (((s) & 0xff00) >> 8)

#ifndef assert
#define assert(f) \
        if (!(f)) \
        panic("\n --- Assert Fatal ! ---\n")
#endif

#ifndef assert_eq
#define assert_eq(a, b)                                                        \
        do {                                                                   \
                long __a = (long)(a);                                          \
                long __b = (long)(b);                                          \
                if (__a != __b) {                                              \
                        printf("assert_eq failed: %ld != %ld\n", __a, __b);    \
                        panic("\n --- Assert Equal Fatal ! ---\n");           \
                }                                                              \
        } while (0)
#endif

#endif //__STDLIB_H__
