#include "fsconfig.h"

int eagle_sys_mknod(const char * path, file_type itype,  mode_t mode);
int eagle_sys_mkdir(const char * path, mode_t mode);
int eagle_sys_unlink(const char * path);
int eagle_sys_rmdir(const char * path);
int eagle_sys_mkroot();
uint64_t eagle_sys_open(const char* path, int flags, mode_t mode);
int eagle_sys_read(uint64_t fd, char *buf, size_t size, off_t offset);
int eagle_sys_write(uint64_t fd, const char *buf, size_t size, off_t offset);
int eagle_sys_access(const char* path, int mask);


