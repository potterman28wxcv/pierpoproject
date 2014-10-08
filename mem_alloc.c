#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

// #define __CHECK_END__
// #define FIRST_FIT
// #define BEST_FIT
// #define WORST_FIT
#define LEAK_TEST

/* memory */
char memory[MEMORY_SIZE]; 

/* Structure declaration for a free block */
typedef struct free_block{
	int size; 
	struct free_block *next; 
} free_block_s, *free_block_t; 

/* Structure declaration for an occupied block */
typedef struct{
	int size; 
} busy_block_s, *busy_block_t; 


/* Pointer to the first free block in the memory */
free_block_t first_free; 


#define ULONG(x)((long unsigned int)(x))

#ifdef __CHECK_END__
static void check_for_unfreed(void)
{
	free_block_t cur = first_free;
	int free_memory = 0;

	if (cur->size != MEMORY_SIZE){
		fprintf(stderr, "WARNING : some blocks are not freed !\n");
		while (cur != NULL){
			free_memory += cur->size;
			cur = cur->next;
		}
		fprintf(stderr, "%lu bytes haven't been freed\n", \
				(long unsigned int)(MEMORY_SIZE - free_memory));
	}

}
#endif

void memory_init(void){
	free_block_s free_block = {MEMORY_SIZE, NULL};

	memcpy(memory, &free_block, sizeof(free_block_s));
   
   	first_free = (free_block_t) memory;

#ifdef __CHECK_END__
	atexit(&check_for_unfreed);
#endif
}




/********************* MEMORY ALLOC **************************/

char *memory_alloc(int size){

#ifdef BEST_FIT
	int min_remaining_size = -1;
	free_block_t best_block = NULL;
	free_block_t best_previous = NULL;
#elif defined(WORST_FIT)
	int max_remaining_size = -1;
	free_block_t best_block = NULL;
	free_block_t best_previous = NULL;
#endif

	/* Common part */
	free_block_t current_free;
	free_block_t previous;
	free_block_t new_free;
	free_block_s new_free_s;

	int allocated_memory_size;

	current_free = first_free;
	previous = first_free;

	if (first_free == NULL) {
		printf("first_free does not exist.\n");
		exit(EXIT_FAILURE);
	}

	/***************/

#if !defined(BEST_FIT) && !defined(WORST_FIT)

	/* ***** FIRST FIT ***** */

	/* Browse through the free_block list */
	while (current_free != NULL && current_free->size < size + sizeof(busy_block_s)) {
		previous = current_free;
		current_free = current_free->next;
	}

	/* If we went through the whole list, then the memory is full */
	if (current_free == NULL) {
		printf("Not enough memory space.\n");
		exit(EXIT_FAILURE);
	}

	/***************/

#elif defined(BEST_FIT)

	/* ***** BEST FIT ***** */

	/* Browse through the free_block list */
	while (current_free != NULL) {
		previous = current_free;
		if (current_free->size > size + sizeof(busy_block_s)) {
			if (min_remaining_size == -1 || (current_free->size - size - sizeof(busy_block_s) < min_remaining_size)) {
				best_block = current_free;
				min_remaining_size = current_free->size - size - sizeof(busy_block_s) ;
				best_previous = previous;
			}
		}
		current_free = current_free->next;
	}

	/* If we went through the whole list and didn't find a match, then the memory is full */
	if (min_remaining_size == -1) {
		printf("Not enough memory space.\n");
		exit(EXIT_FAILURE);
	}

	current_free = best_block;
	previous = best_previous;

	/* ******************* */

#elif defined(WORST_FIT)

	/* ***** WORST FIT ***** */

	/* Browse through the free_block list */
	while (current_free != NULL) {
		previous = current_free;
		if (current_free->size > size + sizeof(busy_block_s)) {
			if (max_remaining_size == -1 || (current_free->size - size - sizeof(busy_block_s) > max_remaining_size)) {
				best_block = current_free;
				max_remaining_size = current_free->size - size - sizeof(busy_block_s) ;
				best_previous = previous;
			}
		}
		current_free = current_free->next;
	}

	/* If we went through the whole list and didn't find a match, then the memory is full */
	if (max_remaining_size == -1) {
		printf("Not enough memory space.\n");
		exit(EXIT_FAILURE);
	}

	current_free = best_block;
	previous = best_previous;

	/* ******************* */

#endif

	/* We now allocate the block */
	/* If it is too small to be a free_block in the future,
	 * we force it to be at least of this size */
	if (size + sizeof(busy_block_s) < sizeof(free_block_s))
		allocated_memory_size = sizeof(free_block_s);
	else
		allocated_memory_size = size + sizeof(busy_block_s);

	/* New pointer to the beginning of the new free block */
	new_free = (free_block_s*) (((char*)current_free) + allocated_memory_size);
	/* Write the new size left in the structure */
	new_free_s.size = current_free->size - allocated_memory_size;
	/* Write the new next position in the structure */
	new_free_s.next = current_free->next;
	memcpy(new_free, &new_free_s, sizeof(free_block_s));

	/* Now we have to replace the old free block by a busy one */
	current_free->size = allocated_memory_size;

	/* previous -> new_free */
	if (previous == first_free) {
		first_free = new_free;
	} else {
		previous->next = new_free;
	}

 	print_alloc_info((char*) current_free + sizeof(busy_block_s), current_free->size - sizeof(busy_block_s)); 

	return ((char*) current_free) + sizeof(busy_block_s);
}



