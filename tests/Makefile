CC = gcc
CFLAGS = -Wall -g -D_FILE_OFFSET_BITS=64
PROG_RW = test_rw
PROG_FLAGS = test_flags
PROG_MKFS  = test_mkfs

PROG_INODE = test_inode

PROG_I_HD = i_hd

PROG_INFO = info

SRCS_INODE = test_inode.c ../diskio.c  ../spblk.c ../inode.c ../dir.c ../datablk.c ../syscalls.c

SRCS_I_HD = i_hd.c ../inode.c ../spblk.c ../diskio.c ../datablk.c ../dir.c ../syscalls.c

SRCS_INFO = test_get_info.c ../spblk.c  ../diskio.c

SRCS_RW = test_rw.c
SRCS_FLAGS = test_flags.c
SRCS_MKFS  = test_mkfs.c ../diskio.c ../spblk.c

all: $(PROG_INODE) $(PROG_RW) $(PROG_FLAGS) $(PROG_MKFS) $(PROG_I_HD) $(PROG_INFO)


$(PROG_RW):	$(SRCS_RW)
	$(CC) $(CFLAGS) $(SRCS_RW) -o $(PROG_RW)

$(PROG_FLAGS):	$(SRCS_FLAGS)
	$(CC) $(CFLAGS) $(SRCS_FLAGS) -o $(PROG_FLAGS)

$(PROG_MKFS):  $(SRCS_MKFS)
	$(CC) $(CFLAGS) $(SRCS_MKFS) -o $(PROG_MKFS)
	
$(PROG_INODE):	$(SRCS_INODE)
	$(CC) $(CFLAGS) $(SRCS_INODE) -o $(PROG_INODE)

$(PROG_I_HD):	$(SRCS_I_HD)
	$(CC) $(CFLAGS) $(SRCS_I_HD) -o $(PROG_I_HD)

$(PROG_INFO):	$(SRCS_INFO)
	$(CC) $(CFLAGS) $(SRCS_INFO) -o $(PROG_INFO)

clean:
	rm -f $(PROG_INODE) $(PROG_RW) $(PROG_FLAGS) $(PROG_MKFS) $(PROG_I_HD) $(PROG_INFO)

