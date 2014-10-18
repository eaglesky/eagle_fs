Eagle File System

By: Yalun Qin(yalun@umail.ucsb.edu)
       Junsheng Guo(junsheng.guo90@gmail.com)
       Bin Xu(bin_xu@umail.ucsb.edu)

Phase 1: 

Requirements completed. Our Eagle File System passed all the test we made, therefore it  works well on our laptops, and should will pass other tests , 
if God bless us. ^_^

Our tests have two types.  Since data block tests and bmap tests are more concerned with the lower structures in our file system, we put them in tests.c among
all the other fs kernel sources.  Test_bmap has no output files since we just use gdb to see the values. Test_data_blk generates disk files for debugging when our
file system size is  set to be 12800. Other test codes are in the folder tests. And all the output files are in the folder tests/Outputs. 

Supported system calls: 
open, close, read, write, mkdir, rmdir,  mknod, unlink, readdir.

Phase 2:

We separate mkfs function from previous code in this phase and integrate it into our new
program "eagle_mkfs". This program is used to format the disk, so it must be
called at least once before mounting our file system to a new disk. Also note that
this program takes one argument which is the path of the disk.

Our main program is still "fs". The mount command is: ./fs [-d] path_of_disk
path_of_mount_point. Unmount command is not changed.

Our test routines are still located in the tests folder. The test code check the
correctness of mkfs, data blocks management, inodes management, large file
support and use of commands like ls, touch, vim, dd, mkdir, rm, cp. We also
tested the persistence. Our file system passed all the tests.

Phase 3:

In this phase, our file system supports more commands like tar, chmod and chown. We didn't do well in our presentation because we forgot to test the "gcc" 
command. After that, we fixed that problem and now Professor Wolski's test files can be run in our file system, except that there are still some errors
when compilling the file "c-file-test2.c".  We compiled this file on usual linux system and the same errors appeared. Others like Bonnie works fine on our file
system.

We changed the block size from 512 bytes to 4048 bytes, which greatly shortens the running time of eagle_mkfs program.

Our test programs are all located in the "tests" subfolder. 
1. ./test_mkfs /dev/vdb  
     Used to test the content of disk.

2. ./test_all.sh /dev/vdb ../mnt
     Used to test operations about inodes and datablocks.

3. We tested other commands like cp(-r), rm(-r), tar, gcc manually. To test "gcc", we copied the folder containing our project code to our file system and ran
"make" which worked successfully.

4. Our file system also supports multi-thread access, since we added multi-thread access protection in this phase. We tested it by manually opening multiple
windows and tried to read or write files at the same time. 

Out test results are located in tests/Outputs. We also put the test results on our 2-G instance of Professor Wolski's test programs in it(tests_results.txt).
