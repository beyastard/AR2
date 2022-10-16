#include "intmath.h"

// a = b^c
void iexp(ireg* a, ireg* b, ireg* c)
{
	static long i;
	static long f;
	static unsigned long j;

	if (b->value == NULL)
		iinvparm("iexp");

	if (b->digits < 1)
		iinvparm("iexp");

	if (c->value == NULL)
		iinvparm("iexp");

	if (c->digits < 1)
		iinvparm("iexp");

	imovk(a, 1);

	if (isgn(c) <= 0)
		return;

	f = 0;

	for (i = c->digits - 1; i >= 0; i--)
	{
		for (j = 0x80000000; j >= 1; j >>= 1)
		{
			if (f)
				isqu(a);

			if (c->value[i] & j)
			{
				imul(a, b);
				f++;
			}
		}
	}
}
