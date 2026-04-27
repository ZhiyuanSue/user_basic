#include "errno.h"

/* Thread-local errno storage */
__thread int __errno_value;

int *__errno_location(void)
{
        return &__errno_value;
}
