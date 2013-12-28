#include "fsconfig.h"

void data_blk_init();
UINT db_alloc();

//MAKE SURE THE BLK_NUM IS NOT ON THE FREE LIST!!
int db_free(UINT blk_num);
int bmap(inode_incore * incore, off_t offset, UINT * const in_offset, UINT * const num_IO);
int file_free(inode_incore * incore);
