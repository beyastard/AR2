#include "intmath.h"

// Display an integer register in decimal
void idisp(ireg* a)
{
	static long i;
	static long ddigits;
	static long dec;
	static long significant;

	if (a->value == NULL)
		iinvparm("idisp");

	if (a->digits < 1)
		iinvparm("idisp");

	// Decimal occupies at least log(2^32)/log(10^9) times as much space
	ddigits = ((9 * a->digits) >> 3) + 4;

	if (res->value == NULL)
		ualloc(res, ddigits, 1);

	if (res->capacity < ddigits)
		uextend(res, ddigits);

	// Repeatedly divide by 10^9, saving remainder starting at ddigits - 1
	umov(res, a);
	res->digits = a->digits;
	dec = ddigits;

	while (xsig(res->value, res->digits))
	{
		dec--;
		res->value[dec] = xdivk(res->value, 1000000000, res->digits);
		utrim(res);
	}

	// Print the remainders
	if (isgn(a) < 0)
		printf("-");

	significant = 0;

	for (i = dec; i < ddigits; i++)
	{
		if (res->value[i] && !significant)
		{
			printf("%d", res->value[i]);
			significant = 1;
		}
		else
			printf("%09d", res->value[i]);
	}

	if (!significant)
		printf("0");
}
