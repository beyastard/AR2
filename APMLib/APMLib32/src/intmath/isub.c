#include "intmath.h"

// a = a - b
void isub(ireg* a, ireg *b)
{
	static long f;
	if (a->value == NULL)
		iinvparm("isub");

	if (b->value == NULL)
		iinvparm("isub");

	if (a->digits < 1)
		iinvparm("isub");

	if (b->digits < 1)
		iinvparm("isub");

	// Extend a if shorter than b
	if (a->digits < b->digits)
	{
		if (a->capacity < b->digits)
			uextend(a, b->digits);

		xmovz(&a->value[a->digits], b->digits - a->digits);
		a->digits = b->digits;
	}

	if ((a->flags & NegativeReg) != (b->flags & NegativeReg))
	{
		// Different signs
		f = xadd(a->value, b->value, a->digits, b->digits);

		if (f & CarryFlag)
		{
			if (a->capacity <= a->digits)
				uextend(a, a->digits + 1);

			a->value[a->digits] = 1;
			a->digits++;
		}
	}
	else
	{
		// Same signs
		f = xsub(a->value, b->value, a->digits, b->digits);

		if (f & CarryFlag)
		{
			xneg(a->value, a->digits);
			a->flags ^= NegativeReg;
		}
	}
}
