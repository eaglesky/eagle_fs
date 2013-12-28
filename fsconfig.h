
#ifndef _F_CONFIG
#define _F_CONFIG


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <stdint.h>
#include <pthread.h>

#define DISK_SIZE       (sp_blk.fs_size)

#define BLOCK_SIZE      4048
#define INODE_LST_SZ 20

#define DIRECT_NUM 2
#define SINGLE_NUM 1
#define DOUBLE_NUM 1


#define SP_INODES_SZ    20
#define MAX_OPEN_FILES      300
#define INODE_HASH_SLOTS    20
#define MAX_FNAME_LEN	   256




#define INODE_FP_SIZE (DIRECT_NUM+SINGLE_NUM+DOUBLE_NUM)

#define BLK_NUM         (DISK_SIZE/BLOCK_SIZE) //Total block number: 100

#define DATA_BLKS_NUM   (BLK_NUM - INODE_LST_SZ - 2) //Data block number: 78

#define INODE_NUM_PER_BLK (BLOCK_SIZE/sizeof(inode_disk))

#define FREE_BLK_NODE_SZ       (BLOCK_SIZE/sizeof(UINT))



typedef uint8_t BYTE; //This type must be 1 byte


typedef uint32_t UINT; //This type must be 4 bytes


typedef BYTE file_type;
typedef struct {
//    int owner_id; //0:not set
    uid_t user_id;
    gid_t group_id;
    file_type itype;  //0:free; 1:regular; 2: directory; 3: special; 4: pipe
    mode_t perms;
    UINT link_num;
    UINT contents[INODE_FP_SIZE];
    UINT file_sz;
    struct timespec i_tv[2];

} inode_disk;

typedef struct inode_incore inode_incore;

struct inode_incore{

    inode_disk inode;

//    UINT dev_num; // logical device num of the file system that contains the file
    UINT inode_num;
    UINT ref_count;
    BYTE status;/* 
		 *	0x04 -> the in-core copy differs from disk copy due to a change to the data in inode
		 *	0x08 -> the in-core copy differs from disk copy due to a change to the file data
		 *	0x10 -> the file is a mount point
		*/
    pthread_mutex_t lock;
    inode_incore * next_free;
    inode_incore * previous_free;
    inode_incore * next_hash;
    inode_incore * previous_hash;

};



typedef struct {
    UINT fs_size;
    UINT free_blks_num;
    UINT inode_lst_sz;
    UINT free_inodes_num;
    BYTE is_modified; //0x01--Is modified
    BYTE rem_index; //remembered index for next free disk inode search
    BYTE lock; // two bits are used ! 0x01 -> free blks list locked, 0x10 -> free inodes list locked

    BYTE i_index; //search index for next free inode
    UINT free_blks_hd; //0 means there's no free blocks
    UINT next_free_blks_pos;
    UINT free_inodes_lst[SP_INODES_SZ]; //Inode id
} super_blk;

/* directory entry */
typedef struct {
    UINT d_inode;	//inode number
    UINT d_offset;	//offset to next entry
    UINT d_length;	//length of this record
    char d_name[MAX_FNAME_LEN + 1];	//name of this entry
} dir_ent;

typedef struct {
	UINT d_inode;
	char d_name[MAX_FNAME_LEN + 1];
} dirent_d;

extern int disk_fd;
extern super_blk sp_blk;
extern pthread_mutex_t sp_lock;
extern UINT free_db_hd_lst[FREE_BLK_NODE_SZ];


#endif

