#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;
int *frame; 
int *frame_copy;
/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int fifo_evict() {
	int result = frame[0];
	frame_copy = frame;
    frame[0] = -1;
	int index = 0;
	for (; index < memsize-1; index++){
		frame_copy[index] = frame[index+1];
	}
	frame_copy[memsize-1] = -1;
	frame = frame_copy;
	return result;
	
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {

	if (frame == NULL){
		fifo_init();
	}
	int p_frame = p->frame >> PAGE_SHIFT;
	int idx = 0;
	for (; idx < memsize; idx++){
		if(frame[idx] == p_frame)break; 
		if(frame[idx] == -1){
			frame[idx] = p_frame;
			break;	
		}
	}
   
	return;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	frame = malloc(memsize * sizeof(int));
	int i = 0;
	for (; i < memsize; i++){
		frame[i] = -1; 
	}
	frame_copy = malloc(memsize * sizeof(int));
}
