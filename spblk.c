#include "spblk.h"
#include "diskio.h"
#include <unistd.h>

super_blk sp_blk;

void super_init()
{

    sp_blk.fs_size = lseek(disk_fd, 0, SEEK_END);
    lseek(disk_fd, 0, SEEK_SET);

    sp_blk.free_blks_num = DATA_BLKS_NUM;
    sp_blk.inode_lst_sz = INODE_LST_SZ;
    sp_blk.free_inodes_num = INODE_NUM_PER_BLK * INODE_LST_SZ;
    sp_blk.is_modified = 0x01;

    int i;

    /*for (i = 0; i < SP_BLK_SZ; i++)
    {
        sp_blk.free_blks_lst[i] = BLK_NUM - DATA_BLKS_NUM + i;
    }*/
    sp_blk.free_blks_hd = 2 + INODE_LST_SZ;
    sp_blk.next_free_blks_pos = 0;

    for (i = 0; i < SP_INODES_SZ; i++)
    {
        sp_blk.free_inodes_lst[i] = SP_INODES_SZ - i - 1;
    }
    sp_blk.i_index = SP_INODES_SZ;
    sp_blk.rem_index = 0;
    sp_blk.lock = 0x00;


}

int super_read()
{
    return disk_read(&sp_blk, 1, 1);
}

int super_write()
{
    sp_blk.is_modified = 0x00;

    return disk_write(&sp_blk, 1, 1);

}

void super_print()
{
    printf("File system size: %d\n", sp_blk.fs_size);
    printf("Free blocks number: %d\n", sp_blk.free_blks_num);
    printf("Inode list size: %d\n", sp_blk.inode_lst_sz);
    printf("Free inodes number: %d\n", sp_blk.free_inodes_num);
    printf("Is modified? %x\n", sp_blk.is_modified);
    printf("Remembered index for next free disk inode search: %d\n", sp_blk.rem_index);
    printf("Lock flag: %x\n", sp_blk.lock);
    printf("Search index for next free inode: %d\n", sp_blk.i_index);
    printf("Head of free blocks list: %d\n", sp_blk.free_blks_hd);
    printf("Next free block position: %d\n", sp_blk.next_free_blks_pos);
    printf("Super Block Size: %d\n", sizeof(sp_blk));
    printf("Disk inode size: %d\n", sizeof(inode_disk));

    printf("Free inode temp list: ");
    int i;
    for (i = 0; i < SP_INODES_SZ; i++)
    {
        printf("%d, ", sp_blk.free_inodes_lst[i]);
    }
    printf("\n");
}
