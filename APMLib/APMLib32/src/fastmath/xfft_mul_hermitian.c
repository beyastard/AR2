#include "fastmath.h"

void xfft_mul_hermitian(double* a, double* b, int n)
{
	register int k;
	register int half = n >> 1;

	register double aa;
	register double bb;
	register double am;
	register double bm;

	a[0] *= b[0];
	a[half] *= b[half];

	for (k = 1; k < half; k++)
	{
		aa = a[k];
		bb = b[k];
		am = a[n - k];
		bm = b[n - k];

		a[k] = aa * bb - am * bm;
		a[n - k] = aa * bm + am * bb;
	}
}
