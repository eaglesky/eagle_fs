CC = gcc
CFLAGS =-Wall -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -g
PROG = fs
MKFS = eagle_mkfs

SRCS = fs.c syscalls.c diskio.c datablk.c inode.c spblk.c dir.c
SRCS_MKFS = eagle_mkfs.c diskio.c spblk.c inode.c datablk.c dir.c


all: $(PROG) $(MKFS) $(TDISK)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) `pkg-config fuse --cflags --libs` -o $(PROG)

$(MKFS):	$(SRCS_MKFS)
	$(CC) $(CFLAGS) $(SRCS_MKFS) -o $(MKFS)

clean:
	rm -f $(PROG) $(MKFS)
