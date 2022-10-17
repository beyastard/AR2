#include <assert.h>

#include "intmath.h"

// a = a + k, -2^31 < k < 2^31
void iaddk(ireg* a, long k)
{
	long f;

	if (a->value == NULL || a->digits < 1)
		iinvparm("iaddk");

	if (k >= 0)
	{
		if (!(a->flags & NegativeReg))
		{
			f = xaddk(a->value, k, a->digits);

			if (f & CarryFlag)
			{
				if (a->capacity <= a->digits)
					uextend(a, a->digits + 1);

				assert(a->value != NULL);
				a->value[a->digits] = 1;
				a->digits++;
			}
		}
		else
		{
			f = xsubk(a->value, k, a->digits);

			if (f & CarryFlag)
			{
				xneg(a->value, a->digits);
				a->flags ^= NegativeReg;
			}
		}
	}
	else // k < 0
	{
		if (!(a->flags & NegativeReg))
		{
			f = xsubk(a->value, -k, a->digits);

			if (f & CarryFlag)
			{
				xneg(a->value, a->digits);
				a->flags ^= NegativeReg;
			}
		}
		else
		{
			f = xaddk(a->value, -k, a->digits);

			if (f & CarryFlag)
			{
				if (a->capacity <= a->digits)
					uextend(a, a->digits + 1);

				assert(a->value != NULL);
				a->value[a->digits] = 1;
				a->digits++;
			}
		}
	}
}
