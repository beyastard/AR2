#include "intmath.h"

// a = (b^c mod q)
// 
// Assumes b, c >= 0, q > 0
void iexpmod(ireg* a, ireg* b, ireg* c, ireg* q)
{
	static long i;
	static long f;
	static unsigned long j;

	if (b->value == NULL)
		iinvparm("iexpmod");

	if (b->digits < 1)
		iinvparm("iexpmod");

	if (c->value == NULL)
		iinvparm("iexpmod");

	if (c->digits < 1)
		iinvparm("iexpmod");

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
				isqu(a);
				imod(a, q);
			}

			if (c->value[i] & j)
			{
				imul(a, b);
				imod(a, q);
				f++;
			}
		}
	}
}
