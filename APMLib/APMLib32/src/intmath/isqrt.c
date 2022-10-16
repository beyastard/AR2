#include "intmath.h"

// Set a = [b^(1/2)], res = b - [b^(1/2)]^2.
void isqrt(ireg* a, ireg* b)
{
	static unsigned long j;
	static unsigned long w;
	static double u;

	if (b->value == NULL)
		iinvparm("isqrt");

	if (b->digits < 1)
		iinvparm("isqrt");

	if (isgn(b) < 0)
		iinvparm("isqrt");

	imovk(temp1, 1);

	if (icmp(b, temp1) <= 0)
	{
		imov(a, b);
		imovk(res, 0);
		return;
	}

	// Compute a = b^(1/2) by Newton's method
	// Compute first approximation to a
	u = ilog(b) / 2;
	j = floor(u / ilog2);

	if (j >= 30)
		j -= 30;
	else
		j = 0;

	w = floor(exp(u - ilog2 * j));

	// Our first guess for a is |w|.
	imovk(a, w);
	iabs(a);
	imul2k(a, j);

	// Iterate a -> (b + a^2) / (2*a)
	for (;;)
	{
		imov(temp2, a); // save old a
		isqu(a);
		iadd(a, b);     // a' = b + a^2
		idiv(a, temp2);
		idiv2k(a, 1);   // a' = (b + a^2)/(2*a)
		isub(temp2, a);
		iabs(temp2);

		if (icmp(temp2, temp1) <= 0)
			break;
	}

	// a = [b^(1/2)] +/- 1
	iaddk(a, 1);
	imov(temp2, a);
	isqu(temp2);
	imov(res, b);
	isub(res, temp2); // res = b - a^2

	while (isgn(res) == 1)
	{
		isub(res, a);
		iaddk(a, 1);
		isub(res, a);
	}

	while (isgn(res) == -1)
	{
		iadd(res, a);
		isubk(a, 1);
		iadd(res, a);
	}
}
