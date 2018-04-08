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

int inode = 2;
unsigned char *disk;
struct ext2_super_block *sb;
struct ext2_group_desc *b;
struct ext2_dir_entry_2 *dir;
struct ext2_inode * inodetable;
unsigned char * blockbitmap;
unsigned char * inodebitmap;
char *dirname;

char type(unsigned short i_mode){
	unsigned short type = i_mode & 0xf000;
	if (type == 0x8000){
		return 'f';
	}else if(type == 0x4000){
		return 'd';
	}
	return 0;
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

char dir_type(unsigned short type){
	if (type == 2){
		return 'd';
	}else if(type == 1){
		return 'f';
	}
	return 0;
}


//get one free inode for the new directory, return the inode number
int free_inode(){
	int inode_found = -1;
    int inode_index = 0;
    int inode_available = 0;
    int position = 0;
    while(inode_found == -1  && inode_index<(sb->s_inodes_count/8)){
		
		const char* inode_bit = byte_to_binary(inodebitmap[inode_index]);
		int i = 0;
		for (;i<8;i++){
			
			inode_available += 1;
			if (inode_bit[i] == '0'){
				inode_found = 1;
				inode_index -= 1;
				position = i;
				b->bg_free_inodes_count -= 1;
				break;
			}
		}
		inode_index += 1;
		
	}
	if(inode_found == -1  && inode_index == (sb->s_inodes_count/8)){
		return ENOSPC;
	}
	int result = pow(2,position);
    inodebitmap[inode_index] = inodebitmap[inode_index] ^ result;
    
    return inode_available;
}

//get one free block for the new directory,return the block number
int free_block(){
	int block_found = -1;
    int block_index = 0;
    int block_available = 0;
    int block_position = 0;
    while(block_found == -1  && block_index<(sb->s_blocks_count/8)){
		
		const char* block_bit = byte_to_binary(blockbitmap[block_index]);
		int i = 0;
		for (;i<8;i++){
			
			block_available += 1;
			if (block_bit[i] == '0'){
				block_found = 1;
				block_index -= 1;
				block_position = i;
				b->bg_free_blocks_count -= 1;
				break;
			}
		}
		block_index += 1;
		
	}
	if(block_found == -1  && block_index == (sb->s_blocks_count/8)){
		return ENOSPC;
	}
	int result = pow(2,block_position);
    blockbitmap[block_index] = blockbitmap[block_index] ^ result;
    
    return block_available;
}

//copy from stack overflow
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
	char url_new[length_of_url +2];

	if (url[0] == '/'){
		url_new[0] = '.';
		url_new[1] = '.';
		strcat(url_new,url);
		strcpy(url,url_new);
	}
	return url;
}

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
//for search input url
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
	dirname = paths[length-1];
	while (index < length-1){
		char* path = paths[index];
		//printf("name:%s\n",path);
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
		char* url = argv[2];
		//Deal with url
		url = input(url);
		
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
		dir = (struct ext2_dir_entry_2 *)(disk + (&inodetable[1])->i_block[0] * 1024); 
		blockbitmap  = b->bg_block_bitmap * EXT2_BLOCK_SIZE +disk;
		inodebitmap  = b->bg_inode_bitmap * EXT2_BLOCK_SIZE +disk;
		//search
		int remain, inode_available, block_available;
		int temp = 0;
		//search the url and get the free inode number and block number for new directory
		if(search_r(url)!=0){
			inode_available = free_inode(); 
			block_available = free_block();
			int count = 0;
			while (count<1024){
				if ((strcmp(dir->name,dirname)==0)){
					return EEXIST;
				}
				if(temp <= dir->rec_len && (count + dir->rec_len < 1024)){
					temp = dir->rec_len;
				}
				count += dir->rec_len;
				remain = dir->rec_len;
				//refresh the address of dir entry
				dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + count);			
			}
			dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + (1024 - remain) );
			int name_len;
			//set the rec_len
			if((dir->name_len + 8)%4 != 0){
				name_len = ((dir->name_len + 8)/4 + 1)*4 ;
			}else{
				name_len = ((dir->name_len + 8)/4)*4;
			}
			
			dir->rec_len = name_len;
			remain = remain - name_len;
			//new directory
			dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + (1024 - remain));
			dir->rec_len = remain;
			dir->file_type = 2;
			dir->inode = inode_available;
			
			dir->name_len = strlen(dirname);
			strcpy(dir->name,dirname);
		}else{
			return ENOENT; 

		} 
		//make block for . and .., set all the data 
	    int inode_number = inode_available;
	    (&inodetable[inode_number-1])->i_size = 1024;
	    (&inodetable[inode_number-1])->i_mode = 0x4fff;
	    (&inodetable[inode_number-1])->i_blocks = 2;
	    (&inodetable[inode_number-1])->i_block[0] = block_available;
	    (&inodetable[inode_number-1])->i_links_count =1;
	    struct ext2_dir_entry_2 *new_dir = (struct ext2_dir_entry_2 *)(disk + (&inodetable[inode_number-1])->i_block[0] * 1024); 
	    (&inodetable[inode_number-1])->i_links_count +=1;
	    new_dir->inode = inode_number;
	    new_dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode_number-1])->i_block[0] * 1024));
	    new_dir->file_type = 2;
	    new_dir->inode = inode_number;
	    new_dir->rec_len = 12;
	    new_dir->name_len = 1;
	    strcpy(new_dir->name,".");
	    new_dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode_number-1])->i_block[0] * 1024) + 12);
	    (&inodetable[inode-1])->i_links_count +=1;
	    new_dir->file_type = 2;
	    new_dir->inode = inode;
	    new_dir->rec_len = 1024 - 12;
	    new_dir->name_len = 1;
	    strcpy(new_dir->name,"..");
	    
	    
    return 0;
}





