#include "intmath.h"

// a = a * k
void imulk(ireg* a, long k)
{
	if (a->value == NULL || a->digits < 1)
		iinvparm("imulk");

	utrim(a);

	if (a->capacity <= a->digits)
		uextend(a, a->digits + 1);

	if (k < 0)
	{
		xmulk(a->value, -k, a->digits);
		a->flags ^= NegativeReg;
	}
	else
		xmulk(a->value, k, a->digits);

	a->digits++;
}
