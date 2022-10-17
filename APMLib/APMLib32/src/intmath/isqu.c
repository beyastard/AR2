#include "intmath.h"

// a = a^2
void isqu(ireg* a)
{
	static long da2;
	static long da4;
	register long L;

	if (a->value == NULL || a->digits < 1)
		iinvparm("isqu");

	utrim(a);
	da2 = (a->digits + a->digits);

	if (a->digits < FFTsqu) // School method
	{
		if (a->capacity < da2)
			uextend(a, da2);

		xsqu(a->value, a->digits);
	}
	else // Traditional FFT method
	{
		// Allocate FFT vector
		// Set L = least power of two greater than 4 * d(a)
		// FFT elements are 16 bits
		// BUG! This routine does not work well with numbers >= 2^1000000
		da4 = da2 + da2;
		L = 1;

		while (L <= da4)
			L += L;

		if (a->capacity <= (L >> 1))
			uextend(a, 1 + (L >> 1));

		xfft_init(L, 1);
		xfft_pack(FFTv1, (unsigned short*)a->value, L, da2);
		xfft_real_to_hermitian(FFTv1, L);
		xfft_square_hermitian(FFTv1, L);
		xfft_inv_hermitian_to_real(FFTv1, L);
		xfft_unpack(FFTv1, (unsigned short*)a->value, L);
	}

	a->digits = da2;
	a->flags = 0;
}
