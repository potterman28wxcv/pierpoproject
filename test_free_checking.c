#include "mem_alloc.h"

int main(void)
{
	long *a, *b;

	memory_init();
	a = (long *) memory_alloc(sizeof(long));
	b = (long *) memory_alloc(sizeof(long));
	memory_free((char *) a);

	return 0;
}
