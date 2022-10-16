#include "regmem.h"

// Unformatted print of an integer register
void udump(ireg* a)
{
	static long i;

	printf("c=%ld d=%ld f=%ld &v=%ld v=", a->capacity, a->digits, a->flags, (long)a->value);

	for (i = 0; i < a->digits; i++)
		printf("%d ", (unsigned)a->value[i]);

	printf("\n");
}
