#include "fastmath.h"

// Unformatted print of an array
void xdump(long* a, long d)
{
	static long i;

	printf("%ld ", d);

	for (i = 0; i < d; i++)
		printf("%d ", (unsigned long)a[i]);

	printf("\n");
}
