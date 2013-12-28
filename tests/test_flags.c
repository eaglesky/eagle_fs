/* This program is used to test access control for the flags of open. It also
 * tests wheather the file descripor works correctly or not. Correct answers
 * are showed in the comments. Answers given by our file system are captured
 * and saved in Output folder.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    FILE* fp1, *fp2, *fp3;
    char *buf1 = (char*)malloc(5*sizeof(char));
    char *buf2 = (char*)malloc(5*sizeof(char));
    char *buf3 = (char*)malloc(5*sizeof(char));
    char *buf4 = (char*)malloc(5*sizeof(char));
    char *buf5 = (char*)malloc(5*sizeof(char));

    char *buf = "Hello World!\nWe are heros!\nHaha, I am so happy!\n";

    if (argc < 2) {
        printf("Please specify path of mount point:\n");
        return -1;
    }
    printf("Mount point: %s\n", argv[1]);

    char file_name[100];
    strcpy(file_name, argv[1]);
    strcat(file_name, "/test_flags.txt");

    fp1 = fopen(file_name, "wt");

    fread(buf1, sizeof(char), 5, fp1);

    fwrite(buf, sizeof(char), strlen(buf), fp1);

    fread(buf2, sizeof(char), 5, fp1);

    fclose(fp1);

    fp2 = fopen(file_name, "rt");
    fread(buf3, sizeof(char), 5, fp2);

    fp3 = fopen(file_name, "rw");
    fread(buf4, sizeof(char), 5, fp3);

    fread(buf5, sizeof(char), 5, fp2);


    printf("buf1: %s\n", buf1);  //buf1 should be empty
    printf("buf2: %s\n", buf2);  //buf2 should be empty
    printf("buf3: %s\n", buf3);  //buf3 should be "hello"
    printf("buf4: %s\n", buf4);  //buf4 should be "hello"
    printf("buf5: %s\n", buf5);  //buf5 should be " Worl"


    free(buf1);
    free(buf2);
    free(buf3);
    free(buf4);
    free(buf5);

    fclose(fp2);
    fclose(fp3);
    return 0;
}
