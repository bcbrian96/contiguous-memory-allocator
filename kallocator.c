#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kallocator.h"
#include "list_sol.h"

struct KAllocator {
    enum allocation_algorithm aalgorithm;
    int size;
    void* memory;
    // Some other data members you want, 
    // such as lists to record allocated/free memory
	
	struct nodeStruct* allocated_blocks;
	struct nodeStruct* free_blocks;
};

struct KAllocator kallocator;


void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm) {
    assert(_size > 0);
    kallocator.aalgorithm = _aalgorithm;
    kallocator.size = _size;
    kallocator.memory = malloc((size_t)kallocator.size); //contiguous memory chunk

    // Add some other initialization 
	
	kallocator.allocated_blocks = NULL;
	kallocator.free_blocks = List_createNode(kallocator.memory, _size);
}

void destroy_allocator() {
    free(kallocator.memory);

    // free other dynamic allocated memory to avoid memory leak
	while(kallocator.allocated_blocks != NULL){		
		List_deleteNode(&kallocator.allocated_blocks, kallocator.allocated_blocks);
	}
	
	while(kallocator.free_blocks != NULL){
		List_deleteNode(&kallocator.free_blocks, kallocator.free_blocks);
	}
}

void allocate_block(struct nodeStruct* node, int size){
	
	struct nodeStruct* tmp  = List_createNode(node->ptr_block, size);
	
	node->ptr_block = node->ptr_block + size; 
	node->size = node->size - size; 
	
	if(node->size <= 0){
		List_deleteNode(&kallocator.free_blocks, node);
	}
	
	List_insertHead(&kallocator.allocated_blocks, tmp); //update

}


void* kalloc(int _size) {
    void* ptr = NULL;

    // Allocate memory from kallocator.memory 
    // ptr = address of allocated memory
	
	if(kallocator.aalgorithm == FIRST_FIT){
		struct nodeStruct* cur = kallocator.free_blocks; //reference to head
		
		while(cur != NULL){
			if(cur->size >= _size){
				break;
			}
			cur = cur->next;
		}
		
		if(cur != NULL){
			ptr = cur->ptr_block;
			allocate_block(cur, _size);
		}
	}
	
	/* else if(kallocator.aalgorithm == BEST_FIT){ //Keeping the free list sorted can speed up the process of finding the right hole.
		
		struct nodeStruct* cur = kallocator.free_blocks;
		int remainder = 0;
		int minRemainder = cur->size;
		struct nodeStruct* tmp = NULL;
		
		while(cur != NULL){
			if(cur->size >= _size){
				remainder = cur->size - _size;
				
				if(remainder < minRemainder) {
					minRemainder = remainder;
					tmp = cur;
				}
			}
			
			cur = cur->next;
		}
		
		if(tmp != NULL){
			allocate_block(_size, tmp);
			ptr = tmp->ptr_block;
		}
		else{
			ptr = NULL;
		}
		
	}
	
	else if(kallocator.aalgorithm == WORST_FIT){ //largest remainder
		
		struct nodeStruct* cur = kallocator.free_blocks;
		int remainder = 0;
		int maxRemainder = -1;
		struct nodeStruct* tmp = NULL;
		
		while(cur != NULL){
			if(cur->size >= _size){
				remainder = cur->size - _size;
				
				if(remainder > maxRemainder){
					maxRemainder = remainder;
					tmp = cur;
				}
			}
			
			cur = cur->next;
		}
		
		if(tmp != NULL){
			allocate_block(_size, tmp); 
			ptr = tmp->ptr_block;
		}
		else{
			ptr = NULL;
		}
	} */
	
	

    return ptr;
}


void kfree(void* _ptr) {
    assert(_ptr != NULL);

	struct nodeStruct* newNode = NULL;
	struct nodeStruct* replaceNode = NULL;
	
    printf("Attempting to find pointer %p: \n", _ptr);
	struct nodeStruct* targetNode = List_findNode(kallocator.allocated_blocks, _ptr);
	
	if(targetNode != NULL) {
		printf("FOUND pointer %p and size %d\n", _ptr, targetNode->size);
		newNode = List_createNode(_ptr, targetNode->size);
		List_insertTail(&kallocator.free_blocks, newNode);
		
		List_deleteNode(&kallocator.allocated_blocks, targetNode);
	}
	else{
		printf("I cannot find\n");
	}
	
	//fix contiguous free_blocks (&kallocator.free_blocks, newNode);
	struct nodeStruct* cur = kallocator.free_blocks;
	while(cur != NULL){
		if( (cur->ptr_block + cur->size) == newNode->ptr_block ){
			replaceNode = List_createNode(cur->ptr_block, cur->size + newNode->size);
			printf("NEW SIZE: %d\n", replaceNode->size);
			printf("NEW POINTER: %p\n", replaceNode->ptr_block);
			
			List_deleteNode(&kallocator.free_blocks, cur);
			List_deleteNode(&kallocator.free_blocks, newNode);
			List_insertTail(&kallocator.free_blocks, replaceNode);
			
		}
		//backwards compatibility???
		
		cur = cur->next;
	}
	
	
	
}

