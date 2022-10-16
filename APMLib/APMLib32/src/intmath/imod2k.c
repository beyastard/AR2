#include "intmath.h"

// a = (a mod 2^k) > 0,  k = 32-bit positive integer
// 
// d(a)=6, k=72           kr=8 km=255
// |
// |a a a a|a a a a|a a a a|a a a a|a a a a|
// |<-----kq------>|
// |<---------da---------->|
void imod2k(ireg* a, long k)
{
	static long da;
	static long kq;
	static long kr;
	static unsigned long km;

	if (a->value == NULL)
		iinvparm("imod2k");

	if (a->digits < 1)
		iinvparm("imod2k");

	if (k)
	{
		kq = k >> 5;
		kr = k & 31;
		km = 1 << kr;
		km--;
		da = (k + 31) >> 5;

		if (a->flags & NegativeReg)
		{
			a->flags ^= NegativeReg;

			if (!usig(a))
				return;

			// a < 0: compute 2^k - |a|
			if (a->capacity < da)
				uextend(a, da);

			if (a->digits < da)
				xmovz(&a->value[a->digits], da - a->digits);

			xneg(a->value, da);

			if (kr)
				a->value[kq] &= km;
		}
		else
		{
			if (kq >= a->digits)
				return;

			if (kr)
				a->value[kq] &= km;
		}

		a->digits = da;
	}
	else
		imovk(a, 0);
}
