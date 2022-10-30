CC=gcc
CFLAGS=-g -O0
LDFLAGS=

all: mkfs lsfs

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

OBJS=mkfs.o io.o inode_op.o
LSOBJS=lsfs.o io.o inode_op.o

mkfs: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

lsfs: $(LSOBJS)
	$(CC) -o $@ $(LDFLAGS) $(LSOBJS)

clean:
	rm -f *.o mkfs lsfs

.PHONY: clean mkfs lsfs