int compact_allocation(void** _before, void** _after) {
    int compacted_size = 0;

    // compact allocated memory
    // update _before, _after and compacted_size
	List_sort(&kallocator.allocated_blocks);
	
	struct nodeStruct* cur = kallocator.allocated_blocks;
	void *address = kallocator.memory;
	int size_cpy = 0;
	void *ptr_cpy = NULL;
	int i = 0;
	int total_size = 0;
	
	while(cur != NULL){
		
		size_cpy = cur->size;
		ptr_cpy = cur->ptr_block;
		
		_before[i] = ptr_cpy;
		_after[i] = memcpy(address, ptr_cpy, (size_t)size_cpy);
		
		cur->ptr_block = _after[i];
		
		address = address + size_cpy;
		cur = cur->next;
		i++;
		compacted_size++;
	}
	
	while(kallocator.free_blocks != NULL){	
		total_size = total_size + kallocator.free_blocks->size;
		List_deleteNode(&kallocator.free_blocks, kallocator.free_blocks);
	}
	
	struct nodeStruct* newNode = List_createNode(address, total_size);
	List_insertHead(&kallocator.free_blocks, newNode);
	
    return compacted_size;
}

/* int compact_allocation(void** _before, void** _after) {
	
	int compacted_size = 0;
	
	struct nodeStruct* cur = kallocator.allocated_blocks;
	struct nodeStruct* cur_free = kallocator.free_blocks;
	int i = 0;
	
	for(cur_free; cur_free != NULL; cur_free = cur_free->next){
		for(cur; cur != NULL; cur = cur->next){
			if(cur->ptr_block > cur_free->ptr_block){
				
				_before[i] = cur->ptr_block;
				memmove(cur_free->ptr_block, cur->ptr_block, cur->size);
				cur_free->ptr_block = cur_free->ptr_block + cur->size;
				cur->size = cur->size - cur_free->size;
			}
	}
	
} */

int available_memory() {
    int available_memory_size = 0;
    // Calculate available memory size
	struct nodeStruct* cur = kallocator.free_blocks;
	while(cur != NULL){
		available_memory_size = available_memory_size + cur->size;
		cur = cur->next;
	}
	
    return available_memory_size;
}

void print_statistics() {
    int allocated_size = 0;
    int allocated_chunks = 0;
    int free_size = 0;
    int free_chunks = 0;
    int smallest_free_chunk_size = kallocator.size;
    int largest_free_chunk_size = 0;

    // Calculate the statistics
	struct nodeStruct* cur = kallocator.allocated_blocks;
	while(cur != NULL){
		allocated_size = allocated_size + cur->size;
		++allocated_chunks;
		cur = cur->next;
	}
	
	free_size = available_memory();
	struct nodeStruct* cur_free = kallocator.free_blocks;
	smallest_free_chunk_size = cur_free->size;
	largest_free_chunk_size = cur_free->size;
	
	while(cur_free != NULL){
		++free_chunks;
		
		if(cur_free->size < smallest_free_chunk_size){
			smallest_free_chunk_size = cur_free->size;
		}
		
		if(cur_free->size > largest_free_chunk_size){
			largest_free_chunk_size = cur_free->size;
		}
		
		cur_free = cur_free->next;
	}
	

    printf("Allocated size = %d\n", allocated_size);
    printf("Allocated chunks = %d\n", allocated_chunks);
    printf("Free size = %d\n", free_size);
    printf("Free chunks = %d\n", free_chunks);
    printf("Largest free chunk size = %d\n", largest_free_chunk_size);
    printf("Smallest free chunk size = %d\n", smallest_free_chunk_size);
	
}

void print_memory(){
	printf("\n PRINTING allocated blocks LL:\n");
	List_print(&kallocator.allocated_blocks);
	printf("\n PRINTING free blocks LL:\n");
	List_print(&kallocator.free_blocks);
}



