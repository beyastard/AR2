#include "intmath.h"

// a = string of decimal digits
void imovd(ireg* a, char* c)
{
	static long i;
	static long k;
	static long s;

	if (a->value == NULL)
		ualloc(a, 8, 1);

	if (a->digits < 1)
		iinvparm("imovd");

	i = 0;
	s = 0;

	// see if continuation (_) or minus sign (-)
	if (c[0] == '_')
	{
		s = (a->flags & NegativeReg);
		i++;
	}
	else
	{
		imovk(a, 0);

		if (c[0] == '-')
		{
			s = NegativeReg;
			i++;
		}
	}

	while (1) // Stop at first non-decimal
	{
		k = c[i] - '0';

		if (k < 0 || k > 9)
			break;

		if (a->capacity <= a->digits)
			uextend(a, a->digits + 1);

		xmulk(a->value, 10, a->digits);
		a->digits++;
		xaddk(a->value, k, a->digits);

		utrim(a);
		i++;
	}

	a->flags = s;
}
