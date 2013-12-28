#include "diskio.h"
#include "inode.h"
#include "dir.h"
#include "spblk.h"
#include "datablk.h"
#include <unistd.h>

pthread_mutex_t sp_lock;

int load_disk(char* disk_path)
{
   // disk = malloc(disk_size);
    if ((disk_fd = open(disk_path, O_RDWR)) == -1) {
        fprintf(stderr, "Disk loaded error!\n");
        return -1;
    }

    fprintf(stderr, "Disk loaded!\n");
    return 0;
}

void mkfs_flag_init()
{
    char flag_string[20];
   // memset(flag_string, '0', 20);
    strcpy(flag_string, "Eagle\0");


    disk_write(flag_string, 0, 1);
}



int eagle_sys_read(uint64_t fd, char *buf, size_t size, off_t offset)
{
    inode_incore* pinode = (inode_incore*)fd;

    if (!pinode)
        return -1;

    int count = 0;

    while (count < size) {
        UINT in_offset, num_left;
        UINT blk_id;
        if (offset < pinode->inode.file_sz) {
            blk_id = bmap(pinode, offset, &in_offset, &num_left);

            if (blk_id <= 0)
                break;
        } else
            break;

        BYTE buf0[BLOCK_SIZE];
        disk_read(buf0, blk_id, 1);

        int i;
        for (i = in_offset; i < BLOCK_SIZE; i++)
        {
            if (count >= size)
                break;

            buf[count] = buf0[i];

            count++;
        }
        offset += i - in_offset;
    }

    return count;
}

int eagle_sys_write(uint64_t fd, const char *buf, size_t size, off_t offset)
{
    inode_incore* pinode = (inode_incore*)fd;

    if (!pinode)
        return -1;

    int j;
    for (j = 0; j < size; )
    {
        UINT in_offset, num_left;
        //if (offset >= 8447)
         //   printf("here!\n");

        UINT blk_id = bmap(pinode, offset, &in_offset, &num_left);

        if (blk_id <= 0)
            break;


        BYTE buf0[BLOCK_SIZE];

        disk_read(buf0, blk_id, 1);

        int i;
        for (i = in_offset; i < BLOCK_SIZE; i++)
        {
            if (j >= size)
                break;

            buf0[i] = buf[j];
            j++;
        }


        disk_write(buf0, blk_id, 1);

        offset += i - in_offset;
        int sz_inc = offset  - pinode->inode.file_sz;

        if (sz_inc > 0)
            pinode->inode.file_sz += sz_inc;


    }

    pinode->inode.i_tv[1].tv_sec = time(NULL);
    return j;
}

/* only used while file system is made */
int eagle_sys_mkroot()
{
    inode_incore * incore = ialloc();
    dir_ent ent;
    if(incore->inode_num != 0)
        return -1;  /* this is not right !*/
    incore->inode.itype = 2;
    incore->inode.perms = 0770;

    ent.d_inode = 0;
    strcpy(ent.d_name, ".");
    dirent_add(incore, &ent);
    strcpy(ent.d_name, "..");
    dirent_add(incore, &ent);

    incore->inode.link_num = 3;
    incore->inode.i_tv[0].tv_sec = time(NULL);
    incore->inode.i_tv[1].tv_sec = time(NULL);

    incore->status |= 0x04;
    iput(incore);

    return 0;
}

void mkfs()
{
    super_init();
    super_write();

    pthread_mutex_init(&sp_lock, NULL);

   /* sp_blk.fs_size = 1;
    sp_blk.free_blks_num = 1;
    sp_blk.inode_lst_sz = 1;
    sp_blk.free_inodes_num = 1;

    int i;

    for (i = 0; i < SP_BLK_SZ; i++)
    {
        sp_blk.free_blks_lst[i] = 1;
    }

    for (i = 0; i < SP_INODES_SZ; i++)
    {
        sp_blk.free_inodes_lst[i] = 1;
    }

    super_read();*/

    inode_lst_init();
    inode_incore_init();

    data_blk_init();
    eagle_sys_mkroot();


    fprintf(stderr, "mkfs ok!\n");
}

int eagle_sync()
{
    int ret1 = super_write();
    int ret2 = disk_write(free_db_hd_lst, sp_blk.free_blks_hd, 1);

    return ret1&&ret2;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Path of device must be provided!\n");
        return -1;
    }

    char* disk_path = realpath(argv[1], NULL);
    if (!disk_path) {
        fprintf(stderr, "Disk path is not correct!\n");
        return -1;
    }
    fprintf(stderr, "Disk path: %s\n", disk_path);

    if (load_disk(disk_path) == -1)
        return -1;

    mkfs_flag_init();

    mkfs();

    fprintf(stderr, "DISK_SIZE = %ld\n", DISK_SIZE);

    fprintf(stderr, "BLOCK_SIZE = %ld\n", BLOCK_SIZE);

    eagle_sync();
    close(disk_fd);

    return 0;
}
