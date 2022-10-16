#include "intmath.h"

// a = [a / 2^k],  (k = 32-bit positive integer)
void idiv2k(ireg* a, long k)
{
	static long d;

	if (a->value == NULL)
		iinvparm("idiv2k");

	if (a->digits < 1)
		iinvparm("idiv2k");

	d = k >> 5;

	if (d < a->digits)
	{
		xdiv2k(a->value, k, a->digits);
		a->digits -= d;
	}
	else
		imovk(a, 0);
}
