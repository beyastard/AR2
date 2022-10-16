#include "fastmath.h"

void xfft_unpack(double* z, unsigned short* x, long n)
{
	register long j;
	register long r;
	register long rb;
	register long half = n >> 1;

	register double d;
	register double e;
	register double f;
	register double g;
	register double h;

	maxFFTerror = 0;
	d = 1 / (double)n;
	g = 0.0;

	for (j = 0, r = 0; j < n; j++)
	{
		h = z[r] * d;
		f = floor(h + 0.5);

		if (checkFFTerror)
		{
			e = fabs(f - h);

			if (e > maxFFTerror)
				maxFFTerror = e;
		}

		f += g;
		g = floor(f * TWOM16);
		f -= (g * TWO16);
		x[j] = f;
		inc_bit_rev(r);
	}

	if (g < TWO16)
		x[n] = g;
	else
		maxFFTerror += 100;

	x[n + 1] = 0;

	if (checkFFTerror)
	{
		if (maxFFTerror > 0.25)
		{
			if (maxFFTerror > 0.5)
				printf("FFT error: maxFFTerror=%g \n", maxFFTerror);
			else
				printf("FFT warning: maxFFTerror=%g \n", maxFFTerror);
		}
	}
}
