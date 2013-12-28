#include "syscalls.h"
#include "fs.h"

#include "diskio.h"
#include "spblk.h"
#include "inode.h"
#include "datablk.h"
#include "dir.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>



int eagle_sys_mknod(const char * path, file_type itype,  mode_t mode)
{
/*   only super user can create special files and directories 
 *   coming soon...
 */
	char name[MAX_FNAME_LEN + 1];
	char parent[strlen(path)];
	get_parent(path, parent, name);
	inode_incore * pa = namei(parent);

	dpackage *dpk = calloc(1, sizeof(dpackage));

	dir_ent * entry;
	entry = dirent_get(pa, dpk);
	while(entry != NULL) {
		if(! strcmp(entry->d_name, name))
			return -1; // the inode has already exist !
		entry = dirent_get(NULL, dpk);
	}

	free(dpk);

	inode_incore *i_new = ialloc();
	if(! i_new)
		return -1; // allocating new inode failed! probably no more free node

	dir_ent * new_ent = calloc(1, sizeof(dir_ent));
	new_ent->d_inode = i_new->inode_num;
	strcpy(new_ent->d_name, name);
	
	if(dirent_add(pa, new_ent))
		return -1; // write new entry failed, should examine the errno

	i_new->inode.itype = itype;
	i_new->inode.perms = mode & 0777;
	if(itype == 2) {  // it will be a directory!
		strcpy(new_ent->d_name, ".");
		if(dirent_add(i_new, new_ent))
			return -1;  // new directory initialization failed
		strcpy(new_ent->d_name, "..");
		new_ent->d_inode = pa->inode_num;
		if(dirent_add(i_new, new_ent))
			return -1; //  new directory initialization fialed
		i_new->inode.link_num = 2;
	}

	free(new_ent);

	pa->inode.i_tv[1].tv_sec = time(NULL);
	i_new->status |= 0x04;  /* inode disk has changed! */

	iput(pa);

/*	if new node will beblock and character special file, 
 *	write major & minor device num to the disk inode.
 */
	i_new->inode.i_tv[0].tv_sec = time(NULL);
	i_new->inode.i_tv[1].tv_sec = time(NULL);
	iput(i_new);

	return 0;
}

/* try to create a new directory with path */
int eagle_sys_mkdir(const char * path, mode_t mode)
{
	int result = eagle_sys_mknod(path, 2, mode);
	if(result)
		return -1;
	return 0;
}

int eagle_sys_unlink(const char * path)
{
    //fprintf(stderr, "unlink begin!\n");
	char name[MAX_FNAME_LEN + 1];
	char parent[strlen(path)];

	if(get_parent(path, parent, name))
		return -1; // invalid path or the path is root!
	if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return -1; // path is a directory!

	inode_incore * incore = NULL;
	inode_incore * pinode = namei(parent);
	if(pinode == NULL || pinode->inode.itype != 2)
		return -1; // no such directory !

	dpackage * dpk = calloc(1, sizeof(dpackage));
	dir_ent * ent = dirent_get(pinode, dpk);

	if(! ent)
		return -1; // wrong while reading entry
	else {
		ent = dirent_get(NULL, dpk);
		ent = dirent_get(NULL, dpk);
	}
	while(ent) {
		if(strcmp(name, ent->d_name) == 0) {
			incore = iget(ent->d_inode);
			break;
		}
		ent = dirent_get(NULL, dpk);	
	}

	free(dpk);

	if(! incore) 
		return -1; // no such file !

	if(dirent_rm(pinode, incore->inode_num))
		return -1; // wrong while remove the entry from parent directory
	pinode->inode.i_tv[1].tv_sec = time(NULL);
	iput(pinode);

	incore->inode.link_num --;
	incore->status |= 0x04;
	iput(incore);
	
	fprintf(stderr, "leaving unlink...\n");
	return 0;
}

