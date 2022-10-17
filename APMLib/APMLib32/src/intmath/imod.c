#include <assert.h>

#include "intmath.h"

// a = (a mod b) >= 0, b > 0
// 
// (b is trimmed by this routine)
void imod(ireg* a, ireg* b)
{
	if (a->value == NULL || b->value == NULL || a->digits < 1 || b->digits < 1 || isgn(b) <= 0)
		iinvparm("imod");

	utrim(a);
	utrim(b);

	if (b->digits > 1)
	{
		if (a->digits >= b->digits)
		{
			if (a->capacity <= a->digits)
				uextend(a, a->digits + 1);

			xmod(a->value, b->value, a->digits, b->digits);
			a->digits = b->digits;
		}

		// Remainder must be non-negative
		if (isgn(a) < 0)
			iadd(a, b);
	}
	else // b->digits = 1
    {
		assert(b->value != NULL);
        imodk(a, b->value[0]);
    }
}
