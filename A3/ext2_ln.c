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
int file_inode;
char *filename;
char *filename1;
unsigned char *disk;
struct ext2_super_block *sb;
struct ext2_group_desc *b;
struct ext2_dir_entry_2 *dir;
struct ext2_inode * inodetable;



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
// form stackoverflow
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
	filename1 = paths[length-1];
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
		dir = (struct ext2_dir_entry_2 *)(disk + (&inodetable[1])->i_block[0] * 1024); 
		
		//serach url1
		if(search_r(url1)!=0){
			int count = 0;
			int found = 0;
	        while (count<1024){
				char *name = malloc(dir->name_len);
				strncpy(name,dir->name,dir->name_len);
				if ((dir->file_type == 1) && (strcmp(name,filename)==0)){
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
				char *name = malloc(dir->name_len);
				strncpy(name,dir->name,dir->name_len);
				if ((dir->file_type == 1) && strcmp(name,filename1)==0){
					return EEXIST;
				}else if((dir->file_type == 2) && strcmp(name,filename1)==0){
					return EISDIR;
				}
				free(name);
				if(temp <= dir->rec_len && (count + dir->rec_len < 1024)){
					temp = dir->rec_len;
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
			//create the new file and link it.
			dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + (1024 - remain));
			dir->rec_len = remain;
			dir->file_type = 1;
			dir->inode = file_inode;
			
			dir->name_len = strlen(filename1);
			strcpy(dir->name,filename1);
			(&inodetable[file_inode-1])->i_links_count +=1;
		}else{
			return ENOENT;
		} 
		free(url1);
		free(url2);
    return 0;
}





