#include <assert.h>

#include "intmath.h"

// Write an integer register in decimal to a file
void idispf(FILE* f, ireg* a)
{
	if (a->value == NULL || a->digits < 1)
		iinvparm("idisp");

	// Decimal occupies at least log(2^32)/log(10^9) times as much space
    const long ddigits = ((9 * a->digits) >> 3) + 4;

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
		(void)fprintf(f, "-");

	long significant = 0;

	for (long i = dec; i < ddigits; i++)
	{
		if (res->value[i] && !significant)
		{
			(void)fprintf(f, "%ld", res->value[i]);
			significant = 1;
		}
		else
			(void)fprintf(f, "%09ld", res->value[i]);
	}

	if (!significant)
		(void)fprintf(f, "0");
}
