#include "intmath.h"

// a = b
void imov(ireg* a, ireg* b)
{
	if (b->value == NULL || b->digits < 1)
		iinvparm("imov");

	if (a->value == NULL)
		ualloc(a, b->digits, b->digits);
	
	if (a->capacity < b->digits)
		uextend(a, b->digits);

	xmovf(a->value, b->value, b->digits);

	a->digits = b->digits;
	a->flags = b->flags;
}
