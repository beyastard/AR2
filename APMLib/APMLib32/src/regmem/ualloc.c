#include "regmem.h"

// Allocate array memory for an integer register
// 
// a = integer register
// c = capacity of integer register in digits
// d = initial number of digits
void ualloc(ireg* a, long c, long d)
{
	a->capacity = max(c, 1);
	a->digits = min(max(d, 1), c);
	a->flags = 0;
	a->value = (long*)malloc(sizeof(long) * c);

	if (a->value != NULL)
		xmovz(a->value, a->digits);
	else
	{
		printf("Unable to allocate %ld digits in ualloc\n", a->capacity);
		give_up();
	}
}
