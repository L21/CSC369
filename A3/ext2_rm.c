#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>
#include <assert.h>
#include <math.h>
#include <errno.h>
int block_numbers = 0;
int inode = 2;
int file_inode;
char *filename;
char *filename1;
unsigned char *disk;
struct ext2_super_block *sb;
struct ext2_group_desc *b;
struct ext2_dir_entry_2 *dir;
struct ext2_inode * inodetable;
unsigned char * blockbitmap;
unsigned char * inodebitmap;

//free the given block; reset the block bitmap
void reset_block(int block){
   int block_index = 0;
   block_index = block/8;
   if (block%8 ==0){
	   block_index -= 1;
   }
   int position = 0;
   position = (block-1)- (block_index)*8;
   int result = pow(2,position);
	
   blockbitmap[block_index] = blockbitmap[block_index]  ^ result;
   return ;
}
//free the given inode; reset the inode bitmap
void reset_inode(int inode){
	
   
   int inode_index = 0;
   inode_index = inode/8;
   if (inode%8 ==0){
	   inode_index -= 1;
   }
   int position = 0;
   position = (inode-1)- (inode_index)*8;
   int result = pow(2,position);
	
   inodebitmap[inode_index] = inodebitmap[inode_index] ^ result;
   return ;
}



char dir_type(unsigned short type){
	if (type == 2){
		return 'd';
	}else if(type == 1){
		return 'f';
	}
	return 0;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;
    /* Count how many elements will be extracted. */
    
     while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }
    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);
    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;
    result = malloc(sizeof(char*) * count);
   
    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);
        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }
    return result;
}

char* input(char* url){
	if (strcmp(url,"") == 0){
		perror("Empty path!");
	    exit(1);
	}
	int length_of_url = strlen(url);
	char *url_new = malloc(length_of_url +2);

	if (url[0] == '/'){
		url_new[0] = '.';
		url_new[1] = '.';
		strcat(url_new,url);
		strcpy(url,url_new);
	}
	free(url_new);
	return url;
}
//search the url
int search(char* path){
	int count = 0;
	//char **paths = str_split(path,'/');
	
	while(count < 1024){ 
		if(strcmp(dir->name,path)== 0 && dir_type(dir->file_type) == 'd'){	
			//printf("yangShu%d%s%s\n",strcmp(dir->name,path),path,dir->name);
			return dir->inode;	
        } 
		count += dir->rec_len;
		dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024+count));
    }
    return 0;
}

//search the input url
int search_r(char* url){
	int index = 0;
	int i = 0;
	int length = 1;
	for(;i < strlen(url);i++){
		if (url[i] == '/'){
			length += 1;
		}
    }
    if (url[strlen(url)-1] == '/'){
		length -= 1;
	}
	//file name here
	
	char **paths = str_split(url,'/');
	filename = paths[length-1];
	//printf("len %ld\n",sizeof(*paths));
	while (index < length-1){
		char* path = paths[index];
		
		int result = search(path);
		if (result!=0){
			inode = result;
			dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024));
		}else{
			return 0;
		}
		index += 1;
	}
	return 1;
}

int main(int argc, char **argv) {
	
		if(argc != 3) {
			fprintf(stderr, "Usage: readimg <image file name>\n");
			exit(1);
		}
		char* url1 = malloc(sizeof(argv[2]));
		strcpy(url1,argv[2]);
		
		//Deal with (url1:source)(url2:dest) 
		
		url1 = input(url1);
       
		
		//Deal with img
		int fd = open(argv[1], O_RDWR);
		disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if(disk == MAP_FAILED) {
			perror("mmap");
			exit(1);
		}
		
		//data
		sb = (struct ext2_super_block *)(disk + 1024);  
		b = (struct ext2_group_desc *)(disk + 1024 + 1024);
		inodetable =(struct ext2_inode  *) (b->bg_inode_table * EXT2_BLOCK_SIZE +disk);
		blockbitmap  = b->bg_block_bitmap * EXT2_BLOCK_SIZE +disk;
		inodebitmap  = b->bg_inode_bitmap * EXT2_BLOCK_SIZE +disk;
		dir = (struct ext2_dir_entry_2 *)(disk + (&inodetable[1])->i_block[0] * 1024); 
		
		//serach the first input	
		int last_rec_len = 0;
		
		if(search_r(url1)!=0){
			int count = 0;
			int found = 0;
	        while (count<1024){
				char *name = malloc(dir->name_len);
				strncpy(name,dir->name,dir->name_len);
				//get the inode of the file that must be removed
				if ((dir->file_type == 1) && (strcmp(name,filename)==0)){
					file_inode = dir->inode;
					
					found = 1;
				 }
				//reset the previous directory entry's rec_len
                if (found ==1){
					int rec_length = dir->rec_len;
					dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + count - last_rec_len);
					dir->rec_len += rec_length;
					break;
				}
				free(name);
				last_rec_len = dir->rec_len;
				count += dir->rec_len;
				dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + count);			
			}
			if(found == 0){
				return ENOENT;
			}
		}else{
			return ENOENT;
		}
		
		//find all the bolck as well as the block in the single indirectly, and set the memory of that blocks as 0;
        int block_number = 0;
        block_number = (&inodetable[file_inode-1])->i_size/1024;
        if ((&inodetable[file_inode-1])->i_size % 1024 != 0){
			block_number += 1;
		}
        if(block_number <= 12){
			int block_index = 0;
			for(; block_index < block_number; block_index++){
				int new_block = (&inodetable[file_inode-1])->i_block[block_index];
				memset(disk + new_block*1024,0,1024);
				reset_block(new_block);
			}
			
		}else{
			int block_index = 0;
			for(; block_index < 12; block_index++){
				int new_block = (&inodetable[file_inode-1])->i_block[block_index];
				memset(disk + new_block*1024,0,1024);
				reset_block(new_block);
				(&inodetable[file_inode-1])->i_block[block_index] = 0;
			}
			int* new_block_array = (int*)(disk + 1024 * (&inodetable[file_inode-1])->i_block[12]);
			for(;block_index < block_number;block_index++){
				int new_block = new_block_array[block_index - 12];
				memset(disk + new_block*1024,0,1024);
				reset_block(new_block);
			}
			reset_block((&inodetable[file_inode-1])->i_block[12]);
			(&inodetable[file_inode-1])->i_block[12] = 0;
		}
		//reset all the data for that inode
		(&inodetable[file_inode-1])->i_size = 0;
		(&inodetable[file_inode-1])->i_mode = 0;
		(&inodetable[file_inode-1])->i_blocks = 0;
		(&inodetable[file_inode-1])->i_links_count = 0;
		
		reset_inode(file_inode);
		
		return 0;
	}
		
