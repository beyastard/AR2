#include "intmath.h"

// a = (a mod k) >= 0,  k = 32-bit positive integer
void imodk(ireg* a, long k)
{
	if (a->value == NULL)
		iinvparm("imodk");

	if (a->digits < 1)
		iinvparm("imodk");

	if (k == 0)
		iinvparm("imodk");

	utrim(a);

	xmodk(a->value, k, a->digits);
	a->digits = 1;

	// Result must be non-negative
	if ((a->flags & NegativeReg) && a->value[0])
	{
		a->value[0] = k - a->value[0];
		a->flags ^= NegativeReg;
	}
}
