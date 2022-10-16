#include "fastmath.h"

void xfft_square_hermitian(double* b, long n)
{
	register long k;
	register long half = n >> 1;
	register double c;
	register double d;

	b[0] *= b[0];
	b[half] *= b[half];

	for (k = 1; k < half; k++)
	{
		c = b[k];
		d = b[n - k];

		b[n - k] = 2.0 * c * d;
		b[k] = (c + d) * (c - d);
	}
}
