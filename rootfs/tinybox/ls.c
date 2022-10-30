#include "include/sys_call.h"
#include "include/libc.h"

char*
fmtname(char *path)
{
    static char buf[FS_NAME_SIZE + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;

    p++;

    // Return blank-padded name.
    if (strlen(p) >= FS_NAME_SIZE)
        return p;

    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', FS_NAME_SIZE - strlen(p));
    return buf;
}

void
ls(char *path)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        printf("ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        printf("ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
        case T_FILE:
            printf("%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
            break;

        case T_DIR:
            if (strlen(path) + 1 + FS_NAME_SIZE + 1 > sizeof buf) {
                printf("ls: path too long\n");
                break;
            }

            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';

            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.idx == 0)
                    continue;

                memmove(p, de.name, FS_NAME_SIZE);
                p[FS_NAME_SIZE] = 0;

                if (stat(buf, &st) < 0) {
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }

                printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
            }

            break;

        default:
            printf("unknown type %d, idx %d\n", &st, st.type, st.ino);
    }

    close(fd);
}

int
main(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        ls(".");
        exit();
    }

    for (i = 1; i < argc; i++)
        ls(argv[i]);

    exit();
}
