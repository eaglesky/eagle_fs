#include "dir.h"
#include "diskio.h"
#include "datablk.h"
#include "syscalls.h"

#include <stdlib.h>
#include <string.h>



/* dirent_get: get one directory entry each call , directory is specified by inocre inode, 
 * NULL should be provided in the successive calls
 */
dir_ent * dirent_get(const inode_incore *i_incore, dpackage * dpk)
{
	int reclen = sizeof(dirent_d);
	dirent_d ent;
	dpk->dirent.d_length = reclen;

	if(i_incore) {
		dpk->incore = i_incore;
		dpk->offset = 0;
	}

	if(! dpk->incore)   /* valid incore inode should be provided every first call */
		return NULL; 

	while(1) {

		if(dpk->offset == dpk->incore->inode.file_sz)
			return NULL;  /* no more entry in this directory! */

		int rd_num;
		if((rd_num = eagle_sys_read((uint64_t)(dpk->incore), (char *) &ent, reclen, dpk->offset))<=0)
			return NULL; // cannot read entry!

		dpk->offset += reclen;
		if(ent.d_inode == 0 && strcmp(ent.d_name, ".") != 0 && strcmp(ent.d_name, "..") != 0) 
			continue;
		
		dpk->dirent.d_inode = ent.d_inode;
		strcpy(dpk->dirent.d_name, ent.d_name);
		dpk->dirent.d_offset = dpk->offset;
		return &(dpk->dirent);		
	}
}


/* Add an entry into the directory, inode_incore point of the directory is dirp */
int dirent_add(inode_incore * dirp, const dir_ent * entry)
{
	if(! dirp)
		return -1; // invalid directory inode inocore !
	
	dirent_d ent_d;
	int reclen = sizeof(dirent_d);

	ent_d.d_inode = entry->d_inode;
	strcpy(ent_d.d_name, entry->d_name);


/*	dpackage * dpk = calloc(1, sizeof(dpackage));
	dir_ent * ent = dirent_get(dirp, dpk);

	if(ent) {      // skip the directory itself "." and the parent ".."
		ent = dirent_get(NULL, dpk);
		ent = dirent_get(NULL, dpk);
	}

	while(ent) {
		if(ent->d_inode == 0) {
            if((eagle_sys_write((uint64_t)dirp, (char *) &ent_d, reclen, ent->d_offset - reclen)) <= 0)
				return -1;  // can not write entry!
			return 0;
		}
		ent = dirent_get(NULL);
	}
*/

    if((eagle_sys_write((uint64_t)dirp, (char *) &ent_d, reclen, dirp->inode.file_sz))<=0)
		return -1;

	dirp->status |= 0x04;

	return 0;
}	

/* remove an entry in directory */
int dirent_rm(inode_incore * dirp, UINT i_num) 
{
    //fprintf(stderr, "dirent_rm begin!\n");
	dpackage * dpk = calloc(1, sizeof(dpackage));
	dir_ent * ent = dirent_get(dirp, dpk);

	while(ent) {
		if(ent->d_inode == i_num) {
/*
			dirent_d ent_d;
			ent_d.d_inode = 0;
			ent_d.d_name[0] = '\0';
			eagle_sys_write((uint64_t)dirp, (char*) &ent_d, ent->d_length, ent->d_offset - ent->d_length);
			dirp->status |= 0x04;
			return 0;
*/
			break;
		}
		ent = dirent_get(NULL, dpk);
	}
	if(! ent) 
		return -1;  // No such entry in given directory !

	UINT new_size = dirp->inode.file_sz - ent->d_length;
	char *buf = malloc(dirp->inode.file_sz);
	eagle_sys_read((uint64_t)dirp, buf, dirp->inode.file_sz, 0);
	char *buf_new = malloc(new_size);
	memcpy(buf_new, buf, ent->d_offset - ent->d_length);
	memcpy(buf_new + ent->d_offset - ent->d_length, buf + ent->d_offset, dirp->inode.file_sz - ent->d_offset);
	free(buf);

	file_free(dirp);
	eagle_sys_write((uint64_t)dirp, buf_new, new_size, 0);
	free(buf_new);
	dirp->status |= 0x04;

	free(dpk);
	fprintf(stderr, "leaving dirent...\n");
	return 0;
}

int get_parent(const char * path, char * parent, char * name) 
{
	char * last = strrchr(path, '/');
	if(last == NULL || *path != '/' || strcmp(path, "/") == 0)
		return -1;  // no hierarchy in the path or not absolute path or path is root

	int tail = 0;
	if(last - path == strlen(path) - 1) {
		tail = 1;
		do {
			 last--;
		} while(*last != '/');
	}
	int leng = last -path;
	if (leng == 0)
		leng = 1;
	strncpy(parent, path, leng);
	parent[leng] = '\0';
	if (leng == 1)
		leng = 0;
	int name_leng = strlen(path) - leng - 1 - tail;
	strncpy(name, last + 1, name_leng);
	name[name_leng] = '\0';

	return 0;
}
