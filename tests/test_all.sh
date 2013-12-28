#!/bin/bash
# Test all functions of eagle file system

echo disk path: $1
echo mount point: $2 "\n"

echo Testing creating files of different sizes:
args=($( ./test_rw))
echo Max size supported by one direct block: ${args[0]}
echo Max size supported by one single indirect block: ${args[1]}
echo Max size supported by one double indirect block: ${args[2]}
echo Create a file using direct block: 
dd if=/dev/zero of=$2/file_direct bs=$((${args[0]}-1)) count=1
echo

echo Create a file using single indirect block: 
dd if=/dev/zero of=$2/file_single_indirect bs=$((${args[0]}+${args[1]}-1)) count=1
echo

echo Create a file using double indirect block: 
dd if=/dev/zero of=$2/file_double_indirect bs=$((${args[0]}+${args[1]}+10)) count=1
echo

echo Try to create a file with maximum size supported by eagle file system:
dd if=/dev/zero of=$2/file_max bs=$((${args[0]}+${args[1]}+${args[2]}-1)) count=1
echo

echo result:
ls -l $2
echo

echo Testing read and write flags:
./test_flags $2

echo Using up the disk inodes :
./test_use_up_inode.sh  $1 $2

