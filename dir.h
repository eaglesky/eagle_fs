#include "fsconfig.h"

typedef struct {
	int offset;
	const inode_incore* incore;
	dir_ent dirent;
} dpackage;

int get_parent(const char *path, char * parent, char * name);
dir_ent * dirent_get(const inode_incore * i_incore, dpackage * dpk);
int dirent_add(inode_incore * dirp, const dir_ent * entry);
int dirent_rm(inode_incore * dirp, UINT i_num);
