#include "fastmath.h"

double xcos(long n)
{
	if (n < FFTdimq1)
		return FFTsincos[FFTdimq1 - n];

	if (n < FFTdimq2)
		return -FFTsincos[n - FFTdimq1];

	if (n < FFTdimq3)
		return -FFTsincos[FFTdimq3 - n];

	return FFTsincos[n - FFTdimq3];
}
