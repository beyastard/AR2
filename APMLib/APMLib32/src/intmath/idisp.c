#include <assert.h>

#include "intmath.h"

// Display an integer register in decimal
void idisp(ireg* a)
{
	if (a->value == NULL || a->digits < 1)
		iinvparm("idisp");

	// Decimal occupies at least log(2^32)/log(10^9) times as much space
	long ddigits = ((9 * a->digits) >> 3) + 4;

	if (res->value == NULL)
		ualloc(res, ddigits, 1);

	if (res->capacity < ddigits)
		uextend(res, ddigits);

	// Repeatedly divide by 10^9, saving remainder starting at ddigits - 1
	umov(res, a);
	res->digits = a->digits;
	long dec = ddigits;

	while (xsig(res->value, res->digits))
	{
		dec--;
		assert(res->value != NULL);
		res->value[dec] = xdivk(res->value, 1000000000, res->digits);
		utrim(res);
	}

	// Print the remainders
	if (isgn(a) < 0)
		printf("-");

	long significant = 0;

	for (long i = dec; i < ddigits; i++)
	{
		if (res->value[i] && !significant)
		{
			printf("%ld", res->value[i]);
			significant = 1;
		}
		else
			printf("%09ld", res->value[i]);
	}

	if (!significant)
		printf("0");
}
