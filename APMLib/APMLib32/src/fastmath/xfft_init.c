#include "fastmath.h"

// Allocate FFT vectors and initialise sin/cos table
// b = 1, 2, 3: allocate FFTv1, FFTv2, both
void xfft_init(long n, long b)
{
	static double* v;
	static long j;
	static double e;

	if (b & 1)
	{
		if (n > FFTdimv1)
		{
			v = (double*)realloc(FFTv1, sizeof(double) * n);

			if (v != NULL)
				FFTv1 = v;
			else
			{
				printf("Unable to allocate FFT %d-vector in xfft_init\n", n);
				give_up();
			}

			FFTdimv1 = n;
		}
	}

	if (b & 2)
	{
		if (n > FFTdimv2)
		{
			v = (double*)realloc(FFTv2, sizeof(double) * n);

			if (v != NULL)
				FFTv2 = v;
			else
			{
				printf("Unable to allocate FFT %d-vector in xfft_init\n", n);
				give_up();
			}

			FFTdimv2 = n;
		}
	}

	if (n > FFTdim)
	{
		e = TWOPI / n;
		j = 1 + (n >> 2);
		v = (double*)realloc(FFTsincos, sizeof(double) * j);

		if (v != NULL)
			FFTsincos = v;
		else
		{
			printf("Unable to allocate %d-vector for sin/cos table in xfft_init\n", j);
			give_up();
		}

		for (j = 0; j <= (n >> 2); j++)
			FFTsincos[j] = sin(e * j);

		FFTdim = n;
		FFTdimq2 = (FFTdim >> 1);
		FFTdimq1 = (FFTdimq2 >> 1);
		FFTdimq3 = FFTdimq1 + FFTdimq2;
	}
}
