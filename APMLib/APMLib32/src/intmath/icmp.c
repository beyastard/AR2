#include "intmath.h"

// sgn(a - b) = -1 if a < b, 0 if a = b, 1 if a > b
long icmp(ireg* a, ireg* b)
{
	static long s;

	if (a->value == NULL)
		iinvparm("icmp");

	if (a->digits < 1)
		iinvparm("icmp");

	if (b->value == NULL)
		iinvparm("icmp");

	if (b->digits < 1)
		iinvparm("icmp");

	if (!(a->flags & NegativeReg))
	{
		if (!(b->flags & NegativeReg))
		{
			s = xcmp(a->value, b->value, a->digits, b->digits); // a >= 0, b >= 0

			if (s & ZeroFlag)
				return 0;

			if (s & CarryFlag)
				return -1;
			else
				return 1;
		}
		else
		{
			if (isgn(a))
				return 1; // a >= 0, b <= 0

			if (isgn(b))
				return 1;

			return 0;
		}
	}
	else
	{
		if (b->flags & NegativeReg)
		{
			s = xcmp(b->value, a->value, b->digits, a->digits); // a <= 0, b <= 0

			if (s & ZeroFlag)
				return 0;

			if (s & CarryFlag)
				return -1;
			else
				return 1;
		}
		else
		{
			if (isgn(a))
				return -1; // a <= 0, b >= 0

			if (isgn(b))
				return -1;

			return 0;
		}
	}
}
