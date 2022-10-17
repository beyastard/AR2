#include <assert.h>

#include "intmath.h"

// a = (b^c mod q)
// 
// Assumes b, c >= 0, q > 0
void iexpmod(ireg* a, ireg* b, ireg* c, ireg* q)
{
	if (b->value == NULL || c->value == NULL || b->digits < 1 || c->digits < 1)
		iinvparm("iexpmod");

	imovk(a, 1);

	if (isgn(c) <= 0)
		return;

	long f = 0;

	for (long i = c->digits - 1; i >= 0; i--)
	{
		for (unsigned long j = 0x80000000; j >= 1; j >>= 1)
		{
			if (f)
			{
				isqu(a);
				imod(a, q);
			}

			assert(c->value != NULL);
			if (c->value[i] & j)
			{
				imul(a, b);
				imod(a, q);
				f++;
			}
		}
	}
}
