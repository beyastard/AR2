#include "intmath.h"

// sgn(a - k) = -1 if a < k, 0 if a = k, 1 if a > k, -2^31 < k < 2^31
long icmpk(ireg* a, long k)
{
	static long s;
	static long b;

	if (a->value == NULL)
		iinvparm("icmp");

	if (a->digits < 1)
		iinvparm("icmp");

	if (k > 0)
	{
		if (!(a->flags & NegativeReg))
		{
			b = k; // a >= 0, k > 0
			s = xcmp(a->value, &b, a->digits, 1);

			if (s & ZeroFlag)
				return 0;

			if (s & CarryFlag)
				return -1;
			else
				return 1;
		}
		else
			return -1; // a <= 0, k > 0
	}
	else if (k < 0)
	{
		if (a->flags & NegativeReg)
		{
			b = -k; // a <= 0, k < 0
			s = xcmp(&b, a->value, 1, a->digits);

			if (s & ZeroFlag)
				return 0;

			if (s & CarryFlag)
				return -1;
			else
				return 1;
		}
		else
			return 1; // a >= 0, k < 0
	}
	else
		return isgn(a); // k = 0
}
