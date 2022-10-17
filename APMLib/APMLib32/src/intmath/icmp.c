#include "intmath.h"

// sgn(a - b) = -1 if a < b, 0 if a = b, 1 if a > b
long icmp(ireg* a, ireg* b)
{
	long s;

	if (a->value == NULL || b->value == NULL || a->digits < 1 || b->digits < 1)
		iinvparm("icmp");

	if (!(a->flags & NegativeReg))
	{
		if (!(b->flags & NegativeReg))
		{
			s = xcmp(a->value, b->value, a->digits, b->digits); // a >= 0, b >= 0

			return s & ZeroFlag ? 0 : s & CarryFlag ? -1 : 1;
		}

		return isgn(a) ? 1 : isgn(b) ? 1 : 0; // a >= 0, b <= 0
	}

	if (b->flags & NegativeReg)
	{
		s = xcmp(b->value, a->value, b->digits, a->digits); // a <= 0, b <= 0

		return s & ZeroFlag ? 0 : s & CarryFlag ? -1 : 1;
	}

	return isgn(a) ? -1 : isgn(b) ? -1 : 0; // a <= 0, b >= 0
}
