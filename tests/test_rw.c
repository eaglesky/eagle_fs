/* This program is used to test open, read and write system calls, management
 * of direct and indirect blocks and data blocks, including bmap function.
 * Output files are located in Output folder, which are all correct.
 **/

#include "../fsconfig.h"

int main(int argc, char** argv)
{
    /*FILE* fp;

    if (argc < 2) {
        printf("Please specify path of mount point:\n");
        return -1;
    }
    printf("Mount point: %s\n", argv[1]);

    char file1[100];
    char file2[100];
    char file3[100];
    char file4[100];

    strcpy(file1, argv[1]);
    strcat(file1, "/file_rw1.txt");

    strcpy(file2, argv[1]);
    strcat(file2, "/file_rw2.txt");

    strcpy(file3, argv[1]);
    strcat(file3, "/file_rw3.txt");

    strcpy(file4, argv[1]);
    strcat(file4, "/file4_exceed.txt");*/

    int len1 = DIRECT_NUM*BLOCK_SIZE*sizeof(char);
    int len2 = (SINGLE_NUM*BLOCK_SIZE*FREE_BLK_NODE_SZ)*sizeof(char);
    long unsigned int len3 = DOUBLE_NUM*FREE_BLK_NODE_SZ*FREE_BLK_NODE_SZ*BLOCK_SIZE
            *sizeof(char);

    printf("%d %d %ld\n", len1, len2, len3);
 /*   printf("len1 = %d\n", len1);
    printf("len2 = %d\n", len2);
    printf("len3 = %ld\n", len3);

    char* buf = (char*)malloc(len1+len2+len3);
    memset(buf, 'a', len1);
    memset(buf+len1, 'b', len2);
    memset(buf+len1+len2, 'c', len3);

    long unsigned int written_sz;
    fp = fopen(file1, "wt+");
    if (!fp) {
        printf("Error when creating file rw1\n");
        return -1;
    }
    written_sz = fwrite(buf, len1-1, 1, fp);
    printf("file rw1 size: %ld\n", written_sz);
    fclose(fp);

    fp = fopen(file2, "wt+");
    if (!fp) {
        printf("Error when creating file rw2\n");
        return -1;
    }
    written_sz = fwrite(buf, len1+len2-1, 1, fp);
    printf("file rw2 size: %ld\n", written_sz);
    fclose(fp);

    fp = fopen(file3, "wt+");
    if (!fp) {
        printf("Error when creating file rw3\n");
        return -1;
    }
    written_sz = fwrite(buf, len1+len2+len3-1, 1, fp);
    printf("file rw3 size: %ld\n", written_sz);


    char* buf1 = "Operating Systems!";
    fseek(fp, DIRECT_NUM*BLOCK_SIZE + 1, SEEK_SET);
    fwrite(buf1, sizeof(char), strlen(buf1), fp);
    fclose(fp);


    fp = fopen(file4, "wt");
    if (!fp) {
        printf("Error when creating file 4\n");
        return -1;
    }
    long unsigned int extra_size = (len1+len2+len3+1);
    char *buf_extra = (char*)malloc(extra_size);
    written_sz = fwrite(buf_extra, extra_size, 1, fp);
    printf("File 4 size: %ld; Exceeding the limit!\n", written_sz);

    free(buf);
    free(buf_extra);*/
    return 0;
}
