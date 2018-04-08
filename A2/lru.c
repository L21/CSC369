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

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	//get the max p_frame, which means the oldest
	int max = 0;
	int i = 0;
	for(; i< memsize; i++){
		if (array[i]> max){
			max = array[i];	
		}
	}
	int index = 0;
	for(; index< memsize; index++){
		if (array[index]== max)break;	
	}
	return index; 

}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	//get the frame number
	int p_frame = p->frame >> PAGE_SHIFT;
	int idx = 0;
	//add 1 to all the number in array except -1
	for (; idx< memsize; idx++){
		if(array[idx]!=-1){
			array[idx] += 1;
		}
	}
	//mark the p_frame as youngest.
	array[p_frame] = 1;
	
	
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	array = malloc(memsize*sizeof(int));
	int i = 0;
	for(; i< memsize; i++){
		array[i] = -1;
	}

}
