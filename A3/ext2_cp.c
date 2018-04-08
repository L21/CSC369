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
int is_file = 0;


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
//find a free block and return the block number
int free_block(){
	int block_found = -1;
    int block_index = 0;
    int block_available = 0;
    int block_position = 0;
    //printf("inode %d\n", sb->s_blocks_count/8);
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

//find a free inode and return the inode number
int free_inode(){
	int inode_found = -1;
    int inode_index = 0;
    int inode_available = 0;
    int position = 0;
   // printf("inode %d\n", sb->s_blocks_count/8);
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

char dir_type(unsigned short type){
	if (type == 2){
		return 'd';
	}else if(type == 1){
		return 'f';
	}
	return 0;
}

//from stackoverflow
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

int search_r1(char* url){
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
	char **paths = str_split(url,'/');
	//printf("len %ld\n",sizeof(*paths));
	while (index < length){
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
	
		if(argc != 4) {
			fprintf(stderr, "Usage: readimg <image file name>\n");
			exit(1);
		}
		char* url1 = malloc(sizeof(argv[2]));
		strcpy(url1,argv[2]);
		char* url2 = malloc(sizeof(argv[3]));
		strcpy(url2,argv[3]);
		//Deal with (url1:source)(url2:dest) 
		
		url1 = input(url1);
       
		url2 = input(url2);
		
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
		//serach url1
		
		if(search_r(url1)!=0){
			int count = 0;
			int found = 0;
	        while (count<1024){
				char *name = malloc(dir->name_len);
				strncpy(name,dir->name,dir->name_len);
				
				if ((dir->file_type == 1) && (strcmp(name,filename)==0)){
					//get the file inode in order to copy
					file_inode = dir->inode;
					found = 1;
				 }

				free(name);
				count += dir->rec_len;
				dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + count);			
			}
			if(found == 0){
				return ENOENT;
			}
		}else{
			return ENOENT;
		}
		
         dir = (struct ext2_dir_entry_2 *)(disk + (&inodetable[1])->i_block[0] * 1024); 
         inode = 2;
		 //search url2
		 int remain = 0;
		 int temp = 0;
		
		 if(search_r1(url2)!=0){
			
		 	int count = 0;
		 	while (count<1024){
		 		if(temp <= dir->rec_len && (count + dir->rec_len < 1024)){
		 			temp = dir->rec_len;
		 		}
		 		if( strcmp(dir->name,filename)==0){
					return EEXIST;
				}
		 		
		 		count += dir->rec_len;
		 		remain = dir->rec_len;
		 		dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + count);			
		 	}	
		 	
		 	dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + (1024 - remain) );
		 	int name_len = 0;
		 	if((dir->name_len + 8)%4 != 0){
		 		name_len = ((dir->name_len + 8)/4 + 1)*4 ;
		 	}else{
		 		name_len = ((dir->name_len + 8)/4)*4;
	     	}
		 	
		 	dir->rec_len = name_len;
		 	remain = remain - name_len;
		 	//get a free inode and copy all the data of the file in url1 to a new file
		 	int new_inode = free_inode();
			
		 	dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + (1024 - remain));
		 	
		 	dir->rec_len = remain;
		 	dir->file_type = 1;
		 	
		 	dir->inode = new_inode;
		 	if(strlen(filename)%4 != 0){
				name_len = (strlen(filename)/4 + 1)*4;
			}else{ 
		 		name_len = (strlen(filename)/4)*4;
		 	}
		 	
		 	dir->name_len = name_len;
		 	strcpy(dir->name,filename);
		 	
		 	
		 	
		 	//create a buffer to save all the data
		 	int file_size = (&inodetable[file_inode-1])->i_size;
		 	block_numbers = file_size /1024;
			if ((file_size % 1024) != 0){
				block_numbers += 1;
			}
			int *block_array_copy = malloc(sizeof(int)*(block_numbers-12));
			char *buffer = malloc(file_size);
			if ((file_size / 1024 <= 12 && (file_size % 1024 == 0)) || (file_size / 1024 <12)){
				int block_index = 0;
			
				if (file_size / 1024 <= 12 && (file_size % 1024 == 0)){
					block_numbers = file_size / 1024;
				}else{
					block_numbers = file_size / 1024 + 1;
				}	
				for(; block_index < block_numbers; block_index++){
					memcpy(buffer + block_index * 1024,(&inodetable[file_inode-1])->i_block[block_index]*1024 + disk,1024);
				}
			}else{
				block_numbers = file_size /1024;
				if ((file_size % 1024) != 0){
					block_numbers += 1;
				}
				int block_index = 0;
				
				for(; block_index < 12; block_index++){
					memcpy(buffer + block_index * 1024,(&inodetable[file_inode-1])->i_block[block_index]*1024 + disk,1024);
				}
				int* block_array = (int*)(disk + 1024 * (&inodetable[file_inode-1])->i_block[12]);
				int i;
				for  (i = 12; i < block_numbers;i++){
					memcpy(buffer + i * 1024,block_array[i-12]*1024 + disk,1024);
				}
			}
			//set the data for that inode.
		    (&inodetable[new_inode-1])->i_size = file_size;
		    (&inodetable[new_inode-1])->i_mode = 0x8fff;
		    (&inodetable[new_inode-1])->i_blocks = (&inodetable[file_inode-1])->i_blocks;
		    (&inodetable[new_inode-1])->i_links_count = 1;
		    
			
	
			//copy the data in buffer to the new inode's i_block, include the indirectly blocks.
             if (block_numbers <=12) {
                 int new_block_index = 0;
                 int buffer_count = 0;
                 int new_block = 0;
                 for(;new_block_index < block_numbers; new_block_index++){
					 new_block = free_block();
					 (&inodetable[new_inode-1])->i_block[new_block_index] = new_block;
					 
                     if (file_size - buffer_count >= 1024) {
						 
                         memcpy(disk + new_block*1024,buffer,1024);
                         
                     }else{
                         memcpy(disk + new_block*1024,buffer,file_size - buffer_count);
                     }
                    
                     buffer_count += 1024;
                     if (file_size - buffer_count >= 1024) {
                         buffer += 1024;
                     }
                 }
             }else{
                 int new_block_index = 0;
                 int buffer_count = 12*1024;
                 int new_block = 0;
                 for(;new_block_index < 12; new_block_index++){
					 new_block = free_block();
					 (&inodetable[new_inode-1])->i_block[new_block_index] = new_block;
                     memcpy(disk + new_block*1024,buffer,1024);
                
                     buffer += 1024;
                 }
                 new_block = free_block();
                 (&inodetable[new_inode-1])->i_block[12] = new_block;
                 int new_block_index_l12 = 12;
                 for  (; new_block_index_l12 < block_numbers;new_block_index_l12++){
					 new_block = free_block();
					 block_array_copy[new_block_index_l12-12] = new_block;
                     if (file_size - buffer_count >= 1024) {
                         memcpy(disk + new_block*1024,buffer,1024);
                       
                     }else{
                         memcpy(disk + new_block*1024,buffer,file_size - buffer_count);
                         
                     }
                     buffer_count += 1024;
                     if (file_size - buffer_count >= 1024) {
                         buffer += 1024;
                     }

                 }
                memcpy(disk+(&inodetable[new_inode-1])->i_block[12]*1024,block_array_copy,sizeof(block_array_copy));
             }
           
		 }else{
		 	return ENOENT;
		 }
		  
		free(url1);
		free(url2);
		
    return 0;
}





