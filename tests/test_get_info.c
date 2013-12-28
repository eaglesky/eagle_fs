#include "../fsconfig.h"
#include "../spblk.h"

int load_disk(char* disk_path)
{
    if ((disk_fd = open(disk_path, O_RDWR)) == -1) {
        fprintf(stderr, "Disk loaded error!\n");
        return -1;
    }

    fprintf(stderr, "Disk loaded!\n");
    return 0;
}

int main(int argc, char* argv[])
{
	if ((argc < 2)||(argv[argc-1][0] == '-')) {
	        fprintf(stderr, "Last two argument must be path of device and path of mount point\n");
	        return -1;
	}


	char* disk_path = realpath(argv[argc-1], NULL);

	if (!disk_path) {
	        fprintf(stderr, "Disk path is not correct!\n");
	        return -1;
	}

	if (load_disk(disk_path) == -1)
		return -1;

 //   mkfs();
	char flag_string[BLOCK_SIZE];
	disk_read(flag_string, 0, 1);

	if (strncmp(flag_string, "Eagle", strlen("Eagle"))) {
	        fprintf(stderr, "Call eagle_mkfs first!\n");
	        close(disk_fd);
	        return -1;
	}

	super_read();

	printf("%d  %d", INODE_NUM_PER_BLK, sp_blk.free_inodes_num);
}
