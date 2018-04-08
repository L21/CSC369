 all:
	gcc -Wall -c "readimage.c"
	gcc -Wall -o "readimage" "readimage.c" -lm
	gcc -Wall -c "ext2_ls.c"
	gcc -Wall -o "ext2_ls" "ext2_ls.c"
	gcc -Wall -c "ext2_mkdir.c"
	gcc -Wall -o "ext2_mkdir" "ext2_mkdir.c" -lm
	gcc -Wall -c "ext2_ln.c"
	gcc -Wall -o "ext2_ln" "ext2_ln.c"
	gcc -Wall -c "ext2_cp.c"
	gcc -Wall -o "ext2_cp" "ext2_cp.c" -lm
	gcc -Wall -c "ext2_rm.c"
	gcc -Wall -o "ext2_rm" "ext2_rm.c" -lm
 clean:
	-rm *.o $(objects)  readimage ext2_ls ext2_mkdir ext2_ln
 
