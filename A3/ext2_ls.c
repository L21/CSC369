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

int inode = 2;
unsigned char *disk;
struct ext2_group_desc *b;
struct ext2_dir_entry_2 *dir;
struct ext2_inode * inodetable;


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

//deal with the input url in order for search
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

//search the input url, to check whether it exists or not
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
//search the input url, to check whether it exists or not
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
		
		//initialized  inodetable and dir entry
		b = (struct ext2_group_desc *)(disk + 1024 + 1024);
		inodetable =(struct ext2_inode  *) (b->bg_inode_table * EXT2_BLOCK_SIZE +disk);
		dir = (struct ext2_dir_entry_2 *)(disk + (&inodetable[1])->i_block[0] * 1024);
		
		//Search url
		if(search_r(url)!=0){		
			int count = 0;
			while (count<1024){
				int len = 0;
				//print the name
				for(; len < dir->name_len; len++){
					printf("%c", dir->name[len]);
				}
				printf("\n");
				count += dir->rec_len;
				dir = (struct ext2_dir_entry_2 *)(disk + ((&inodetable[inode-1])->i_block[0] * 1024) + count);
			}
				
		}else{
			perror("No such file or diretory");
			exit(1);
		} 
    return 0;
}
