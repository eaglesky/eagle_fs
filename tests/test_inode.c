#include "../inode.h"
#include "../fsconfig.h"
#include "../spblk.h"

pthread_mutex_t sp_lock;

static int test_hash_queue(FILE * file)
{
	inode_incore *in[INODE_HASH_SLOTS*2];
	
	int i, j;
	for(i = 0; i < INODE_HASH_SLOTS*2; i++) {
		for(j = 0; j < i; j++) {
			iget(i);
		}
		in[i] = iget(i);
	}
	
	fprintf(file, "\niget operation test:\n");
	for(i = 0; i < INODE_HASH_SLOTS*2; i++) {
	fprintf(file, "\ninode: \t%d\t|\trun iget times:\t%d\nref_count:\t%d\nfile_type:\t%d\n", in[i]->inode_num, i+1, in[i]->ref_count, in[i]->inode.itype);

	if(in[i]->next_free || in[i]->previous_free)
		fprintf(file, "After iget: Free list is wrong!\n");
	else 
		fprintf(file, "After iget, free list remains consistent!\n");

	if(in[i]->next_hash == NULL && i < INODE_HASH_SLOTS)
		fprintf(file, "After iget, hash queue remains consistent!\n");
	else if(in[i]->next_hash != NULL && i >= INODE_HASH_SLOTS)
		fprintf(file, "After iget, hash queue remains consistent!\n");
	else
		fprintf(file, "After iget: Hash queue is wrong!\n");
		
//	if(i_hash[2].next_hash != NULL)
//		fprintf(file, "Need initialization\n");
	}
	return 0;
}

static int should_refill_spblk_free_inode_lst(FILE * file)
{
	int i;
	fprintf(file, "\nialloc operation test:\noriginal inode free list in super blk:\n");
	for(i = 0; i < SP_INODES_SZ; i++) {
		fprintf(file, "%d\t", sp_blk.free_inodes_lst[i]);
	}
	fprintf(file, "\n\nsuper blk status:\ni_index:\t%d\nfree_inodes_num:\t%d\n", sp_blk.i_index, sp_blk.free_inodes_num);


	inode_incore *in[1];

	for(i = 0; i < SP_INODES_SZ + 3 ; i ++) {
		in[0] = ialloc();
//		in[0]->inode.itype = 1;
//		in[0]->inode.link_num = 0;
		iput(in[0]);
	}

/*	if(in[0]->next_free == NULL || in[0]->previous_free ==NULL)
 *		fprintf(file, "Something wrong with free list!\n");
 *	in[1] = iget(0);
 *	if(in[1] == in[0]) 
 *		fprintf(file, "\nRight!\n");
 */
	fprintf(file, "\n\nafter ialloc has used up all the inodes in super blk:\n");
	for(i = 0; i < SP_INODES_SZ; i++) {
		fprintf(file, "%d\t", sp_blk.free_inodes_lst[i]);
	}

	fprintf(file, "\n\nsuper blk status:\ni_index:\t%d\nfree_inodes_num:\t%d\n", sp_blk.i_index, sp_blk.free_inodes_num);

	return 0;
}

int load_disk(char* disk_path)
{
    if ((disk_fd = open(disk_path, O_RDWR)) == -1) {
        fprintf(stderr, "Disk loaded error!\n");
        return -1;
    }

    fprintf(stderr, "Disk loaded!\n");
    return 0;
}

int main(int argc, char *argv[])
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

	pthread_mutex_init(&sp_lock, NULL);
	inode_incore_init();

	FILE * file = fopen("Outputs/test_inode.txt", "w");

	super_print();
	test_hash_queue(file);
	should_refill_spblk_free_inode_lst(file);

	i_incore_destroy();
	close(disk_fd);
}
