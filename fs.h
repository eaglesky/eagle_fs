/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall myfs.c mydiskio.c `pkg-config fuse --cflags --libs` -o myfs
*/

#define FUSE_USE_VERSION 26
#define LOG_FILE ((FILE*)fuse_get_context()->private_data)

#include <fuse.h>
#include "fsconfig.h"







//DRESULT super_check();//For debugging


int load_disk(char* disk_path);
void dump_disk(char *path);
void mkfs();

int sync();





