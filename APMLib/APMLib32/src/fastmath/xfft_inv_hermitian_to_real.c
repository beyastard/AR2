#include "fastmath.h"

// Input  is   {Re(z^[0]), ..., Re(z^[n/2]), Im(z^[n/2-1]), ..., Im(z^[1]).
// Output is n*{z[0], z[n/2], z[n/4], ..., z[n-1]} in bit-reversed order
// This is a decimation-in-frequency, split-radix algorithm.
void xfft_inv_hermitian_to_real(double* z, long n)
{
	register double cc1;
	register double ss1;
	register double cc3;
	register double ss3;
	register double e;

	register long is;
	register long id;

	register long i0;
	register long i1;
	register long i2;
	register long i3;
	register long i4;
	register long i5;
	register long i6;
	register long i7;

	register long a;
	register long a3;
	register long scmask = FFTdim - 1;
	register long dil;

	long nn = n >> 1;

	double t0;
	double t1;
	double t2;
	double t3;
	double t4;

	long n2;
	long n4;
	long n8;

	long i;
	long j;

	n2 = n << 1;

	while (nn >>= 1)
	{
		is = 0;
		id = n2;

		n2 >>= 1;
		n4 = n2 >> 2;
		n8 = n4 >> 1;

		do
		{
			for (i = is; i < n; i += id)
			{
				i0 = i;
				i1 = i0 + n4;
				i2 = i1 + n4;
				i3 = i2 + n4;

				t0 = z[i0] - z[i2];

				z[i0] += z[i2];
				z[i1] += z[i1];
				z[i2] = t0 - 2.0 * z[i3];
				z[i3] = t0 + 2.0 * z[i3];

				if (n4 == 1)
					continue;

				i0 += n8;
				i1 += n8;
				i2 += n8;
				i3 += n8;

				t0 = (z[i1] - z[i0]) * SQRTHALF;
				t1 = (z[i3] + z[i2]) * SQRTHALF;

				z[i0] += z[i1];
				z[i1] = z[i3] - z[i2];
				z[i2] = -2.0 * (t1 + t0);
				z[i3] = 2.0 * (t0 - t1);
			}

			is = (id << 1) - n2;
			id <<= 2;
		} while (is < n - 1);

		dil = FFTdim / n2;
		a = dil;

		for (j = 1; j < n8; j++)
		{
			a3 = (a + (a << 1)) & scmask;

			//cc1 = xcos(a);
			if (a < FFTdimq1)
				cc1 = FFTsincos[FFTdimq1 - a];
			else if (a < FFTdimq2)
				cc1 = -FFTsincos[a - FFTdimq1];
			else if (a < FFTdimq3)
				cc1 = -FFTsincos[FFTdimq3 - a];
			else
				cc1 = FFTsincos[a - FFTdimq3];

			//cc3 = xcos(a3);
			if (a3 < FFTdimq1)
				cc3 = FFTsincos[FFTdimq1 - a3];
			else if (a3 < FFTdimq2)
				cc3 = -FFTsincos[a3 - FFTdimq1];
			else if (a3 < FFTdimq3)
				cc3 = -FFTsincos[FFTdimq3 - a3];
			else
				cc3 = FFTsincos[a3 - FFTdimq3];

			//ss1 = xsin(a);
			if (a < FFTdimq1)
				ss1 = FFTsincos[a];
			else if (a < FFTdimq2)
				ss1 = FFTsincos[FFTdimq2 - a];
			else if (a < FFTdimq3)
				ss1 = -FFTsincos[a - FFTdimq2];
			else
				ss1 = -FFTsincos[FFTdim - a];

			//ss3 = xsin(a3);
			if (a3 < FFTdimq1)
				ss3 = FFTsincos[a3];
			else if (a3 < FFTdimq2)
				ss3 = FFTsincos[FFTdimq2 - a3];
			else if (a3 < FFTdimq3)
				ss3 = -FFTsincos[a3 - FFTdimq2];
			else
				ss3 = -FFTsincos[FFTdim - a3];

			a = (a + dil) & scmask;
			is = 0;
			id = n2 << 1;

			do
			{
				for (i = is; i < n; i += id)
				{
					i0 = i + j;
					i1 = i0 + n4;
					i2 = i1 + n4;
					i3 = i2 + n4;
					i4 = i + n4 - j;
					i5 = i4 + n4;
					i6 = i5 + n4;
					i7 = i6 + n4;

					t0 = z[i0] - z[i5];
					z[i0] += z[i5];

					t1 = z[i4] - z[i1];
					z[i4] += z[i1];

					t2 = z[i7] + z[i2];
					z[i5] = z[i7] - z[i2];

					t3 = z[i3] + z[i6];
					z[i1] = z[i3] - z[i6];

					t4 = t0 - t3;
					t0 += t3;
					t3 = t1 - t2;
					t1 += t2;

					z[i2] = t4 * cc1 + t3 * ss1;
					z[i6] = -t3 * cc1 + t4 * ss1;
					z[i3] = t0 * cc3 - t1 * ss3;
					z[i7] = t1 * cc3 + t0 * ss3;
				}

				is = (id << 1) - n2;
				id <<= 2;
			} while (is < n - 1);
		}
	}

	is = 1;
	id = 4;

	do
	{
		for (i1 = is; i1 <= n; i1 += id)
		{
			i0 = i1 - 1;
			e = z[i0];

			z[i0] = e + z[i1];
			z[i1] = e - z[i1];
		}

		is = (id << 1) - 1;
		id <<= 2;
	} while (is < n);
}
