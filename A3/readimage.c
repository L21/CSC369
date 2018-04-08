#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

unsigned char *disk;

char type(unsigned short i_mode){
	unsigned short type = i_mode & 0xf000;
	if (type == 0x8000){
		return 'f';
	}else if(type == 0x4000){
		return 'd';
	}
	return 0;
}
char dir_type(unsigned short type){
	if (type == 2){
		return 'd';
	}else if(type == 1){
		return 'f';
	}
	return 0;
		

}

void print_blocks(struct ext2_inode *inode){
	if (inode->i_size > 1024*12){
		int block_number = inode->i_size /1024;
		if ((inode->i_size % 1024) != 0){
			block_number += 1;
		}
		int i = 0;
		for (;i< 12;i++){
			printf("%d ", inode->i_block[i]);
		}
		int* block_array = (int*)(disk + 1024 * inode->i_block[12]);
		for  (i = 12; i < block_number;i++){
			printf("%d ", block_array[i-12]);
		}
		printf("\n");
		
	}else{
		int block_number = inode->i_size /1024;
		if ((inode->i_size % 1024) != 0){
			block_number += 1;
		}
		int i = 0;
		for (;i< block_number;i++){
			printf("%d ", inode->i_block[i]);
		}
		printf("\n");
	}
}
//from stackoverflow
//http://stackoverflow.com/questions/198199/how-do-you-reverse-a-string-in-place-in-c-or-c
void binary_reverse(char *p)
{
  char *q = p;
  while(q && *q) ++q;
  for(--q; p < q; ++p, --q)
    *p = *p ^ *q,
    *q = *p ^ *q,
    *p = *p ^ *q;

  return ;
}

//from stackoverflow
//http://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
const char *byte_to_binary(int x){
    static char binary[9];
    binary[0] = '\0';
    int z;
    for (z = 128; z >0; z >>= 1)
    {
        strcat(binary, ((x & z) == z) ? "1" : "0");
    }
    binary_reverse(binary);
    return binary;

}

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: readimg <image file name>\n");
        exit(1);
    }
    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
	perror("mmap");
	exit(1);
    }

    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    struct ext2_group_desc *b = (struct ext2_group_desc *)(disk + 1024 + 1024);
    printf("Inodes: %d\n", sb->s_inodes_count);
    printf("Blocks: %d\n", sb->s_blocks_count);
    printf("Block group:\n");
    printf("    block bitmap: %d\n",b->bg_block_bitmap);
    printf("    inode bitmap: %d\n",b->bg_inode_bitmap);
    printf("    inode table: %d\n",b->bg_inode_table);
    printf("    free blocks: %d\n",b->bg_free_blocks_count);
    printf("    free inodes: %d\n",b->bg_free_inodes_count);
    printf("    used_dirs: %d\n",b->bg_used_dirs_count);
    printf("Block bitmap: %d\n", sb->s_inodes_count);
    printf("Inode bitmap: %d\n", sb->s_blocks_count);
    //int block_number = sb->s_blocks_count/sb->s_blocks_per_group;
    //printf("#block: %d\n", sb->s_blocks_per_group);
    unsigned char * blockbitmap  = b->bg_block_bitmap * EXT2_BLOCK_SIZE +disk;
    int index = 0;
    printf("Block bitmap: ");
    for(;index<(sb->s_blocks_count/8);index++){
		printf("%s ",byte_to_binary(blockbitmap[index]));

    }
    printf("\n");
    unsigned char * inodebitmap  = b->bg_inode_bitmap * EXT2_BLOCK_SIZE +disk;
    int idx = 0;
    printf("Inode bitmap: ");
    
    for(;idx<(sb->s_inodes_count/8);idx++){
		printf("%s ",byte_to_binary(inodebitmap[idx]));

    }
    
    printf("\n\n");
	struct ext2_inode * inodetable =(struct ext2_inode  *) (b->bg_inode_table * EXT2_BLOCK_SIZE +disk);
	printf("Inodes: \n");
	printf("[2] type: %c size: %d links: %d blocks: %d\n", type((&inodetable[1])->i_mode),(&inodetable[1])->i_size,(&inodetable[1])->i_links_count,(&inodetable[1])->i_blocks);
	printf("[2] Blocks: %d\n", (&inodetable[1])->i_block[0]);
	int num_block  =11;
	for(; num_block < sb->s_inodes_count; num_block++){
		if((&inodetable[num_block])->i_size >0){
			printf("[%d] type: %c size: %d links: %d blocks: %d\n", num_block+1,type((&inodetable[num_block])->i_mode),(&inodetable[num_block])->i_size,(&inodetable[num_block])->i_links_count,(&inodetable[num_block])->i_blocks);
			printf("[%d] Blocks: ",num_block +1);
			print_blocks(&inodetable[num_block]);
		}
	}
	
	printf("\n");
	

	struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + (&inodetable[1])->i_block[0] * 1024);
	printf("Directory Blocks:\n");
	printf("   DIR BLOCK NUM: %d (for inode 2)\n", (&inodetable[1])->i_block[0]);
	int count = 0;
	while(count < 1024){
    	printf("Inode: %d ", dir->inode);
    	printf("rec_len: %d ", dir->rec_len);
    	printf("name_len: %u ", dir->name_len);
    	printf("type=: %c ",dir_type( dir->file_type));
    	printf("name=: ");
    	int len = 0;
    	for(; len < dir->name_len; len++){
    		printf("%c", dir->name[len]);
    	}
    	printf("\n");
    	count += dir->rec_len;
    	dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[1])->i_block[0] * 1024+count));
    	
    }
    
    num_block  =11;
	for(; num_block < sb->s_inodes_count; num_block++){
		if((&inodetable[num_block])->i_size >0 && (type((&inodetable[num_block])->i_mode) == 'd')){
			struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + (&inodetable[num_block])->i_block[0] * 1024);
			printf("   DIR BLOCK NUM: %d (for inode %d)\n", (&inodetable[num_block])->i_block[0],dir->inode);
			count = 0;
			while(count < 1024){
    			printf("Inode: %d ", dir->inode);
    			printf("rec_len: %d ", dir->rec_len);
    			printf("name_len: %u ", dir->name_len);
    			printf("type=: %c ",dir_type(dir->file_type));
    			printf("name=: ");
    			int len = 0;
    			for(; len < dir->name_len; len++){
    				printf("%c", dir->name[len]);
    			}
    			printf("\n");
    			count += dir->rec_len;
    			dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[num_block])->i_block[0] * 1024+count));
    		}
		}
	}
    
    
    return 0;
    
    
}
