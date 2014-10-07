#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/*#define BEST_FIT */

/* #define DEBUG */
/* #define __CHECK_FREE__ */
/* #define __CHECK_END__ */
/* #define FIRST_FIT */

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
#define max(x,y) (x>y?x:y)
#define min(x,y) (x<y?x:y)

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

/* FIRST FIT */
#ifdef FIRST_FIT
char *memory_alloc(int size){

	free_block_t current_free;
	free_block_t previous;
	free_block_t new_free;
	free_block_s new_free_s;

	current_free = first_free;
	previous = first_free;

#ifdef DEBUG
	printf("\nStarting to allocate %i bytes\n", size);
#endif

	if (first_free == NULL) {
		printf("first_free does not exist.\n");
		exit(EXIT_FAILURE);
	}

	/* Browse through the free_block list */
	while (current_free->size < size + sizeof(busy_block_s) && current_free != NULL) {
		previous = current_free;
		current_free = current_free->next;
	}

	/* If we went through the whole list, then the memory is full */
	if (current_free == NULL) {
		printf("Not enough memory space.\n");
		exit(EXIT_FAILURE);
	}

	/* We now allocate the block */

	/* PROBLEME POTENTIEL : SI ON A TOUT PILE LA PLACE */
	/* New pointer to the beginning of the new free block */
	new_free = (free_block_s*) (((char*)current_free) + size + sizeof(busy_block_s));
	/* Write the new size left in the structure */
	new_free_s.size = current_free->size - size - sizeof(busy_block_s);
	/* Write the new next position in the structure */
	new_free_s.next = current_free->next;
	memcpy(new_free, &new_free_s, sizeof(free_block_s));

#ifdef DEBUG
	printf("We're allocating in a free block of size %i\n", current_free->size);
#endif
#ifdef DEBUG
	printf("The next free block should be ");
#endif
#ifdef DEBUG
	if (current_free->next != NULL) {
		printf("%x\n", (char *)current_free->next - memory);
		printf("current_free->next : %x\n",current_free->next);
	} else
		printf("NULL\n");
#endif

	/* Now we have to replace the old free block by a busy one */
	current_free->size = size + sizeof(busy_block_s);

	/* previous -> new_free */
	/* Works even if we are on first_free */
	if (previous == first_free) {
#ifdef DEBUG
		printf("First_free = new_free\n");
#endif
		first_free = new_free;
	} else {
#ifdef DEBUG
		printf("Previous->next = new_free\n");
#endif
		previous->next = new_free;
	}

 	print_alloc_info((char*) current_free + sizeof(busy_block_s), current_free->size - sizeof(busy_block_s)); 

#ifdef DEBUG
	printf("Allocating finished at %i\n", ((char*) current_free) + sizeof(busy_block_s) - memory);
	printf("Now the first_free is %i\n", (char*)first_free - memory);
#endif
	return ((char*) current_free) + sizeof(busy_block_s);
}
#endif



/***** BEST FIT *****/
#ifdef BEST_FIT
char *memory_alloc(int size){
	int min_remaining_size = -1;
	free_block_t best_block = NULL;
	free_block_t best_previous = NULL;

	free_block_t current_free;
	free_block_t previous;
	free_block_t new_free;
	free_block_s new_free_s;

	current_free = first_free;
	previous = first_free;

	if (first_free == NULL) {
		printf("first_free does not exist.\n");
		exit(EXIT_FAILURE);
	}

	/* Browse through the free_block list */
	while (current_free != NULL) {
		previous = current_free;
		if (current_free->size > size + sizeof(busy_block_s)) {
			/* Not sure about the condition */
			if (min_remaining_size == -1 || (current_free->size - size - sizeof(busy_block_s) < min_remaining_size)) {
				best_block = current_free;
				min_remaining_size = current_free->size - size - sizeof(busy_block_s) ;
				printf("New min_remaining_size : %i\n",min_remaining_size);
				best_previous = previous;
			}
		}
		current_free = current_free->next;
	}
#ifdef DEBUG
	printf("Remaining size : %i\n",min_remaining_size);
#endif
	

	/* If we went through the whole list and didn't find a match, then the memory is full */
	if (min_remaining_size == -1) {
		printf("Not enough memory space.\n");
		exit(EXIT_FAILURE);
	}

	/* We now allocate the block */

	/* New pointer to the beginning of the new free block */
	new_free = (free_block_s*) (((char*)best_block) + size + sizeof(busy_block_s));
	/* Write the new size left in the structure */
	new_free_s.size = best_block->size - size - sizeof(busy_block_s);
	/* Write the new next position in the structure */
	new_free_s.next = best_block->next;
	memcpy(new_free, &new_free_s, sizeof(free_block_s));

	/* Now we have to replace the old free block by a busy one */
	best_block->size = size + sizeof(busy_block_s);

	/* previous -> new_free */
	/* Works even if we are on first_free */
	if (best_previous == first_free) {
		first_free = new_free;
	} else {
		best_previous->next = new_free;
	}

 	print_alloc_info((char*) best_block + sizeof(busy_block_s), best_block->size - sizeof(busy_block_s)); 

	return ((char*) best_block) + sizeof(busy_block_s);
}
#endif




/************************ MEMORY FREE **********************/

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
	busy_block_t to_be_freed = (char *) (p - sizeof(busy_block_s));
	
	print_free_info(p);
#ifdef DEBUG
	printf("\nStart of memory_free\n");
	printf("Block to be freed : size %i\n", to_be_freed->size);
#endif

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
		}
	}
	if (!(cursor == (char *) to_be_freed && cursor != (char *) cur)){
		fprintf(stderr, "You're trying to free an invalid adress\n");
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
#ifdef DEBUG
	printf("End of memory_free !\n");
#endif
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
int main(int argc, char **argv){

	/* The main can be changed, it is *not* involved in tests */
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
}
#endif 
