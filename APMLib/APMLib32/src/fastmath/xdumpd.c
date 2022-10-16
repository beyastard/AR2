#include "fastmath.h"

// Unformatted print of an array
void xdumpd(double* a, long d)
{
	static long i;

	printf("%ld ", d);

	for (i = 0; i < d; i++)
		printf("%e ", a[i]);

	printf("\n");
}
