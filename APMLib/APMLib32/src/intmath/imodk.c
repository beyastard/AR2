#include <assert.h>

#include "intmath.h"

// a = (a mod k) >= 0,  k = 32-bit positive integer
void imodk(ireg* a, long k)
{
	if (a->value == NULL || a->digits < 1 || k == 0)
		iinvparm("imodk");

	utrim(a);

	xmodk(a->value, k, a->digits);
	a->digits = 1;

	// Result must be non-negative
	assert(a->value != NULL);
	if ((a->flags & NegativeReg) && a->value[0])
	{
		a->value[0] = k - a->value[0];
		a->flags ^= NegativeReg;
	}
}