int eagle_sys_rmdir(const char * path)
{
	char name[MAX_FNAME_LEN + 1];
	char parent[strlen(path)];

	if(get_parent(path, parent, name))
		return -1; /* invalid path name */

	if(strcmp(name, ".") == 0)
		return -EINVAL;  /* cannot be "." ! */

	if(strcmp(name, "..") == 0)
		return -EEXIST;  /* cannot be ".." ! */

	inode_incore * dirp = namei(path);
	
	if(dirp == NULL)
		return -ENOENT; /* no such directory ! */

	if(dirp->inode.itype != 2)
		return -ENOTDIR; /* it's not a directory ! */

	dpackage *dpk = calloc(1, sizeof(dpackage));
	dir_ent * ent = dirent_get(dirp, dpk);

	if(ent == NULL)
		return -1; /* something is not right!! */

	ent = dirent_get(NULL, dpk);

	UINT pa_num = ent->d_inode;

	if((ent = dirent_get(NULL, dpk)))
		return -ENOTEMPTY;  /* trying to remove a non empty directory */
	
	free(dpk);

	/* now eligible for removal ! */
	inode_incore * pa = iget(pa_num);
	dirent_rm(pa, dirp->inode_num);
	pa->inode.i_tv[1].tv_sec = time(NULL);
	iput(pa);

	dirp->inode.link_num = 0;
	dirp->status |= 0x04;
	iput(dirp);

	return 0;
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

uint64_t eagle_sys_open(const char* path, int flags, mode_t mode)
{
     inode_incore* opened_inode = namei(path);

     if (strstr(path, "tflag")) {

     }

     if (strstr(path, "newfile39")) {
         fprintf(stderr, "Newfile 39!\n");
     }

     if (flags & O_CREAT) { //File Creation!
         if (opened_inode) {
            /*int i;
            for (i = 0; i < (opened_inode->inode).file_sz; )
            {
                int blk_id = bmap(opened_inode, i, NULL, NULL);
                db_free(blk_id);

                i += BLOCK_SIZE;
            }
            (opened_inode->inode).file_sz = 0;*/
            /* opened_inode->status = 0x04;

             iput(opened_inode);*/
             file_free(opened_inode);


         } else { // File does not exist
             opened_inode = ialloc();

	     if(! opened_inode)
		return (uint64_t) NULL;

             (opened_inode->inode).perms = mode;

             opened_inode->inode.itype = 1; //Regular file

             //Create new entry in parent directory
             dir_ent* p_new_entry = (dir_ent*)malloc(sizeof(dir_ent));

             p_new_entry->d_inode = opened_inode->inode_num;
             char file_name[MAX_FNAME_LEN + 1];
             char paren_name[strlen(path)];

             int ret = get_parent(path, paren_name, file_name);
             if (ret!=0) {
		 free(p_new_entry);
                 return (uint64_t)NULL;
	     }

             strcpy(p_new_entry->d_name, file_name);

             inode_incore* paren_node = namei(paren_name);

             ret = dirent_add(paren_node, p_new_entry);

             paren_node->status |= 0x04;
             iput(paren_node);

             if(ret!=0) {
		 free(p_new_entry);
                 return (uint64_t)NULL;
	     }
	     free(p_new_entry);
            }

     } else { //File Open!
         if (!opened_inode)
             return (uint64_t)NULL;

        //if (flags & O_TRUNC) {
            /* int i;
             for (i = 0; i < (opened_inode->inode).file_sz; )
             {
                 int blk_id = bmap(opened_inode, i, NULL, NULL);
                 db_free(blk_id);

                 i += BLOCK_SIZE;
             }
             (opened_inode->inode).file_sz = 0;*/
        /*     opened_inode->status = 0x04;

             iput(opened_inode);

             opened_inode = ialloc();
         }*/


     }

     // opened_inode->ref_count++;

     opened_inode->inode.i_tv[0].tv_sec = time(NULL);

     pthread_mutex_unlock(&(opened_inode->lock));
     return (uint64_t)opened_inode;
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

    int max_file_len = DIRECT_NUM*BLOCK_SIZE*sizeof(char) + (SINGLE_NUM*BLOCK_SIZE*FREE_BLK_NODE_SZ)*sizeof(char) + DOUBLE_NUM*FREE_BLK_NODE_SZ*FREE_BLK_NODE_SZ*BLOCK_SIZE*sizeof(char);

    if ((pinode->inode.file_sz == offset)&&
            (pinode->inode.file_sz+size > max_file_len)) {
            fprintf(stderr, "Exceed file size limit!\n");
            return -EFBIG;
}

    int j;
    int error_bmap = 0;

    for (j = 0; j < size; )
    {
        UINT in_offset, num_left;
        //if (offset >= 8447)
         //   printf("here!\n");

        int blk_id = bmap(pinode, offset, &in_offset, &num_left);

        if (blk_id <= 0) {
            error_bmap = 1;
            break;
        }


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

   // if (error_bmap)
   //     return -ENOSPC;

    return j;
}

//Assume user have root permission
int eagle_sys_access(const char* path, int mask)
{
    inode_incore* pinode = namei(path);
    if (!pinode)
        return -1;

    mode_t i_mode = pinode->inode.perms;
    iput(pinode);

    if (((mask & R_OK)&&(!(i_mode & S_IREAD)))||
        ((mask & W_OK)&&(!(i_mode & S_IWRITE)))||
        ((mask & X_OK)&&(!(i_mode & S_IEXEC))))
        return -1;

    return 0;
}
