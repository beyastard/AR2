#include "fastmath.h"

// Unformatted print of an array
void xdumps(short* a, long d)
{
	static long i;

	printf("%ld ", d);

	for (i = 0; i < d; i++)
		printf("%d ", (unsigned short)a[i]);

	printf("\n");
}
