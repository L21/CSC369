#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;
#define MAXLINE 256
extern struct frame *coremap;
extern char *tracefile;
FILE *tfp;
int *array_memory;
int *array_file;
int size_of_file = 0;
int file_ptr = 0;

/*code from the sim.c, given from the instructor
 * open the tracefile, calculate the number of address in tracefile
 * in order to malloc the array_file to store all the address 
 */
 void find_trace(FILE *tfp) {
    char buf[MAXLINE];
    addr_t vaddr = 0;
	char type;
	while(fgets(buf, MAXLINE, tfp) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			if(debug)  {
				printf("%c %lx\n", type, vaddr);
			}
			size_of_file += 1;
		} else {
			continue;
		}

	}
}


/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	int max = 0;
	int memory_index = 0;
	//use for loop and max to get the index of the address in array_memory that appear latest in array_file
	for (;memory_index < memsize; memory_index++){
		//loop the array_file from the global pointer;
		int array_index = file_ptr;
		for (;array_index < size_of_file; array_index++){
			// if find one corresponding address, break the for loop.
			if (array_file[array_index] == array_memory[memory_index]){
				//keep update the max position 
				if (array_index >= max){
					max = array_index;
				}
				break;
			}
		}
		if ((array_index==(size_of_file)) && (array_file[array_index-1] != array_memory[memory_index])){
			return memory_index;
		}
	}
	//choose the frame number which should be evict.
	int memory_idx = 0;
	for(;memory_idx < memsize -1; memory_idx++){
		if (array_memory[memory_idx] == array_file[max]){
			break;
		}
	}
	
	
	return memory_idx;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	//get the frame number of *p
	int p_frame = p->frame >> PAGE_SHIFT;
	//set the corresponding address to array_memory
    array_memory[p_frame] = array_file[file_ptr];
    file_ptr += 1;
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	//open the tracefile
	if(tracefile != NULL) {
		if((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
		}
	}
	
	find_trace(tfp);
	array_memory = malloc(memsize*sizeof(int));
	array_file = malloc(size_of_file*sizeof(int));
	int index = 0;
	char buf[MAXLINE];
	addr_t vaddr = 0;
	char type;
	tfp = fopen(tracefile, "r");
	while(fgets(buf, MAXLINE, tfp) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			if(debug)  {
				printf("%c %lx\n", type, vaddr);
			}
			array_file[index] = vaddr;
			index += 1; 
		} else {
			continue;
		}

	}

}