/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/




/******************************* MEMORY FREE ************************************/

/**
 * Merge the block to the left free neighbour, 
 * or the right free neighbour, or both
 * Else, if there's no neighbour, it creates a new free block
 *
 * Assumes that the free blocks are linked by address
 */
void memory_free(char *p){
	char ok = 0;
	char *cursor;
        int new_first_free; /* equals 1 if the new free block is a first_free */
	free_block_s new;
	free_block_t left_neighbour = NULL;
	free_block_t cur = first_free;
	free_block_t prev = first_free;
	busy_block_t cur_busy;
	busy_block_t to_be_freed = (busy_block_t) ((char *) (p - sizeof(busy_block_s)));
	
	print_free_info(p);

	/* Determining if the to_be_freed block is busy */
	cur = first_free;
	cursor = memory;
	while (cursor < (char *) to_be_freed){
		if (cursor == (char *)cur){
			/* cursor is on a free block */
			cursor += cur->size;
			cur = cur->next;
		} else {
			/* cursor is on a busy block */
			cur_busy = (busy_block_t) cursor;
			cursor += cur_busy->size;
			if (cur_busy->size == 0)
				break;
		}
	}
	if (!(cursor == (char *) to_be_freed && cursor != (char *) cur)){
		if (cursor != (char *) to_be_freed)
			fprintf(stderr, "You're trying to free an invalid address\n");
		else if (cursor == (char *) cur)
			fprintf(stderr, "The address is already freed !\n");
		exit(EXIT_FAILURE);
	}
		

	/* Searching for a left neighbour */
	cur = first_free;
	cursor = (char *)first_free;
	if (cursor + cur->size == (char *) to_be_freed)
		ok = 1;
	while (cursor + cur->size != (char *)to_be_freed){
		ok = 0;
		if (cur->next == NULL)
			break;
		cursor = (char *)cur->next;
		cur = cur->next;
		ok = 1;
	}
	if (ok){
		cur->size += to_be_freed->size;
		left_neighbour = cur;
	}

	/* Searching for a right neighbour */
	cur = first_free;
	while (cur != NULL){
		if ((char *)cur == ((char*)to_be_freed + to_be_freed->size)){
			if (left_neighbour == NULL){
				/* No left neighbour */
				new.size = cur->size + to_be_freed->size;
				new.next = cur->next;
				memcpy(to_be_freed, &new, sizeof(free_block_s));
				if (prev == first_free)
					first_free = (free_block_t) to_be_freed;
				else
					prev->next = (free_block_t) to_be_freed;
				return;
			} else {
				/* There's a left neighbour, we merge both */
				left_neighbour->size += cur->size;
				left_neighbour->next = cur->next;
				return;
			}
		}
		prev = cur;
		cur = cur->next;
	}

	/**
	 * If there was a left neighbour, but no right neighbour
	 * In either case we have to terminate the function if we have found a
	 * neighbour ; this case (left neighbour, no right neighbour) is the
	 * only case not tackled yet 
	 */
	if (left_neighbour != NULL)
		return;

	/* No neighbour could be found */
	cur = first_free;

	/**
	 * If there isn't enough space to put a free block, we merge it to the
	 * left neighbour (which is a busy block, because no free neighbour
	 * were found !) 
	 *
	 * For this algorithm, we consider that everytime the cursor is on
	 * a free block. As the free block and the busy block share the size
	 * attribute in their structure (stored at the same relative location
	 * in the memory), it works in this particular case.
	 *
	 * This would have to be changed if we decided to change one of the
	 * structures 
	 */
	if (to_be_freed->size < sizeof(free_block_s)){
		cursor = memory;
		cur = (free_block_t) cursor;
		while (cursor + cur->size != (char *) to_be_freed){
			cursor += cur->size;
			cur = (free_block_t) cursor;
		}

		/* cursor and cur are placed on the left neighbour (busy) */
		cur->size += to_be_freed->size;
		return;
	}

        /* If the block to be freed is before first_free, it will become
         * first_free */
        new_first_free = ((char *)cur > p);

	/* Determining where we must insert the new free block 
         * in the linked list */
        if (new_first_free == 0)
                while ((char *)(cur->next) < p && cur->next != NULL)
                        cur = cur->next;

	/* Then, writing and inserting the new free block */
	new.size = to_be_freed->size;

        if (new_first_free == 1)
                new.next = cur;
        else
                new.next = cur->next;

	memcpy((char *)to_be_freed, &new, sizeof(free_block_s));

	/* Tackling the case when the new free becomes a first free */
        if (new_first_free == 1)
                first_free = (free_block_t) to_be_freed;
        else
                cur->next = (free_block_t) to_be_freed;
}

