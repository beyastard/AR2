#include "fastmath.h"

void xfft_pack(double* z, unsigned short* x, long n, long w)
{
	register long j;
	register long r;
	register long rb;
	register long half = n >> 1;

	for (j = 0, r = 0; j < w; j++)
	{
		z[r] = x[j];
		inc_bit_rev(r);
	}

	while (r)
	{
		z[r] = 0.0;
		inc_bit_rev(r);
	}
}
