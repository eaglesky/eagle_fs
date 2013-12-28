#include "inode.h"
#include "diskio.h"
#include "datablk.h"
#include "dir.h"
#include "spblk.h"

static inode_incore i_hash[INODE_HASH_SLOTS];
static inode_incore *i_free;
static int refill_spblk();
//static pthread_mutex_t h_lock = PTHREAD_MUTEX_INITIALIZER;

void inode_lst_init()
{
    int i;
   /* for (i = 0; i < INODE_NUM_PER_BLK; i++)
    {
        inodes_blk[i].owner_id = 0;
        inodes_blk[i].itype = 0;
        int j;
        for (j = 0; j < 3; j++)
        {
            inodes_blk[i].perms[j] = 0;
        }
        inodes_blk[i].link_num = 0;
        memset(&inodes_blk[i], 0, sizeof(inode_disk));

    }*/


    for (i = 0; i < INODE_LST_SZ; i++)
    {
        inode_disk inodes_blk[INODE_NUM_PER_BLK];
        memset(inodes_blk, 0, sizeof(inode_disk)*INODE_NUM_PER_BLK);

        disk_write(inodes_blk, 2+i, 1);
    }

   /* for (i = 0; i < INODE_LST_SZ; i++)
    {
        inode_disk inodes_blk[INODE_NUM_PER_BLK];

        disk_read(inodes_blk, 2+i, 1);

        int j = 0;
        for (; j < INODE_NUM_PER_BLK; j++)
        {
            fprintf(stderr, "filetype: %d\n", inodes_blk[j].itype);

        }
    }*/

    fprintf(stderr, "sizeof inode_disk: %ld\n", sizeof(inode_disk));
    fprintf(stderr, "sizeof inode_num_per_blk: %ld\n", INODE_NUM_PER_BLK);
}

int inode_incore_init()
{
    i_free = calloc(MAX_OPEN_FILES + 1, sizeof(inode_incore));
    if (!i_free) {
	if ( errno <= 0)
		errno = ENOMEM;
        return -1;
	}

    /* put all the in-core inodes to the free lists  */
    int i;
    i_free->next_free = i_free + 1;
    i_free->previous_free = i_free + MAX_OPEN_FILES;
    pthread_mutex_init(&(i_free->lock), NULL);
    for (i = 1; i < MAX_OPEN_FILES; i++) {
        i_free[i].next_free = i_free + i + 1;
        i_free[i].previous_free = i_free + i - 1;
	pthread_mutex_init(&(i_free[i].lock), NULL);
    }
    i_free[MAX_OPEN_FILES].next_free = i_free;
    i_free[MAX_OPEN_FILES].previous_free = i_free + MAX_OPEN_FILES - 1;
    pthread_mutex_init(&(i_free[MAX_OPEN_FILES].lock), NULL);
    return 0;
}

/* iget ! given an inode number, iget will return a locked inode */
inode_incore * iget(UINT i_num)
{
    //fprintf(stderr, "iget begain!\n");
	pthread_mutex_lock(&(i_free->lock));
    //fprintf(stderr, "lock successfully!\n");
	inode_incore *incore = i_hash[i_num % INODE_HASH_SLOTS].next_hash; 

        while(incore != NULL && (incore->inode_num != i_num))   /* search for inode with given i_num in cache */
            incore = incore->next_hash;


        if(incore) { 
		pthread_mutex_lock(&(incore->lock));
			/*  lock the inode if not locked
			 *  wait until inode is  unlocked if already locked
			 */
		if(incore->next_free) {    /* the incore inode is in the free list, remove it */
	                incore->previous_free->next_free = incore->next_free;
        	        incore->next_free->previous_free = incore->previous_free;
			incore->previous_free = NULL;
	                incore->next_free = NULL;
		}

		incore->ref_count ++;
		pthread_mutex_unlock(&(i_free->lock));
		
        //fprintf(stderr, "leaving iget...\n");
		return(incore);
        }

        /* inode not in the cahce */

        if(i_free->next_free == i_free) {    /* in-core free list is empty, no space for more open files! */
		pthread_mutex_unlock(&(i_free->lock));
		errno = ENFILE;
		return NULL;
        }

        incore = i_free->previous_free;

/*	int eno;
	eno = pthread_mutex_trylock(&(incore->lock));
	if ( eno && eno == EBUSY ) {        // the inode has already been locked! and it should be linked to hash queue
		pthread_mutex_unlock(&(i_free->lock));
		incore
		continue;
	}
*/
	pthread_mutex_lock(&(incore->lock));

	incore->next_free->previous_free = incore->previous_free;
        incore->previous_free->next_free = incore->next_free;
	incore->previous_free = incore->next_free = NULL;
        incore->inode_num = i_num;

//        incore->dev_num ?
        /* remove the in-core inode from old hash queue and insert it into new one */
        if(incore->previous_hash) {
            incore->previous_hash->next_hash = incore->next_hash;
            if(incore->next_hash) 
                incore->next_hash->previous_hash = incore->previous_hash;
        }
        incore->previous_hash = i_hash + i_num % INODE_HASH_SLOTS;
        incore->next_hash = incore->previous_hash->next_hash;
        if(incore->next_hash) 
            incore->next_hash->previous_hash = incore;
        incore->previous_hash->next_hash = incore;

	pthread_mutex_unlock(&(i_free->lock));

        /* read inode from disk */
        inode_rblk(incore);   

        incore->ref_count = 1;

        /* other initialization goes here */
    //fprintf(stderr, "leaving iget...\n");
        return incore;
}

