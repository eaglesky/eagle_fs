#include "diskio.h"
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

//BYTE* disk = NULL;
int disk_fd;

//disk_read and disk_write cannot gurantee that buf is big enough
int disk_read(void* buf, int blk_addr, int blk_num)
{
  //  if (!disk)
   //     return -ENODEV;

   // if ((blk_addr + blk_num) * BLOCK_SIZE > DISK_SIZE)
    //    return -ESPIPE;

   // memcpy(buf, disk + blk_addr*BLOCK_SIZE, blk_num*BLOCK_SIZE);
    int tmp = disk_fd;

    if (pread(disk_fd, buf, blk_num*BLOCK_SIZE, blk_addr*BLOCK_SIZE) < 0) {
	disk_fd = tmp;
        return -1;
    }

    disk_fd = tmp;

    return 0;
}

int disk_write(const void* buf, int blk_addr, int blk_num)
{
  //  if(!disk)
   //     return -ENODEV;

   // if ((blk_addr + blk_num) * BLOCK_SIZE > DISK_SIZE)
    //    return -ESPIPE;

   // memcpy(disk + blk_addr*BLOCK_SIZE, buf, blk_num*BLOCK_SIZE);

    int tmp = disk_fd;

    if (pwrite(disk_fd, buf, blk_num*BLOCK_SIZE, blk_addr*BLOCK_SIZE) < 0) {
	disk_fd = tmp;
        return -1;
    }

    disk_fd = tmp;

    return 0;
}
