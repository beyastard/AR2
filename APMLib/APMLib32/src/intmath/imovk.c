#include <assert.h>

#include "intmath.h"

// a = k, -2^31 < k < 2^31
void imovk(ireg* a, long k)
{
	if (a->value == NULL)
		ualloc(a, 8, 1);

	if (a->digits < 1)
		iinvparm("imovk");

	if (k >= 0)
	{
		assert(a->value != NULL);
		a->value[0] = k;
		a->flags = 0;
	}
	else
	{
		assert(a->value != NULL);
		a->value[0] = -k;
		a->flags = NegativeReg;
	}

	a->digits = 1;
}
