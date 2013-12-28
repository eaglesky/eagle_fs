#!/bin/bash
# test the file system by using up the disk inodes!

tmp=($( ./info $1 ))

echo -e "${tmp[1]} free inodes left\nTrying to use it up..."

name="newfile"
cd $2

if [ tmp ]; then
	for i in $( seq 1 ${tmp[1]} ); do
		touch  $name$i;
	done
else
	exit -1
fi

echo "Done!"
ls .

echo "Trying to create one more file..."

touch one_more

echo "Try again..."

touch another_one_more

echo "Cleaning up..."

for i in $( seq 1 ${tmp[1]} ); do
	rm -v $name$i ;
done

ls .
