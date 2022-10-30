#ifndef __TINY_KERNEL_SYS_CALL_H__
#define __TINY_KERNEL_SYS_CALL_H__

// System call numbers
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_pipe    4
#define SYS_read    5
#define SYS_kill    6
#define SYS_exec    7
#define SYS_fstat   8
#define SYS_chdir   9
#define SYS_dup    10
#define SYS_getpid 11
#define SYS_sbrk   12
#define SYS_sleep  13
#define SYS_uptime 14
#define SYS_open   15
#define SYS_write  16
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21
#define SYS_setup  22

#ifndef __ASSEMBLER__

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);

#define T_DIR 1
#define T_FILE 2
#define T_DEV 3
struct stat {
    short type;
    short n_link;
    int dev;
    int ino;
    int size;
};

#define FS_NAME_SIZE 14
struct dirent {
    short idx;
    char name[FS_NAME_SIZE];
};
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int test(const char*);

#endif //__ASSEMBLER__

#endif
