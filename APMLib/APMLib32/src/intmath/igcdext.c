#include "intmath.h"

// Return u, d, where a*u = d (mod b), d = gcd(a, b)
void igcdext(ireg* u, ireg* d, ireg* a, ireg* b)
{
	if (a->value == NULL)
		iinvparm("igcdext");

	if (a->digits < 1)
		iinvparm("igcdext");

	if (b->value == NULL)
		iinvparm("igcdext");

	if (b->digits < 1)
		iinvparm("igcdext");

	if (isgn(b) <= 0)
		iinvparm("igcdext");

	imov(d, a);
	imod(d, b);
	imovk(u, 1);
	imovk(temp1, 0);
	imov(temp2, b);

	while (usig(temp2))
	{
		imov(temp5, d);
		idiv(temp5, temp2);
		imov(temp4, res);
		imul(temp5, temp1);
		imov(temp3, u);
		isub(temp3, temp5);
		imov(u, temp1);
		imov(d, temp2);
		imov(temp1, temp3);
		imov(temp2, temp4);
	}

	imod(u, b);
}
