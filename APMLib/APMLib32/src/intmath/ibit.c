#include "intmath.h"

// Return [|a|/2^k] mod 2, the value of bit k of a.
// Bit 0 is the units bit.
long ibit(ireg* a, long k)
{
	static long i;

	i = k >> 5;

	if (i >= a->digits)
		return 0;

	if (a->value[i] & (1 << (k & 31)))
		return 1;

	return 0;
}
