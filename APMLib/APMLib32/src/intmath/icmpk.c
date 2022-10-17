#include "intmath.h"

// sgn(a - k) = -1 if a < k, 0 if a = k, 1 if a > k, -2^31 < k < 2^31
long icmpk(ireg* a, long k)
{
	long s;
	long b;

	if (a->value == NULL || a->digits < 1)
		iinvparm("icmp");

	if (k > 0)
	{
		if (!(a->flags & NegativeReg))
		{
			b = k; // a >= 0, k > 0
			s = xcmp(a->value, &b, a->digits, 1);

			return s & ZeroFlag ? 0 : s & CarryFlag ? -1 : 1;
		}

		return -1; // a <= 0, k > 0
		
	}

	if (k < 0)
	{
		if (a->flags & NegativeReg)
		{
			b = -k; // a <= 0, k < 0
			s = xcmp(&b, a->value, 1, a->digits);

			return s & ZeroFlag ? 0 : s & CarryFlag ? -1 : 1;
		}

		return 1; // a >= 0, k < 0
	}

	return isgn(a); // k = 0
}
