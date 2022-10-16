#include "regmem.h"

// Extend an integer register
// 
// a = integer register
// d = number of digits to extend integer register to
void uextend(ireg* a, long d)
{
	static long* v;

	if (a->value == NULL)
	{
		ualloc(a, d, d);
		return;
	}

	if (d <= a->capacity)
		return;

	// Allocate new array, copy old into new, free old
	v = (long*)realloc(a->value, sizeof(long) * d);

	if (v != NULL)
	{
		a->value = v;
		a->capacity = d;
	}
	else
	{
		printf("Unable to allocate %ld extra digits in uextend\n", d - a->capacity);
		give_up();
	}
}
