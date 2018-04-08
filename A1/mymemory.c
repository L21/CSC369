

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
/* mymalloc_init: initialize any data structures that your malloc needs in
                  order to keep track of allocated and free blocks of 
                  memory.  Get an initial chunk of memory for the heap from
                  the OS using sbrk() and mark it as free so that it can  be 
                  used in future calls to mymalloc()
*/

void *current;
void *current_address;
void *end;
void *header_address;
void *free_address;
void *node_address;
#define page 4096






struct node{
	int size;
	struct node *next;
};
struct node *original_node = NULL;
struct node *new_node = NULL;
struct node *temp = NULL;
struct node *free_node = NULL;
struct node *reuse_node = NULL;
struct node *node_ptr = NULL;
struct node *temp_node = NULL;

pthread_mutex_t locked = PTHREAD_MUTEX_INITIALIZER;




int mymalloc_init() {
	pthread_mutex_lock(&locked);
	current = sbrk(0);
	sbrk(page);
	end = sbrk(0);
	pthread_mutex_unlock(&locked);
	
	return 0; // non-zero return value indicates an error
}



/* mymalloc: allocates memory on the heap of the requested size. The block
             of memory returned should always be padded so that it begins
             and ends on a word boundary.
     unsigned int size: the number of bytes to allocate.
     retval: a pointer to the block of memory allocated or NULL if the    
             memory could not be allocated. 
             (NOTE: the system also sets errno, but we are not the system, 
                    so you are not required to do so.)
*/
void *mymalloc(unsigned int size) {
   pthread_mutex_lock(&locked);
   //reload the free list
   free_node = free_address;
  
   
    
    //let the size become multiple of 8
	if(size % 8 != 0){
		size = size + 8 - size % 8;
	}
	//check whether the remain space can hold the size or not, if it can be saved in that space,can we dont need sbrk again.
	if((end-current)> (size + sizeof(struct node))){
		
		new_node =  current;
		new_node->size = size;
		
		//inset the new malloc address, and link them together
		if (original_node == NULL){
			original_node = new_node;
			current = current + size + sizeof(struct node);
			header_address = original_node;
					
		}else{
			
			original_node->next = new_node;
			original_node = original_node->next;
			current = current + size + sizeof(struct node);
			
		}	
	 }else{
		 //check whether the node in free list can meet the need of size
		 
		   if (free_node != NULL){
		        while(free_node->next!=NULL){
					
					if (free_node->size >= (size+8) ){
						break;
					}
					temp = free_node;
				    free_node = free_node->next;
				}
				
				//resuse the free node to malloc
				if (free_node->next == NULL && free_node->size >= size + 8){
					
					new_node = free_node;
					if (original_node == NULL){
						original_node = new_node;
						original_node->next =NULL;
						header_address = original_node;
					}else{
						
						original_node->next = new_node;
						original_node = original_node->next;
						
						original_node->next = NULL;
					}
					if (temp != NULL){
					    temp->next = NULL;
					    free_node = temp;
					}else{
						free_address = NULL;
					}
					temp =NULL;
					
				    pthread_mutex_unlock(&locked);
				    return new_node;
				    
				    
				 //resuse the free node to malloc   
				}else if (free_node->next != NULL){
					
					 
						new_node = free_node;
						
						
						temp_node = free_node->next;
						if (original_node == NULL){
							original_node = new_node;
							original_node->next =NULL;
							header_address = original_node;
						}else{
							original_node->next = new_node;
							original_node = original_node->next;
							original_node->next = NULL;
						}
						if (temp != NULL){
						    temp->next = temp_node;
						    
						    free_node = temp->next;
						}else{
							if (free_node->next == NULL){
							    free_address = NULL;
						    }else{
								free_address = free_node->next;
							}
						}
						temp =NULL;
						
						pthread_mutex_unlock(&locked);
					    return new_node;
					    
					    
					    
				}else{
					temp = NULL;
					//resuse the free node to malloc
					
					   unsigned int rest = end-current;
						size = size -rest + sizeof(struct node);
						int number;
						    number = size / page;
						    number += 1;
						 while (number > 0) {
							
							sbrk(page);
							number -=1;
						}
						
						new_node = current ;
						new_node->size = size - sizeof(struct node) + rest;
						original_node->next = new_node;
						original_node = original_node->next;
						
						current = current + size + rest;
						end = end + (size/page + 1)*page;
						size = size + rest - sizeof(struct node);
						free_node = free_address;
						pthread_mutex_unlock(&locked);
	                    return new_node;
	
				}
				
			 //if there is no free list, and the remaining size is smaller than the size we need, we just need call sbrk() again    
	        }else if(free_node == NULL){
				
		        unsigned int rest = end-current;
		       
				size = size -rest + sizeof(struct node);
				int number;
				    number = size / page;
				    number += 1;
				 while (number > 0) {
					
					sbrk(page);
					number -=1;
				}
				//creat the node and link them 
				new_node = current ;
				
				new_node->size = size - sizeof(struct node) + rest;
				original_node->next = new_node;
				original_node = original_node->next;
				
				current = current + size + rest;
				end = end + (size/page + 1)*page;
				size = size + rest - sizeof(struct node);
				pthread_mutex_unlock(&locked);
				 
	            return original_node; 
 
		    }
		    
		}
    
    pthread_mutex_unlock(&locked);
	return original_node; 
	  	
}


