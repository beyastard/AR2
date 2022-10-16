#include "fastmath.h"

double xsin(long n)
{
	if (n < FFTdimq1)
		return FFTsincos[n];

	if (n < FFTdimq2)
		return FFTsincos[FFTdimq2 - n];

	if (n < FFTdimq3)
		return -FFTsincos[n - FFTdimq2];

	return -FFTsincos[FFTdim - n];
}
