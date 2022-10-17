#include <assert.h>

#include "intmath.h"

// a = a + b
void iadd(ireg* a, ireg* b)
{
	long f;

	if (a->value == NULL || b->value == NULL || a->digits < 1 || b->digits < 1)
		iinvparm("iadd");

	// Extend a if shorter than b
	if (a->digits < b->digits)
	{
		if (a->capacity < b->digits)
			uextend(a, b->digits);

		assert(a->value != NULL);
		xmovz(&a->value[a->digits], b->digits - a->digits);
		a->digits = b->digits;
	}

	if ((a->flags & NegativeReg) == (b->flags & NegativeReg))
	{
		// Same signs
		f = xadd(a->value, b->value, a->digits, b->digits);

		if (f & CarryFlag)
		{
			if (a->capacity <= a->digits)
				uextend(a, a->digits + 1);

			assert(a->value != NULL);
			a->value[a->digits] = 1;
			a->digits++;
		}
	}
	else
	{
		// Different signs
		f = xsub(a->value, b->value, a->digits, b->digits);

		if (f & CarryFlag)
		{
			xneg(a->value, a->digits);
			a->flags ^= NegativeReg;
		}
	}
}
