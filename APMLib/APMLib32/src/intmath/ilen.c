#include "intmath.h"

// Return [log(|a|)/log(2)] + 1, the length of a in bits.
long ilen(ireg* a)
{
	static long i;
	static long k;
	static unsigned long v;
	static unsigned long w;

	if (!isgn(a))
		return 0;

	k = a->digits << 5;

	for (i = a->digits - 1; i >= 0; i--)
	{
		v = a->value[i];

		for (w = 0x80000000; w != 0; w >>= 1)
		{
			if (v & w)
				return k;

			k--;
		}
	}

	return 0;
}
