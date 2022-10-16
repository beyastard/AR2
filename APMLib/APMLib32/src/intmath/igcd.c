#include "intmath.h"

// a = gcd(a, b)  (b is destroyed in the process)
void igcd(ireg* a, ireg* b)
{
	static long sb;

	if (a->value == NULL)
		iinvparm("igcd");

	if (a->digits < 1)
		iinvparm("igcd");

	if (b->value == NULL)
		iinvparm("igcd");

	if (b->digits < 1)
		iinvparm("igcd");

	if (isgn(a) < 0)
		ineg(a);

	sb = isgn(b);

	if (sb < 0)
		ineg(b);

	while (sb)
	{
		imod(a, b);

		if (!usig(a))
		{
			imov(a, b);
			return;
		}

		imod(b, a);

		if (!usig(b))
			return;
	}
}
