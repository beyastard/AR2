#include "intmath.h"

// a =  [ a / k], kres =  ( a mod k), a >= 0,
// a = -[-a / k], kres = -(-a mod k), a <  0;  k = 32-bit positive integer
void idivk(ireg* a, long k)
{
	if (a->value == NULL || a->digits < 1 || k == 0)
		iinvparm("idivk");

	utrim(a);
	kres = xdivk(a->value, k, a->digits);

	if (a->flags & NegativeReg)
		kres = -kres;
}
