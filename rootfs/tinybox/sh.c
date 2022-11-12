#include "include/sys_call.h"
#include "include/libc.h"

int main(int pargc, char *pargv[])
{
    char buf[128];
    char *argv[5], *s, *p;
    int i, argc, pid;

    printf("sh init success!\n");

    while (1) {
        printf("$ ");
        memset(buf, 0x0, sizeof(buf));
        gets(buf, sizeof(buf));
        i = strlen(buf);
        buf[i - 1] = '\0';
        printf("your input: %s\n", buf);
        continue;
        //目前简单处理，读取cmd，参数最多为3个
        argc = 0;
        s = &buf[0];

        while (*s && *s == ' ')
            s++;

        p = s;

        while ((s = strchr(p, ' ')) != 0) {
            argv[argc++] = p;
            *s = '\0';
            p = s + 1;

            if (argc >= 4)
                break;
        }

        if (argc < 4)
            argv[argc++] = p;

        argv[argc++] = 0;

        pid = fork();

        if (pid == 0) {
            exec(argv[0], argv);
        } else {
            if (strcmp(argv[0], "sleep") == 0)
                continue;

            while (wait() != pid)
                printf("zombie!\n");
        }
    }

    exit();
}
