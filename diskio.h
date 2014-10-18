#include "fsconfig.h"

/* Results of Disk Functions
typedef enum {
    RES_OK = 0,		// 0: Successful
    RES_ERROR,		// 1: R/W Error
    RES_WRPRT,		// 2: Write Protected
    RES_NOTRDY,		// 3: Not Ready
    RES_PARERR		// 4: Invalid Parameter
} DRESULT;*/



int disk_read(void* buf, int blk_addr, int blk_num);
int disk_write(const void* buf, int blk_addr, int blk_num);
