/* This program is used to test if eagle_mkfs program has processed the disk to
 * the right format.
 **/

#include "../diskio.h"
#include "../spblk.h"
#include <unistd.h>
#include <stdio.h>

int load_disk(char* disk_path)
{
   // disk = malloc(disk_size);
    if ((disk_fd = open(disk_path, O_RDONLY)) == -1) {
        fprintf(stderr, "Disk loaded error!\n");
        return -1;
    }

    fprintf(stderr, "Disk loaded!\n");
    return 0;
}

//Return the number of free data blocks
int test_free_db_lst()
{
    FILE* fp = fopen("Free_db_lst.txt", "wt");
    int ret = 0;

    UINT db_lst[FREE_BLK_NODE_SZ];

    if (sp_blk.free_blks_hd == 0) {
        fclose(fp);
        return 0;    //No free data blocks
    }

    disk_read(db_lst, sp_blk.free_blks_hd, 1);
    ret++;
    int i = sp_blk.next_free_blks_pos;

    while (1) {
        for (; i < FREE_BLK_NODE_SZ; i++)
        {
            fprintf(fp, "%d, ", db_lst[i]);
            if (((db_lst[i] < 2 + INODE_LST_SZ)
                 &&(db_lst[i]!=0))||(db_lst[i] > BLK_NUM - 1)){
                fclose(fp);

                fprintf(stderr, "Wrong Block Number: %d: %d!\n", i, db_lst[i]);
                return -1;
            }

            if ((db_lst[i] == 0) && (db_lst[FREE_BLK_NODE_SZ-1]!=0)) {
                fclose(fp);
                fprintf(stderr, "Shouldn't be zero!\n");
                return -1;
            }

            if (db_lst[i] > 1 + INODE_LST_SZ)
                ret++;

        }
        fprintf(fp, "\n");

        if (db_lst[i-1] == 0)
            break;

        fprintf(fp, "%d: ", db_lst[i-1]);

        disk_read(db_lst, db_lst[i-1], 1);

        i = 0;
    }

    fclose(fp);

    return ret;
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


    char flag_string[BLOCK_SIZE];
    disk_read(flag_string, 0, 1);

    printf("Testing mkfs flag string in block 1: %s\n", flag_string);

    if (strncmp(flag_string, "Eagle", strlen("Eagle"))) {
        fprintf(stderr, "mkfs flag string not right!It should be 'Eagle'!\n");
        close(disk_fd);
        return -1;
    } else
        printf("Passed!\n");
    printf("\n");

    printf("Testing super block content:\n");
    super_read();
    super_print();
    int is_passed = 1;

    if (sp_blk.inode_lst_sz!=INODE_LST_SZ) {
        is_passed = 0;
        printf("Inode list size is not correct!\n");
    }
    if (sp_blk.is_modified!=0x00) {
        is_passed = 0;
        printf("is_modified is not right!\n");
    }

    if (is_passed)
        printf("Passed!\n");
    else {
        printf("Not Passed!\n");
        close(disk_fd);
        return -1;

    }
    printf("\n");

    printf("Testing free data block list:\n");
    int free_db_num;
    free_db_num = test_free_db_lst();
    printf("Number of free data blocks: %d\n", free_db_num);
    if (sp_blk.free_blks_num == free_db_num) {
        printf("Passed!(%d used)\n", DATA_BLKS_NUM - free_db_num);
    } else {
        printf("Not correct!\n");
        close(disk_fd);
        return -1;

    }


    close(disk_fd);

    return 0;
}