/* iput: release access to in-core inode */
void iput (inode_incore *incore)
{
/*    if(! (incore->status & 0x01))
 *       incore->status |= 0x01;
 */
    //fprintf(stderr, "iput begin!\n");
	pthread_mutex_lock(&(i_free->lock));
//	pthread_mutex_lock(&(incore->lock));
    //fprintf(stderr, "lock acquired!\n");
	incore->ref_count --;
	if(incore->ref_count == 0) {
        	if(incore->inode.link_num == 0) {  /* no link to the file exists ! */
            		incore->inode.itype = 0;
			file_free(incore);
			inode_wblk(incore);
			ifree(incore->inode_num);
			incore->status ^= 0x04;
		} else if(incore->status & 0x04) {   /* if file changed, file accessed or inode changed, modify the disk inode  */
			inode_wblk(incore);
			incore->status ^= 0x04;
		}

    /* insert in-core back into the free list(while it still stays in the recent hash queue) */
        incore->previous_free = i_free;
        incore->next_free = i_free->next_free;
        incore->next_free->previous_free = incore;
        i_free->next_free = incore;
	}
//    incore->status ^= 0x01;

	pthread_mutex_unlock(&(i_free->lock));
	pthread_mutex_unlock(&(incore->lock));

	incore = NULL;
    //fprintf(stderr, "leaving iput...\n");
}

/* ialloc : allocate a new inode from disk */

inode_incore *ialloc()    // inode_incore *ialloc(UINT dev_num) ?
{
    //fprintf(stderr, "ialloc begin!\n");
    int i_num;
    inode_incore * incore;
    while(1) {
/*        if(sp_blk.lock & 0x01) { // the free inode list is locked
            sleep ?
            continue;
        }  */
	pthread_mutex_lock(&sp_lock);
    //fprintf(stderr, "lock acquired!\n");

        if(sp_blk.i_index == sp_blk.rem_index) {
//            sp_blk.lock |= 0x01;
/*	 the starting number for free inode search is
 *	 sp__blk.free_inodes_lst[sp_blk.rem_index], bread brelse involved
 *	 full fill the free_inodes_lst or no more free inodes
 *	 left 
 */
//		fprintf(stderr, "refill spblk begin!\n");
		if(refill_spblk() == -1) {
        //	fprintf(stderr, "No more disk inode!\n");
			pthread_mutex_unlock(&sp_lock);
			errno = ENOSPC;
			return NULL;
		}
//            sp_blk.lock ^= 0x01;
            /* wake up other processes waiting for the unlock */
//		fprintf(stderr, "refill finished!\n");
        }
	pthread_mutex_unlock(&sp_lock);
        i_num = sp_blk.free_inodes_lst[sp_blk.i_index - 1];
        incore = iget(i_num);
        if(incore->inode.itype) { /* inode is not free after all! */
            inode_wblk(incore); //not implemented !
            iput(incore);
            continue;
        }
        /* inode is free */
        incore->inode.link_num = 1; /* other initializations, e.g. set own_id, file_type etc. */
        inode_wblk(incore);
        sp_blk.i_index--;
        sp_blk.free_inodes_num--;

	pthread_mutex_lock(&sp_lock);
	super_write();
	pthread_mutex_unlock(&sp_lock);

    //fprintf(stderr, "ialloc finished!\n");
        return incore;
    }
}

/* ifree: free a disk inode */
void ifree(UINT i_num)
{
    //fprintf(stderr, "ifree begin!\n");
	sp_blk.free_inodes_num++;
/*    if(sp_blk.lock & 0x01)
        return;
    sp_blk.lock |= 0x01;
*/
	pthread_mutex_lock(&sp_lock);
    //fprintf(stderr, "lock acquired!\n");

	if(sp_blk.i_index == sp_blk.rem_index) {
		if(refill_spblk() == -1) {
			if(sp_blk.i_index == SP_INODES_SZ)
				sp_blk.free_inodes_lst[--(sp_blk.rem_index)] = i_num;
			else
				sp_blk.free_inodes_lst[sp_blk.i_index ++] = i_num;

			pthread_mutex_unlock(&sp_lock);
        //	fprintf(stderr, "leaving ifree...\n");

			return;
		}
	}

	if(sp_blk.i_index == SP_INODES_SZ) { /* super block inodes list is full */
		if(sp_blk.free_inodes_lst[sp_blk.rem_index] > i_num)
			sp_blk.free_inodes_lst[sp_blk.rem_index] = i_num;
	} else
		sp_blk.free_inodes_lst[sp_blk.i_index++] = i_num;  /* store the inode num in the list */
//    sp_blk.lock ^= 0x01;
	super_write();
	pthread_mutex_unlock(&sp_lock);
    //fprintf(stderr, "leaving ifree...\n");
	return;
}

