#include <assert.h>

#include "intmath.h"

// a = b^c
void iexp(ireg* a, ireg* b, ireg* c)
{
	if (b->value == NULL || c->value == NULL || b->digits < 1 || c->digits < 1)
		iinvparm("iexp");

	imovk(a, 1);

	if (isgn(c) <= 0)
		return;

	long f = 0;

	for (long i = c->digits - 1; i >= 0; i--)
	{
		for (unsigned long j = 0x80000000; j >= 1; j >>= 1)
		{
			if (f)
				isqu(a);

			assert(c->value != NULL);
			if (c->value[i] & j)
			{
				imul(a, b);
				f++;
			}
		}
	}
}
