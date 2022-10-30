#ifndef __TINY_KERNEL_LIBC_H__
#define __TINY_KERNEL_LIBC_H__

#define stdin 0
#define stdout 1
#define stderr 2

struct stat;
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
char* gets(char*, int max);
int strlen(const char*);
void* memset(void*, int, int);
//void* malloc(uint);
void free(void*);
int atoi(const char*);
void printf(const char *fmt, ...);
#endif
