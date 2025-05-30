#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
        int exit_code;
        int fd = open("test\0", O_CREATE | O_WRONLY);
        printf("open OK, fd = %d\n", fd);
        static char str[100] = "hello world!\0";
        int len = strlen(str);
        write(fd, str, len);
        close(fd);
        puts("write over.");
        int pid = fork();
        if (pid == 0) {
                int fd = open("test\0", O_RDONLY);
                str[read(fd, str, len)] = 0;
                puts(str);
                puts("read over.");
                close(fd);
                exit(0);
        }
        wait(&exit_code);
        puts("filetest passed.");
        return 0;
}