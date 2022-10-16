#include "intmath.h"

// a = [a * 2^k],  (k = 32-bit positive integer)
void imul2k(ireg* a, long k)
{
	static long da;

	if (a->value == NULL)
		iinvparm("imul2k");

	if (a->digits < 1)
		iinvparm("imul2k");

	da = (k + 31) >> 5;

	if (a->capacity < a->digits + da)
		uextend(a, a->digits + da);

	xmul2k(a->value, k, a->digits);
	a->digits += da;
}
