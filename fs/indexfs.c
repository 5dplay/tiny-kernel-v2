#include "indexfs.h"
#include "string.h"

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char*
skipelem(char *path, char *name)
{
    char *s;
    int len;

    while (*path == '/')
        path++;

    if (*path == 0)
        return 0;

    s = path;

    while (*path != '/' && *path != 0)
        path++;

    len = path - s;

    if (len >= DNAME)
        memmove(name, s, DNAME);
    else {
        memmove(name, s, len);
        name[len] = 0;
    }

    while (*path == '/')
        path++;

    return path;
}

static struct inode* namex(char *path, int find_parent, char *name)
{
    struct inode *ip, *next;

    /*
        1. 获取内存inode节点
        2. 逐层取目录名
        3. 如果非目录类型,则结束;如果是查询父目录的,并且无后续项,则结束
        4. 获取下一个目录项
    */

    /*
    if (*path == '/')
        ip = cur_proc()->root;
    else
        ip = cur_proc()->cwd;
    idup(ip);
    */

    while ((path = skipelem(path, name)) != NULL) {
        //FIXME: 记得加锁
        if (ip->type != T_DIR) {
            goto free_ip;
        }

        if (find_parent && *path == '\0') {
            return ip;
        }

        next = lookupi(ip->superb, ip, name, strlen(name));

        if (next == NULL)
            goto free_ip;

        //FIXME: 记得解锁
        ip = next;
    }

    return ip;

free_ip:
    //FIXME: 待完成
    return NULL;
}

struct inode* namei(char *path)
{
    char name[DNAME];
    return namex(path, 0, name);
}

struct inode* namei_parent(char *path, char *name)
{
    return namex(path, 1, name);
}

struct inode* do_createi(char *path, u16 type, u16 major, u16 minor)
{
    struct inode *ip, *dp;
    char name[DNAME];
    //FIXME: 记得加锁和解锁

    dp = namei_parent(path, name);

    if (dp == NULL)
        return NULL;

    ip = lookupi(dp->superb, dp, name, strlen(name));

    if (ip != NULL) {
        if (type == T_FILE && ip->type == T_FILE)
            return ip;
        else
            return NULL;
    }

    ip = alloci(dp->superb, type, major, minor);

    if (type == T_DIR) {
        data_linki(ip->superb, ip, ip, ".", sizeof("."));
        data_linki(ip->superb, ip, dp, "..", sizeof(".."));
    }

    data_linki(dp->superb, dp, ip, name, strlen(name));
    meta_writei(dp->superb, dp);
    meta_writei(ip->superb, ip);

    return ip;
}

