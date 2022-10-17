#include "intmath.h"

// Return [|a|/2^k] mod 2, the value of bit k of a.
// Bit 0 is the units bit.
long ibit(ireg* a, long k)
{
	const long i = k >> 5;

	return i >= a->digits ? 0 : a->value[i] & 1 << (k & 31) ? 1 : 0;
}
