#include "fastmath.h"

// Free memory occupied by FFT vectors and sin/cos table
void xfft_free(void)
{
	if (FFTdimv1)
	{
		free(FFTv1);
		FFTdimv1 = 0;
		FFTv1 = NULL;
	}

	if (FFTdimv2)
	{
		free(FFTv2);
		FFTdimv2 = 0;
		FFTv1 = NULL;
	}

	if (FFTdim)
	{
		free(FFTsincos);
		FFTdim = 0;
		FFTsincos = NULL;
	}
}
