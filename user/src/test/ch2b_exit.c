#include <stdio.h>
#include <unistd.h>

const int MAGIC = 1234;

/// Test exit syscall with specific exit code
/// Expected: exits with code 1234, no output needed

int main()
{
        exit(MAGIC);
}
