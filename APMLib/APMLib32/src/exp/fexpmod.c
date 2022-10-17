#include <assert.h>

#include "exp.h"

// a = (b^c mod q), using squaring function fsqu
// 
// Assumes b, c >= 0, q > 0
void fexpmod(ireg* a, ireg* b, ireg* c, ireg* q, void (fsqu)(ireg* a))
{
	static long i;
	static long f;
	static unsigned long j;

	if (b->value == NULL || c->value == NULL || b->digits < 1 || c->digits < 1)
		iinvparm("fexpmod");

    imovk(a, 1);

	if (isgn(c) <= 0)
		return;

	f = 0;

	for (i = c->digits - 1; i >= 0; i--)
	{
		for (j = 0x80000000; j >= 1; j >>= 1)
		{
			if (f)
			{
				fsqu(a);
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
