#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

#include "stddef.h"

/* Additional mapping flags beyond those in stddef.h */
#define MAP_SHARED_VALIDATE 0x03 /* MAP_SHARED | MAP_PRIVATE check: both supported */
#define MAP_TYPE        0x0f    /* Mask for type of mapping */
#define MAP_ANONYMOUS   0x20    /* Don't use a file */
#define MAP_GROWSDOWN   0x100   /* Stack-like segment */

/* MREMAP flags */
#define MREMAP_MAYMOVE  0x1     /* Allow moving the mapping */

/* Function declarations */
void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset);
int munmap(void *addr, size_t length);
int mprotect(void *addr, size_t len, int prot);
void *mremap(void *old_addr, size_t old_len, size_t new_len, int flags, ...);

#endif // _SYS_MMAN_H_
