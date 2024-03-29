#include <stdio.h>
#include "kallocator.h"

int main(int argc, char* argv[]) {
    initialize_allocator(100, FIRST_FIT);
     //initialize_allocator(100, BEST_FIT);
     //initialize_allocator(100, WORST_FIT);
    printf("Using first fit algorithm on memory size 100\n");

    int* p[50] = {NULL};
    for(int i=0; i<10; ++i) {
        p[i] = kalloc(sizeof(int));
		 //p[i] = kalloc(i);
        if(p[i] == NULL) {
            printf("Allocation failed\n");
            continue;
        }
        *(p[i]) = i;
        printf("p[%d] = %p ; *p[%d] = %d\n", i, p[i], i, *(p[i]));
    }
	
    print_statistics();

      for(int i=0; i<10; ++i) {
        if(i%2 == 0)
            continue;

        printf("Freeing p[%d]\n", i);
        kfree(p[i]);
        p[i] = NULL;
    }  

    printf("available_memory %d\n", available_memory());

	void* before[100] = {NULL};
    	void* after[100] = {NULL};
    	compact_allocation(before, after);
	
	print_statistics(); 
	
    /*
	 for(int i=4; i>0; --i) {

        printf("Freeing p[%d]\n", i);
        kfree(p[i]);
        p[i] = NULL;
    } */
	
	/*
	int i = 12;
        printf("Freeing p[%d]\n", i);
        kfree(p[i]);
        p[i] = NULL; */
	/*
		i = 0;
        printf("Freeing p[%d]\n", i);
        kfree(p[i]);
        p[i] = NULL; */
    
    	

	//printf("\n NEW TESTING BEFORE COMPACTION...\n");
	//int i=10;	
	//p[i] = kalloc(59);
	//*(p[i]) = i;
        //printf("p[%d] = %p ; *p[%d] = %d\n", i, p[i], i, *(p[i]));
	

    // You can assume that the destroy_allocator will always be the 
    // last funciton call of main function to avoid memory leak 
    // before exit

    destroy_allocator();

    return 0;
}