void print_info(void) {
	fprintf(stderr, "Memory : [%lu %lu] (%lu bytes)\n", (long unsigned int) 0, (long unsigned int) (memory+MEMORY_SIZE), (long unsigned int) (MEMORY_SIZE));
	fprintf(stderr, "Free block : %lu bytes; busy block : %lu bytes.\n", ULONG(sizeof(free_block_s)), ULONG(sizeof(busy_block_s))); 
}

void print_free_info(char *addr){
	if(addr)
		fprintf(stderr, "FREE  at : %lu \n", ULONG(addr - memory)); 
	else
		fprintf(stderr, "FREE  at : %lu \n", ULONG(0)); 
}

void print_alloc_info(char *addr, int size){
	if(addr){
		fprintf(stderr, "ALLOC at : %lu (%d byte(s))\n", 
				ULONG(addr - memory), size);
	}
	else{
		fprintf(stderr, "Warning, system is out of memory\n"); 
	}
}

void print_free_blocks(void) {
	free_block_t current; 
	fprintf(stderr, "Begin of free block list :\n"); 
	for(current = first_free; current != NULL; current = current->next) {
		fprintf(stderr, "Free block at address %lu, size %u\n", ULONG((char*)current - memory), current->size);
	}
}

char *heap_base(void) {
	return memory;
}


void *malloc(size_t size){
	static int init_flag = 0; 
	if(!init_flag){
		init_flag = 1; 
		memory_init(); 
		//print_info(); 
	}      
	return (void*)memory_alloc((size_t)size); 
}

void free(void *p){
	if (p == NULL) return;
	memory_free((char*)p); 
	print_free_blocks();
}

void *realloc(void *ptr, size_t size){
	if(ptr == NULL)
		return memory_alloc(size); 
	busy_block_t bb = ((busy_block_t)ptr) - 1; 
	printf("Reallocating %d bytes to %d\n", bb->size - (int)sizeof(busy_block_s), (int)size); 
	if(size <= bb->size - sizeof(busy_block_s))
		return ptr; 

	char *new = memory_alloc(size); 
	memcpy(new, (void*)(bb+1), bb->size - sizeof(busy_block_s) ); 
	memory_free((char*)(bb+1)); 
	return (void*)(new); 
}


#ifdef MAIN

#ifdef LEAK_TEST
int leaking = 1;

void leaking_fun(int n) {
	void *a,*b,*c;
	if(n<0)
		return;
	a = malloc(5);
	b = malloc(10);
	leaking_fun(n-1);
	free(a);
	c = malloc(5);
	leaking_fun(n-2);
	free(c);
	if(!leaking || (n%2)==0)
		free(b);
}
#endif


int main(int argc, char **argv){

#ifdef LEAK_TEST
	if(argc>1) 
		leaking = 0;
	else 
		leaking = 1;
	leaking_fun(6);
	return 0;
#else
	memory_init();
	print_info(); 
	print_free_blocks();
	int i ; 
	for( i = 0; i < 10; i++){
		char *b = memory_alloc(rand()%8);
		memory_free(b); 
		print_free_blocks();
	}




	char * a = memory_alloc(15);
	a=realloc(a, 20); 
	memory_free(a);


	a = memory_alloc(10);
	memory_free(a);

	printf("%lu\n",(long unsigned int) (memory_alloc(9)));
	return EXIT_SUCCESS;
#endif

}
#endif 
