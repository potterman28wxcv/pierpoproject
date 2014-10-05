#include "mem_alloc.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

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

/**
 * IMPORTANT
 * This macro MUST be avoided
 * because sizeof(free_block_s) > sizeof(int) + sizeof(free_block_t)
 */
#define WRITE_IN_MEMORY(type, address, value)(*((type*) address) = value)

void memory_init(void){
   	/* *((int*) memory) = MEMORY_SIZE;*/
	free_block_s free_block = {MEMORY_SIZE - sizeof(free_block_s), NULL};

	/*
	 * The following should be avoided, because
   	WRITE_IN_MEMORY(int, memory, MEMORY_SIZE);
   	WRITE_IN_MEMORY(free_block_t, memory+sizeof(int), 42);
	*/
	/*
	*((int *) memory) = MEMORY_SIZE;
	*((free_block_t *) memory+sizeof(int)) = 42;
	*/

	memcpy(memory, &free_block, sizeof(free_block_s));
   
   	first_free = (free_block_t) memory;

	/*
	printf("TEST MEMORY_INIT : %i %i\n", sizeof(int) + sizeof(free_block_t), sizeof(free_block_s));
	printf("TEST 2 : %i %i\n", first_free->size, first_free->next);
	*/
}

char *memory_alloc(int size){
	/* First Fit */

	free_block_t current_free;
	free_block_t previous;
	free_block_t new_free;
	free_block_s new_free_s;

	busy_block_t bizi_block;

	current_free = first_free;
	previous = first_free;

	printf("\nStarting to allocate %i bytes\n", size);

	if (first_free == NULL) {
		printf("first_free does not exist.\n");
		exit(EXIT_FAILURE);
	}

	/* Browse through the free_block list */
	while (current_free->size < size && current_free != NULL) {
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
	printf("ADDRESS NEW_FREE %x\n",new_free - (free_block_t) memory);
	/* Write the new size left in the structure */
   	/*WRITE_IN_MEMORY(int, new_free, current_free->size - size);*/
	/* Write the new next position in the structure */
   	/*WRITE_IN_MEMORY(free_block_t, ((char*)new_free)+sizeof(int), current_free->next);*/
	printf("We're allocating in a free block of size %i\n", current_free->size);
	new_free_s.size = current_free->size - size - sizeof(busy_block_s);
	printf("The next free block should be ");
	if (current_free->next != NULL) {
		printf("%x\n", (char *)current_free->next - memory);
		printf("current_free->next : %x\n",current_free->next);
	} else
		printf("NULL\n");
	new_free_s.next = current_free->next;
	memcpy(new_free, &new_free_s, sizeof(free_block_s));

	/* Now we have to replace the old free block by a busy one */
	current_free->size = size;

	bizi_block = (busy_block_t) current_free;
	printf("Size bizi_block : %i\n",bizi_block->size);

	/* previous -> new_free */
	/* Works even if we are on first_free */
	if (previous == first_free) {
		printf("First_free = new_free\n");
		first_free = new_free;
	} else {
		printf("Previous->next = new_free\n");
		previous->next = new_free;
	}

 	print_alloc_info((char*) current_free, current_free->size); 
/* 	print_alloc_info(addr, actual_size); 
 */

	printf("Allocating finished at %i\n", ((char*) current_free) + sizeof(busy_block_s) - memory);
	printf("Now the first_free is %i\n", (char*)first_free - memory);
	return ((char*) current_free) + sizeof(busy_block_s);
}

/**
 * Merge the block to the left free neighbour
 * Else, if there's no left neighbour, it creates a new free block
 *
 * Suppose that the free blocks are linked by address
 */
void memory_free(char *p){
	char ok = 0;
	char *cursor;
	free_block_s new;
	free_block_t cur = first_free;
	free_block_t prev = first_free;
	busy_block_t to_be_freed = (busy_block_t) (p - sizeof(busy_block_s));
	
	print_free_info(p); 
	printf("\nStart of memory_free\n");
	printf("p-memory : %i\n",to_be_freed - (busy_block_t) memory);
	printf("Block to be freed : size %i\n", to_be_freed->size);

	/* Searching for a left neighbour */
	cursor = memory;
	while ((int)cursor + (int)(cur->size) + \
			(int)sizeof(free_block_s) != (int)p){
		ok = 0;
		if (cur->next == NULL)
			break;
		cursor = (char *)cur->next;
		cur = cur->next;
		ok = 1;
	}
	if (ok){
		cur->size += to_be_freed->size + sizeof(busy_block_s);
		return;
	}

	/* Searching for a right neighbour */
	cur = first_free;
	while (cur != NULL){
		printf("Whiling %i\n", cur - (free_block_t) memory);
		if (cur == to_be_freed + to_be_freed->size +\
			sizeof(busy_block_s)){
			new.size = cur->size + to_be_freed->size +\
				   sizeof(busy_block_s);
			new.next = cur->next;
			memcpy(to_be_freed, &new, sizeof(free_block_s));
			if (prev == first_free){
				first_free = (free_block_t) to_be_freed;
				printf("changing first_free : %i\n", \
					first_free-(free_block_t)memory);
			}
			else
				prev->next = (free_block_t) to_be_freed;
			return;
		}
		prev = cur;
		cur = cur->next;
	}

	/* No neighbour could be found */
	/* Determining in which index we must insert the new free block */
	cur = first_free;
	while ((char *)(cur->next) < p && cur->next != NULL)
		cur = cur->next;

	/* Then, writing and inserting the new free block */
	new.size = to_be_freed->size + sizeof(busy_block_s) -\
		   sizeof(free_block_s);
	new.next = cur->next;
	memcpy(p, &new, sizeof(free_block_s));
	cur->next = (free_block_t) p;
	printf("End of memory_free !\n");
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
		printf("nique\n");
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