/* myfree: unallocates memory that has been allocated with mymalloc.
     void *ptr: pointer to the first byte of a block of memory allocated by 
                mymalloc.
     retval: 0 if the memory was successfully freed and 1 otherwise.
             (NOTE: the system version of free returns no error.)
*/
unsigned int myfree(void *ptr) {
	
	pthread_mutex_lock(&locked);
	current_address = original_node;
	printf("%p\n", original_node);
	//reload the original list and free list, loop them to find the address that we want free
	original_node = header_address;
	free_node = free_address;
	if(free_node !=NULL){
		while(free_node->next!=NULL){
			
			free_node = free_node->next;
		}
    }
     
    if (original_node != NULL){
		
		if (original_node != ptr){
			
			while(original_node->next != ptr){
				printf("%p\n", original_node);
				if(original_node->next != NULL){
				    original_node = original_node->next;
			    }else{
					
					pthread_mutex_unlock(&locked);
					return 1;
				}
		    }
		    //remove the address we must free, and add it into the free list.
		    if (free_node == NULL){
				free_node = original_node->next;
				if (original_node->next != NULL){
					reuse_node = original_node->next->next;
				}else{
					reuse_node = NULL;
				}
				original_node->next = NULL;
				original_node ->next = reuse_node;
				if (free_node != NULL){
				free_node->next = NULL;
			    }
			    free_address = free_node;

			}else{
				
				free_node->next = original_node->next;
				if (original_node->next != NULL && original_node->next->next != NULL){
					original_node ->next = original_node->next->next;
					
				}else{
					original_node ->next = NULL;
				}
				free_node = free_node->next;
				if (free_node != NULL){
					
					free_node->next = NULL;
				}
				
			}
			
		}else{
			
			if (free_node == NULL){
				free_node = original_node;
				original_node = original_node->next;
				free_node->next = NULL;
			    free_address = free_node;
			    header_address = original_node;
			    
			}else{
				
				free_node->next = original_node;
				original_node = original_node->next;
				free_node = free_node->next;
				free_node->next = NULL;
				header_address = original_node;
			}
		}
	}
	//let the pointer point to the last node, so that we can do malloc again
    /*if (original_node != NULL){
		while(original_node->next != current_address){
			
			original_node = original_node->next;
		}
    }
    original_node = original_node->next;*/
    //original_node->next = NULL;
    original_node = current_address;
	pthread_mutex_unlock(&locked);
	return 0;
	
}

