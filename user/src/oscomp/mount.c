#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// #define MNTPOINT "./mnt"

static char mntpoint[64] = "./mnt";
static char device[64] = "/dev/vda2";
static const char *fs_type = "vfat";

void test_mount()
{
        TEST_START(__func__);

        printf("Mounting dev:%s to %s\n", device, mntpoint);
        int ret = mount(device, mntpoint, fs_type, 0, NULL);
        if (ret < 0) {
                printf("ERROR: mount syscall not implemented or failed (returned %d)\n", ret);
                TEST_FAIL(__func__);
                return;
        }

        printf("mount return: %d\n", ret);
        printf("mount successfully\n");

        ret = umount(mntpoint);
        if (ret < 0) {
                printf("ERROR: umount syscall failed (returned %d)\n", ret);
                TEST_FAIL(__func__);
                return;
        }

        printf("umount return: %d\n", ret);
        TEST_PASS(__func__);
}

int main(int argc, char *argv[])
{
        if (argc >= 2) {
                strcpy(device, argv[1]);
        }

        if (argc >= 3) {
                strcpy(mntpoint, argv[2]);
        }

        test_mount();
        return 0;
}
