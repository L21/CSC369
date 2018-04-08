#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;
int *array;

int idx  = 0;
/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	//check if all the number in array is 1 or not
	//if all 1, change to 0, and reset the global pointer
	int index = 0;
	int count = 0;
	for(; index< memsize; index++){
		if(array[index] == 1){
			count += 1;
		}
	}
	if (count == memsize){
		int i = 0;
	    for(; i< memsize; i++){
			if(array[index] == 1){
				array[index] =0;
			}
		}
		idx = 0;
	}
	
	while (array[idx] != 0 && idx < (memsize -1)){
		array[idx] = 0;
		idx += 1;
	}
	//if the last bit is 1, change it to 0 and reset the global pointer 
	if (idx == (memsize -1) &&  (array[idx] == 1) ){
		array[idx] =0;
		idx = 0;
		while (array[idx] != 0 && idx < (memsize -1)){
			array[idx] = 0;
			idx += 1;
		}
	}
	
	//if the last bit is 0, return last frame number
	 if (idx == (memsize -1) && (array[idx] == 0) ){
		
		return memsize - 1 ;
	}

	return idx;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	int p_frame = p->frame >> PAGE_SHIFT;
	array[p_frame] = 1;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	array = malloc(memsize*sizeof(int));
	int i = 0;
	for(; i< memsize; i++){
		array[i] = -1;
	}

}
