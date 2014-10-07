#include "mem_alloc.h"

int main(void)
{
	char *a, *b;

	memory_init();

	a = memory_alloc(sizeof(char));
	b = memory_alloc(sizeof(char));
	memory_free(b);
	memory_free(b);

	return 0;
}
