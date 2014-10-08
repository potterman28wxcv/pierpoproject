#include "mem_alloc.h"

int main(void)
{
	long *a, *b;

	memory_init();
	a = (long *) memory_alloc(sizeof(long));
	b = (long *) memory_alloc(sizeof(long));
	memory_free((char *) a);

	/* To avoid the -Werror issue */
	if (b){}

	return 0;
}
