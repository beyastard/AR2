#include "intmath.h"

// a = a * b
void imul(ireg* a, ireg* b)
{
	static long da2;
	static long db2;
	static long dab;
	static long dmax2;
	static long dmax4;
	register long L;

	if (a->value == NULL)
		iinvparm("imul");

	if (b->value == NULL)
		iinvparm("imul");

	if (a->digits < 1)
		iinvparm("imul");

	if (b->digits < 1)
		iinvparm("imul");

	utrim(a);
	utrim(b);

	da2 = (a->digits + a->digits);
	db2 = (b->digits + b->digits);
	dab = a->digits + b->digits;

	if (a->digits < FFTmul || b->digits < FFTmul) // School method
	{
		if (a->capacity < dab)
			uextend(a, dab);

		xmul(a->value, b->value, a->digits, b->digits);
	}
	else // Traditional FFT method
	{
		// Allocate FFT vectors
		// Set L = least power of two greater than 4 * max(d(a), d(b))
		// FFT elements are 16 bits
		// BUG! This routine does not work well with numbers >= 2^1000000
		dmax2 = a->digits;

		if (dmax2 < b->digits)
			dmax2 = b->digits;

		dmax2 += dmax2;
		dmax4 = dmax2 + dmax2;

		L = 1;

		while (L <= dmax4)
			L += L;

		if (a->capacity <= (L >> 1))
			uextend(a, 1 + (L >> 1));

		xfft_init(L, 3);
		xfft_pack(FFTv1, (unsigned short*)a->value, L, da2);
		xfft_pack(FFTv2, (unsigned short*)b->value, L, db2);
		xfft_real_to_hermitian(FFTv1, L);
		xfft_real_to_hermitian(FFTv2, L);
		xfft_mul_hermitian(FFTv1, FFTv2, L);
		xfft_inv_hermitian_to_real(FFTv1, L);
		xfft_unpack(FFTv1, (unsigned short*)a->value, L);
	}

	a->digits = dab;
	a->flags = a->flags ^ (b->flags & NegativeReg);
}
