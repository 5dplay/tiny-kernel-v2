#include "include/sys_call.h"
#include "include/libc.h"

char *argv[] = {"sh", 0};

int main(int argc, char *argv[])
{
    int fd;
    int pid, wpid;
    fd = open("/console", 0);

    while (1);

    if (fd < 0) {
        mknod("/console", 1, 1);
        fd = open("/console", 0);
    }

    if (fd != stdin)
        test("sth wrong with open /console");

    dup(stdin);
    dup(stdin);

    while (1) {
        pid = fork();

        if (pid == 0) {
            exec("sh", argv);
            printf("execv sh exit\n");
            exit();
        } else {
            printf("i am parent\n");
            printf("try to wait %d\n", pid);
            wpid = wait();
            printf("wpid = %d\n", wpid);

            while ((wpid = wait()) >= 0 && wpid != pid)
                printf("zombie!\n");
        }

    }
}
