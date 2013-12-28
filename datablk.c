#include "datablk.h"
#include "diskio.h"
#include <errno.h>
#include <string.h>

UINT free_db_hd_lst[FREE_BLK_NODE_SZ];

void data_blk_init()
{
    UINT id_array[FREE_BLK_NODE_SZ];

    int i;

    for (i = sp_blk.free_blks_hd; i < BLK_NUM; i += FREE_BLK_NODE_SZ)
    {
        int j;
        memset(id_array, 0, sizeof(int)*FREE_BLK_NODE_SZ);
        for (j = 0; (j < FREE_BLK_NODE_SZ) && ((i+j+1) < BLK_NUM); j++)
        {
            id_array[j] = i + j + 1;
        }

        disk_write(id_array, i, 1);
    }

   /* for (i = sp_blk.free_blks_lst[SP_BLK_SZ-1]; i < BLK_NUM; i += SP_BLK_SZ)
    {
        memset(id_array, 0, sizeof(UINT)*SP_BLK_SZ);
        disk_read(id_array, i, 1);
    }*/

    disk_read(free_db_hd_lst, sp_blk.free_blks_hd, 1);


}

//Return block number if succeeds, otherwise return error code
UINT db_alloc()
{
    int ret = 0;

    pthread_mutex_lock(&sp_lock);

    int i = sp_blk.next_free_blks_pos;

   /* for (i = sp_blk.next_free_blks_pos; i < FREE_BLK_NODE_SZ - 1; i++)
    {
        if (free_db_hd_lst[i] != 0)
            break;
    }*/
    if (sp_blk.free_blks_hd == 0) {
	pthread_mutex_unlock(&sp_lock);
        return 0;
    }

    if (i < FREE_BLK_NODE_SZ)
        ret = free_db_hd_lst[i];

    if (i < FREE_BLK_NODE_SZ - 1) {

        if (free_db_hd_lst[i] == 0) { //Last block allocated!
            ret = sp_blk.free_blks_hd;
            sp_blk.free_blks_hd = 0;

            sp_blk.free_blks_num--;
            sp_blk.is_modified = 0x01;
	    pthread_mutex_unlock(&sp_lock);
            return ret;
        }

        free_db_hd_lst[i] = 0;
        sp_blk.next_free_blks_pos++;

    } else if ((i == FREE_BLK_NODE_SZ - 1) &&(free_db_hd_lst[i] != 0)){
        disk_read(free_db_hd_lst, ret, 1);
        disk_write(free_db_hd_lst, sp_blk.free_blks_hd, 1);
        sp_blk.next_free_blks_pos = 0;

    } else { // Last one in the list is 0 indicates it's the last free block
        ret = sp_blk.free_blks_hd;
        sp_blk.free_blks_hd = 0;

    }

    sp_blk.free_blks_num--;
    sp_blk.is_modified = 0x01;

    BYTE zeros[BLOCK_SIZE] = {0};
    disk_write(zeros, ret, 1);

    sync();

    pthread_mutex_unlock(&sp_lock);

    fprintf(stderr, "datablk %d used!\n", ret);
    return ret;
}

//MAKE SURE THE BLK_NUM IS NOT ON THE FREE LIST!!
//Otherwise it will CRASH!
int db_free(UINT blk_num)
{


    if ((blk_num > (BLK_NUM-1))||(blk_num < INODE_LST_SZ + 2)) {
        return -1;
    }

    pthread_mutex_lock(&sp_lock);

    if (sp_blk.free_blks_hd == 0) //Free list is empty
    {
        sp_blk.free_blks_hd = blk_num;
        memset(free_db_hd_lst, 0, sizeof(int)*FREE_BLK_NODE_SZ);
        disk_write(free_db_hd_lst, blk_num, 1);//Initialize new free list
        sp_blk.next_free_blks_pos = FREE_BLK_NODE_SZ - 1;

        sp_blk.free_blks_num++;
        sp_blk.is_modified = 0x01;

        pthread_mutex_unlock(&sp_lock);

        return 0;
    }

    int pos = --sp_blk.next_free_blks_pos;
    if (pos >= 0) {
        free_db_hd_lst[pos] = blk_num;

    } else {//Free list is full
        disk_write(free_db_hd_lst, sp_blk.free_blks_hd, 1);
        memset(free_db_hd_lst, 0, sizeof(int)*FREE_BLK_NODE_SZ);
        free_db_hd_lst[FREE_BLK_NODE_SZ - 1] = sp_blk.free_blks_hd;
        sp_blk.free_blks_hd = blk_num;
        sp_blk.next_free_blks_pos = FREE_BLK_NODE_SZ - 1;

    }

    sp_blk.free_blks_num++;
    sp_blk.is_modified = 0x01;

    sync();

    pthread_mutex_unlock(&sp_lock);

    return 0;
}

