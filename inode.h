#include "fsconfig.h"

void inode_lst_init();
int inode_incore_init();
inode_incore * iget(UINT i_num);
void iput (inode_incore *incore);
inode_incore *ialloc();
void ifree(UINT i_num);
int inode_rblk(inode_incore* incore);
int inode_wblk(inode_incore* incore);
void i_incore_destroy();
inode_incore * namei(const char * name_p);
