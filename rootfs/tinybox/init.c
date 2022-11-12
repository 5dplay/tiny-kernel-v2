#include "include/sys_call.h"
#include "include/libc.h"

char *pargv[] = {"sh", 0};

int main(int argc, char *argv[])
{
    int fd;
    int pid, wpid;
    fd = open("/console", 0);


    if (fd < 0) {
        mknod("/console", 1, 1);
        fd = open("/console", 0);
    }

    if (fd != stdin)
        exit();

    dup(stdin);
    dup(stdin);

    printf("init success!\n");

    do {
        pid = fork();
        printf("fork return! pid = %d\n", pid);

        if (pid == 0) {
            printf("i am child\n");
#if 1
            exec("/sh", pargv);
#endif
            printf("exec sh exit\n");
            exit();

            while (1);
        } else {
            printf("i am parent\n");
            printf("try to wait %d\n", pid);
            wpid = wait();
            printf("wpid = %d\n", wpid);

            while ((wpid = wait()) >= 0 && wpid != pid)
                printf("zombie!\n");
        }
    } while (0);

    while (1);
}