static int refill_spblk()
{
            int start_blk_addr = sp_blk.free_inodes_lst[sp_blk.rem_index]/INODE_NUM_PER_BLK + 2;
            int blks_num = SP_INODES_SZ/INODE_NUM_PER_BLK + 2;
            int i_offset = sp_blk.free_inodes_lst[sp_blk.rem_index]%INODE_NUM_PER_BLK + 1;
            BYTE* buf = malloc(blks_num*BLOCK_SIZE);

//	    inode_disk* buf = malloc(blks_num*BLOCK_SIZE);
	    inode_disk *buf_seek = NULL;      /* keep track of the block head */
	    int tmp_mark = SP_INODES_SZ - 1;
            while( tmp_mark >= 0  && (start_blk_addr < INODE_LST_SZ + 2)) {
		if(start_blk_addr + blks_num >= INODE_LST_SZ + 2)
			blks_num = INODE_LST_SZ - start_blk_addr + 2;
                disk_read(buf, start_blk_addr, blks_num);
		buf_seek = (inode_disk*) buf;
/*		int i;
		for(i = 0; i < INODE_NUM_PER_BLK; i++) 
			printf("file type:\t%d\n", (buf+i)->itype);
*/
		while(i_offset < blks_num*INODE_NUM_PER_BLK) {
			if(i_offset && i_offset%INODE_NUM_PER_BLK == 0)    /* new block is incoming ! */
				buf_seek = (inode_disk*)((BYTE*)buf_seek + BLOCK_SIZE);

			if((buf_seek + i_offset%INODE_NUM_PER_BLK)->itype == 0) {
				sp_blk.free_inodes_lst[ tmp_mark-- ] = (start_blk_addr - 2)*INODE_NUM_PER_BLK + i_offset;
			}
                    i_offset ++;
                    if(tmp_mark == -1)
                        break;
//			fprintf(stderr, "searching the read-in buffer!\n");
                }
                start_blk_addr += blks_num;
		i_offset = 0;
//			fprintf(stderr, "about to start reading new buffer!\n");
             }
            free(buf);
            if(tmp_mark == SP_INODES_SZ - 1) {
                /* no inode ! */
//		fprintf(stderr, "No more disk inodes!\n");
		return -1;
            }
            sp_blk.rem_index = tmp_mark + 1;
            sp_blk.i_index = SP_INODES_SZ;
	    return 0;
}

int inode_rblk(inode_incore* incore)
{
    inode_disk *buf = malloc(BLOCK_SIZE);
    if(! buf)
        return -1;
    disk_read(buf, 2 + (incore->inode_num)/INODE_NUM_PER_BLK, 1);
    memcpy(& (incore->inode), buf + (incore->inode_num)%INODE_NUM_PER_BLK, sizeof(inode_disk));
    free(buf);

    return 0;
}

int inode_wblk(inode_incore* incore)
{
    inode_disk *buf = malloc(BLOCK_SIZE);
    int disk_addr = 2 + (incore->inode_num)/INODE_NUM_PER_BLK;
    if(! buf)
        return -1;
    disk_read(buf, disk_addr, 1);
    memcpy(buf + (incore->inode_num)%INODE_NUM_PER_BLK, & (incore->inode), sizeof(inode_disk));
    disk_write(buf, disk_addr, 1);
    free(buf);

    return 0;
}

void i_incore_destroy()
{
	free(i_free);
}

/* namei: convert a file name(path) to an locked incore inode */

inode_incore * namei(const char * name_p)
{
    //fprintf(stderr, "namei begin!\n");
	inode_incore *i_working;
	const dir_ent * entry;

	char * name = malloc(strlen(name_p) + 1);
	strcpy(name, name_p);
	if(strncmp(name, "/", 1) == 0) // start from root
		i_working = iget(0);
//	else 
		/* get the current working directory , u area! */

	else {
		fprintf(stderr, "Invalid path in namei!\n");
		return NULL; /* the path should start from root */
	}

	dpackage * dpk = calloc(1, sizeof(dpackage));

	char * next;
	next = strtok(name , "/");
	while(next) {
		if(i_working->inode.itype != 2) { /* file type and permission varification goes here */
			iput(i_working);
			free(dpk);
			free(name);
			errno = ENOENT;
            //fprintf(stderr, "leaving namei...\n");
			return NULL; //Not valid!
		}
		if(i_working->inode_num == 0 && strcmp(next, "..") == 0) {
			next = strtok(NULL, "/");
			continue;
		}

		entry = dirent_get(i_working, dpk);

		while(entry != NULL && strcmp(entry->d_name, next))
			entry = dirent_get(NULL, dpk);

		iput(i_working);
		if(! entry) {
			free(dpk);
			free(name);
			errno = ENOENT;
            //fprintf(stderr, "leaving namei...\n");
			return NULL;  /* no such directory or file ! */
		}
		i_working = iget(entry->d_inode);
		next = strtok(NULL, "/");
	}

	free(dpk);
	free(name);
    //fprintf(stderr, "leaving namei...\n");
	return i_working;
}
