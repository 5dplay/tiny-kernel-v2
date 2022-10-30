#include "include/sys_call.h"
#include "include/libc.h"

int main(int argc, char *argv[])
{
    int cnt = 0, n;

    if (argc < 2) {
        printf("argc err!\n");
        exit();
    }

    n = atoi(argv[1]);

    while (cnt < 1000) {
        printf("try to sleep %d\n", n);
        sleep(n);
        cnt++;
    }

    exit();
}