int bmap(inode_incore *incore, off_t offset, UINT* const in_offset, UINT* const num_IO)
{
    if ((offset < incore->inode.file_sz)||(offset == incore->inode.file_sz))
        return bmap_core(incore, offset, in_offset, num_IO);
    else {
        int start_blk = (incore->inode.file_sz)/BLOCK_SIZE;
        int end_blk = offset/BLOCK_SIZE;

        off_t init_oft = (off_t)(incore->inode.file_sz);
        bmap_core(incore, init_oft, in_offset, num_IO);


      /*  if (start_blk == end_blk) {
            incore->inode.file_sz = offset;
            return bmap_core(incore, offset, in_offset, num_IO);
        } else {*/
            int ret;
            start_blk++;
            while ((start_blk < end_blk)||
                       (start_blk == end_blk)) {
                incore->inode.file_sz = start_blk*BLOCK_SIZE;
                off_t oft = start_blk*BLOCK_SIZE;
                ret = bmap_core(incore, oft, in_offset, num_IO);
                start_blk++;
            }
            incore->inode.file_sz = offset;
            return bmap_core(incore, offset, in_offset, num_IO);
      //  }

    }
}

// bmap: map logical byte offset in file to block number in file system
int bmap_core(inode_incore *incore, off_t offset, UINT* const in_offset, UINT* const num_IO)
{
	int blk_in_file = offset/BLOCK_SIZE;
	int in_blk_offset = offset%BLOCK_SIZE;
	int tmp = (blk_in_file + 1)*BLOCK_SIZE;
	int num_io = tmp > incore->inode.file_sz ? incore->inode.file_sz - offset : tmp - offset;

    if (in_offset)
        *in_offset = in_blk_offset;

    if (num_IO)
        *num_IO = num_io;

    if (offset > incore->inode.file_sz)
        return -1;

    // check if read ahead applicable ,mark inode


    UINT *indirect, blk_addr, curr_blk_index, buf[FREE_BLK_NODE_SZ];
    BYTE indrct_level;

    blk_addr = 0;
    if(blk_in_file < DIRECT_NUM ) {
         indrct_level = 0;
         indirect = incore->inode.contents;
         curr_blk_index = blk_in_file;
    } else if(blk_in_file < DIRECT_NUM + SINGLE_NUM*FREE_BLK_NODE_SZ) {
        indrct_level = 1;
        indirect = (incore->inode.contents + DIRECT_NUM);   // offset to the first single indirect block number
        curr_blk_index = blk_in_file - DIRECT_NUM;
    } else if(blk_in_file < DIRECT_NUM + SINGLE_NUM*FREE_BLK_NODE_SZ + DOUBLE_NUM*FREE_BLK_NODE_SZ*FREE_BLK_NODE_SZ) {
        indrct_level = 2;
        indirect = (incore->inode.contents + DIRECT_NUM + SINGLE_NUM);   //offset to the first double indirect block number
        curr_blk_index = blk_in_file - DIRECT_NUM - SINGLE_NUM*FREE_BLK_NODE_SZ;
    } else {
        return -1;  // invalid offset!
    }
 
        int i;
        while(indrct_level) {
                for(i = 0, tmp = 1; i < indrct_level; i++)
                        tmp = tmp*FREE_BLK_NODE_SZ;
	        if((curr_blk_index%tmp == 0) && (offset == incore->inode.file_sz) &&(in_blk_offset == 0)){
                    UINT indir_blk_addr = blk_addr;
                    blk_addr = db_alloc();
                    if (blk_addr == 0)
                        return -1;

                    indirect[curr_blk_index/tmp] = blk_addr;

                    if (indir_blk_addr > 0) {
                        disk_write(indirect, indir_blk_addr, 1);
                    }

                } else {
                    blk_addr = indirect[curr_blk_index/tmp];
                }

        indirect = buf;
        curr_blk_index = curr_blk_index%tmp;
        disk_read(indirect, blk_addr, 1);
        indrct_level--;
        }

    if ((offset == incore->inode.file_sz)&&(in_blk_offset == 0)) {
        indirect[curr_blk_index] = db_alloc();
        if (indirect[curr_blk_index] == 0)
            return -1;

        if (blk_addr > 0)
            disk_write(indirect, blk_addr, 1);
    }

    return indirect[curr_blk_index];
}

/* release all the blocks in a file */
int file_free(inode_incore * incore)
{
	int i, blk_num, in_offset, num_io;
	for(i = 0; i < incore->inode.file_sz; ) {  /* free all the data block */
		blk_num = bmap(incore, i, &in_offset, &num_io);
		db_free(blk_num);
		i += BLOCK_SIZE;
	}

	int addr_num = BLOCK_SIZE/sizeof(UINT);
	for(i = 0; i < INODE_FP_SIZE; i ++) /* free all addressing blocks and reset contents array to 0s*/
		if(incore->inode.contents[i]) {
			if(i < DIRECT_NUM) {
				incore->inode.contents[i] = 0;
			} else if (i < DIRECT_NUM + SINGLE_NUM) {
				db_free(incore->inode.contents[i]);
				incore->inode.contents[i] = 0;
			} else {
				UINT addrs[addr_num];
				disk_read(addrs, incore->inode.contents[i], 1);
				int j;
				for(j = 0; j < addr_num; j++)
					if(addrs[j]) {
							db_free(addrs[j]);
							addrs[j] = 0;
					}
				db_free(incore->inode.contents[i]);
				incore->inode.contents[i] = 0;
			}
		}


	incore->inode.file_sz = 0;

	return 0;
}
