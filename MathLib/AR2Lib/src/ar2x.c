/* AR2X.C  2.2.2E  TONY FORBES  October 1999 */

/*
FFT programs from R.E Crandall, Projects in Scientific Computation.
*/

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ar2x.h"

unsigned long Zdx;		// Save EDX
unsigned long Zbx;		// Save EBX
unsigned long Zcx;		// Save ECX
unsigned long Zbp;		// Save EBP
unsigned long Zsi;		// Save ESI
unsigned long Zdi;		// Save EDI
unsigned long Zsg;		// Sign indicator
unsigned long Zmulda;
unsigned long Zmulb0;
unsigned long Zmuldb;
unsigned long Zdiva0;	// Save &a
unsigned long Zdivb0;	// Save &b
unsigned long Zdivda;	// Save d(a)
unsigned long Zdivdb;	// Save d(b)
unsigned long Zdivq0;	// Save &q
unsigned long Zdivbz;	// b_z = number of high order zero bits of b
unsigned long Zdivv1;	// v_1 = high order 32 bits of b
unsigned long Zdivv2;	// v_2 = next 32 bits of b
unsigned long Zdivii;	// i
unsigned long Zdivai;	// &a_i
unsigned long Zdivqi;	// q_i

#define TWOPI (2 * 3.14159265358979323846)
#define SQRTHALF 0.70710678118654752440
#define TWO16 65536.0
#define TWOM16 0.0000152587890625

/* Increment bit-reversed index x */
#define inc_bit_rev(x) rb=half; while(x&rb) {x-=rb; rb>>=1;} x+=rb

extern double *FFTsincos = NULL;
extern double *FFTv1 = NULL;
extern double *FFTv2 = NULL;
extern long   FFTdim = 0;
extern long   FFTdimq1 = 0;
extern long   FFTdimq2 = 0;
extern long   FFTdimq3 = 0;
extern long   FFTdimv1 = 0;
extern long   FFTdimv2 = 0;
extern long   FFTsqu = 730;
extern long   FFTmul = 1200;
extern long   checkFFTerror = 1;
extern double maxFFTerror = 0;

double xsin(long n)
{
	if (n < FFTdimq1)
		return (FFTsincos[n]);

	if (n < FFTdimq2)
		return (FFTsincos[FFTdimq2 - n]);

	if (n < FFTdimq3)
		return (-FFTsincos[n - FFTdimq2]);

	return (-FFTsincos[FFTdim - n]);
}

double xcos(long n)
{
	if (n < FFTdimq1)
		return (FFTsincos[FFTdimq1 - n]);

	if (n < FFTdimq2)
		return (-FFTsincos[n - FFTdimq1]);

	if (n < FFTdimq3)
		return (-FFTsincos[FFTdimq3 - n]);

	return (FFTsincos[n - FFTdimq3]);
}

/* Stop. Then exit if 'X' pressed */
void stop(void)
{
	if (_getch() == 'X')
		exit(0);
}

/* Give up. */
void give_up(void)
{
	printf("Sorry, cannot continue\n");

	while (1)
		stop();
}

/*
Allocate FFT vectors and initialise sin/cos table
b = 1, 2, 3: allocate FFTv1, FFTv2, both
*/
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
			{
				FFTv1 = v;
			}
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
			{
				FFTv2 = v;
			}
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
		{
			FFTsincos = v;
		}
		else
		{
			printf("Unable to allocate %d-vector for sin/cos table in xfft_init\n", j);
			give_up();
		}

		for (j = 0; j <= (n >> 2); j++)
		{
			FFTsincos[j] = sin(e * j);
		}

		FFTdim = n;
		FFTdimq2 = (FFTdim >> 1);
		FFTdimq1 = (FFTdimq2 >> 1);
		FFTdimq3 = FFTdimq1 + FFTdimq2;
	}
}

/* Free memory occupied by FFT vectors and sin/cos table */
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

void xfft_pack(double *z, unsigned short *x, long n, long w)
{
	register long j;
	register long r;
	register long rb;
	register long half = n >> 1;

	for (j = 0, r = 0; j < w; j++)
	{
		z[r] = x[j];
		inc_bit_rev(r);
	}

	while (r)
	{
		z[r] = 0.0;
		inc_bit_rev(r);
	}
}

void xfft_unpack(double *z, unsigned short *x, long n)
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
			{
				maxFFTerror = e;
			}
		}

		f += g;
		g = floor(f * TWOM16);
		f -= (g * TWO16);
		x[j] = f;
		inc_bit_rev(r);
	}

	if (g < TWO16)
	{
		x[n] = g;
	}
	else
	{
		maxFFTerror += 100;
	}

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

/*
Input  is {z[0], z[n/2], z[n/4], ..., z[n-1]} in bit-reversed order
Output is {Re(z^[0]), ..., Re(z^[n/2]), Im(z^[n/2-1]), ..., Im(z^[1]).
This is a decimation-in-time, split-radix algorithm.
*/
void xfft_real_to_hermitian(double* z, long n)
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
	double t5;
	register long n2;
	register long n4;
	register long n8;
	register long i;
	register long j;

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

	n2 = 2;

	while (nn >>= 1)
	{
		n2 <<= 1;
		n4 = n2 >> 2;
		n8 = n2 >> 3;
		is = 0;
		id = n2 << 1;

		do
		{
			for (i = is; i < n; i += id)
			{
				i0 = i;
				i1 = i0 + n4;
				i2 = i1 + n4;
				i3 = i2 + n4;
				t0 = z[i3] + z[i2];
				z[i3] -= z[i2];
				z[i2] = z[i0] - t0;
				z[i0] += t0;

				if (n4 == 1)
					continue;

				i0 += n8;
				i1 += n8;
				i2 += n8;
				i3 += n8;
				t0 = (z[i2] + z[i3]) * SQRTHALF;
				t1 = (z[i2] - z[i3]) * SQRTHALF;
				z[i3] = z[i1] - t0;
				z[i2] = -z[i1] - t0;
				z[i1] = z[i0] - t1;
				z[i0] += t1;
			}

			is = (id << 1) - n2;
			id <<= 2;
		}
		while (is < n);

		dil = FFTdim / n2;
		a = dil;

		for (j = 1; j < n8; j++)
		{
			a3 = (a + (a << 1)) & scmask;

			/* cc1 = xcos(a) */
			if (a < FFTdimq1)
				cc1 = FFTsincos[FFTdimq1 - a];
			else if (a < FFTdimq2)
				cc1 = -FFTsincos[a - FFTdimq1];
			else if (a < FFTdimq3)
				cc1 = -FFTsincos[FFTdimq3 - a];
			else
				cc1 = FFTsincos[a - FFTdimq3];

			/* cc3 = xcos(a3) */
			if (a3 < FFTdimq1)
				cc3 = FFTsincos[FFTdimq1 - a3];
			else if (a3 < FFTdimq2)
				cc3 = -FFTsincos[a3 - FFTdimq1];
			else if (a3 < FFTdimq3)
				cc3 = -FFTsincos[FFTdimq3 - a3];
			else
				cc3 = FFTsincos[a3 - FFTdimq3];

			/* ss1 = xsin(a); */
			if (a < FFTdimq1)
				ss1 = FFTsincos[a];
			else if (a < FFTdimq2)
				ss1 = FFTsincos[FFTdimq2 - a];
			else if (a < FFTdimq3)
				ss1 = -FFTsincos[a - FFTdimq2];
			else
				ss1 = -FFTsincos[FFTdim - a];

			/* ss3 = xsin(a3); */
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
					t0 = z[i2] * cc1 + z[i6] * ss1;
					t1 = z[i6] * cc1 - z[i2] * ss1;
					t2 = z[i3] * cc3 + z[i7] * ss3;
					t3 = z[i7] * cc3 - z[i3] * ss3;
					t4 = t0 + t2;
					t5 = t1 + t3;
					t2 = t0 - t2;
					t3 = t1 - t3;
					t1 = z[i5] + t5;
					z[i2] = t5 - z[i5];
					z[i7] = t1;
					t1 = z[i1] - t2;
					z[i6] = -z[i1] - t2;
					z[i3] = t1;
					t0 = z[i0] + t4;
					z[i5] = z[i0] - t4;
					z[i0] = t0;
					t0 = z[i4] + t3;
					z[i4] -= t3;
					z[i1] = t0;
				}

				is = (id << 1) - n2;
				id <<= 2;
			}
			while (is < n);
		}
	}
}

/*
Input  is   {Re(z^[0]), ..., Re(z^[n/2]), Im(z^[n/2-1]), ..., Im(z^[1]).
Output is n*{z[0], z[n/2], z[n/4], ..., z[n-1]} in bit-reversed order
This is a decimation-in-frequency, split-radix algorithm.
*/
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
		}
		while (is < n - 1);

		dil = FFTdim / n2;
		a = dil;

		for (j = 1; j < n8; j++)
		{
			a3 = (a + (a << 1)) & scmask;

			/* cc1 = xcos(a) */
			if (a < FFTdimq1)
				cc1 = FFTsincos[FFTdimq1 - a];
			else if (a < FFTdimq2)
				cc1 = -FFTsincos[a - FFTdimq1];
			else if (a < FFTdimq3)
				cc1 = -FFTsincos[FFTdimq3 - a];
			else
				cc1 = FFTsincos[a - FFTdimq3];

			/* cc3 = xcos(a3) */
			if (a3 < FFTdimq1)
				cc3 = FFTsincos[FFTdimq1 - a3];
			else if (a3 < FFTdimq2)
				cc3 = -FFTsincos[a3 - FFTdimq1];
			else if (a3 < FFTdimq3)
				cc3 = -FFTsincos[FFTdimq3 - a3];
			else
				cc3 = FFTsincos[a3 - FFTdimq3];

			/* ss1 = xsin(a); */
			if (a < FFTdimq1)
				ss1 = FFTsincos[a];
			else if (a < FFTdimq2)
				ss1 = FFTsincos[FFTdimq2 - a];
			else if (a < FFTdimq3)
				ss1 = -FFTsincos[a - FFTdimq2];
			else
				ss1 = -FFTsincos[FFTdim - a];

			/* ss3 = xsin(a3); */
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
			}
			while (is < n - 1);
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
	}
	while (is < n);
}

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

// a = b, same size
//
// xmovf (&a, &b, d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(b)
// Assumptions: d(a) = d(b) > 0
void xmovf(long* a, long* b, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		
		mov    Zcx,ECX
LmovfB:
		mov    ECX,[EDX]         ; ECX = b_i
        lea    EDX,[EDX+4]
        mov    [EAX],ECX         ; a_i = b_i
        lea    EAX,[EAX+4]
        dec    EBX
        jg     LmovfB
        mov    ECX,Zcx
		
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
        ;ret
	}
}

// a = b
// a = b,  b sign extended
//
// xmov  (&a, &b, d(a), d(b));
// xmovs (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a) >= d(b) > 0
void xmov(long* a, long* b, long da, long db)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db

		mov    Zsg,0
		mov    Zbp,EBP
;
        sub    EBX,ECX           ; EBX = d(a) - d(b)
LmovB:
		mov    EBP,[EDX]         ; EBP = b_i
        lea    EDX,[EDX+4]
        mov    [EAX],EBP         ; Move to a
        dec    ECX
        lea    EAX,[EAX+4]
        jg     LmovB             ; More digits
;
        cmp    EBX,0 
        jbe    LmovX             ; d(a) = d(b)
        sar    EBP,31 
        and    EBP,Zsg           ; EBP = b sign bits
LmovD:
		mov    dword ptr[EAX],0  ; Extend with zero
        dec    EBX
        lea    EAX,[EAX+4]
        jg     LmovD             ; More digits
LmovX:
		mov    EBP,Zbp
		
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
        ;ret
	}
}

// a = b
// a = b,  b sign extended
//
// xmov  (&a, &b, d(a), d(b));
// xmovs (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a) >= d(b) > 0
void xmovs(long* a, long* b, long da, long db)
{
	__asm
	{
		mov    Zdx, edx
		mov    Zbx, ebx
		mov    Zcx, ecx
		mov    eax, a
		mov    edx, b
		mov    ebx, da
		mov    ecx, db

		mov    Zsg,0FFFFFFFFh
		mov    Zbp,EBP
;
        sub    EBX,ECX           ; EBX = d(a) - d(b)
LmovB:
		mov    EBP,[EDX]         ; EBP = b_i
        lea    EDX,[EDX+4]
        mov    [EAX],EBP         ; Move to a
        dec    ECX
        lea    EAX,[EAX+4]
        jg     LmovB             ; More digits
;
        cmp    EBX,0 
        jbe    LmovX             ; d(a) = d(b)
        sar    EBP,31 
        and    EBP,Zsg           ; EBP = b sign bits
LmovD:
		mov    dword ptr[EAX],0  ; Extend with zero
        dec    EBX
        lea    EAX,[EAX+4]
        jg     LmovD             ; More digits
LmovX:
		mov    EBP,Zbp
		
		mov    edx, Zdx
		mov    ebx, Zbx
		mov    ecx, Zcx
        ;ret
	}
}

// a = k                    (k = 32-bit integer)
// a = k,  k sign extended  (k = 32-bit integer)
//
// xmovk  (&a, k, d(a));
// xmovks (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: d(a) > 0
void xmovk(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zsg,0
		mov    [EAX],EDX         ; Move k to a
        dec    EBX
        jz     LmovkX
        SAR    EDX,31   
        and    EDX,Zsg           ; EDX = s sign bits
LmovkD:
		mov    [EAX+4],EDX        ; Extend with sign
        lea    EAX,[EAX+4]
        dec    EBX
        jg     LmovkD
LmovkX:
		mov    edx,Zdx
		mov    ebx,Zbx
		;ret
	}
}

// a = k                    (k = 32-bit integer)
// a = k,  k sign extended  (k = 32-bit integer)
//
// xmovk  (&a, k, d(a));
// xmovks (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: d(a) > 0
void xmovks(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zsg,0FFFFFFFFh
		mov    [EAX],EDX         ; Move k to a
        dec    EBX
        jz     LmovkX
        SAR    EDX,31   
        and    EDX,Zsg           ; EDX = s sign bits
LmovkD:
		mov    [EAX+4],EDX        ; Extend with sign
        lea    EAX,[EAX+4]
        dec    EBX
        jg     LmovkD
LmovkX:
		mov    edx,Zdx
		mov    ebx,Zbx
		;ret
	}
}

// a = 0
//
// xmovz  (&a, d(a));
//
// Registers:   EAX = &a, EDX = d(a)
void xmovz(long* a, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    eax,a
		mov    edx,da

		cmp    EDX,0
        jle    Lmovz8
Lmovz2:
		mov    dword ptr [EAX],0 
        dec    EDX
        lea    EAX,[EAX+4]
        jg     Lmovz2
Lmovz8:
		mov    edx,Zdx
		;ret
	}
}

// a = a + b (mod 2^(32*d(a)))
//
// xaddf (&a, &b, d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(b)
// Assumptions: d(a) = d(b) > 0
void xaddf(long* a, long* b, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zbp,EBP
;
        clc                      ; clear carry
LaddfB:
		mov    EBP,[EDX]         ; EBP = b_i
        mov    ECX,[EAX]         ; ECX = a_i
        lea    EDX,[EDX+4]
        adc    ECX,EBP           ; ECX = a_i + b_i + carry 
        dec    EBX
        mov    [EAX],ECX         ; Move to a
        lea    EAX,[EAX+4]
        jg     LaddfB            ; More digits
;
        mov    EBP,Zbp
        mov    ECX,Zcx
        
		mov    edx,Zdx
		mov    ebx,Zbx
		;ret
	}
}

// a = a + b (mod 2^(32*d(a)))
// a = a + b (mod 2^(32*d(a))), b sign-extended
//
// f = xadd  (&a, &b, d(a), d(b));
// f = xadds (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a) >= d(b) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xadd(long* a, long* b, long da, long db)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		mov    Zsg,0
		mov    Zbp,EBP
        mov    Zdi,EDI
;
        mov    EDI,EAX           ; EDI = &a
        sub    EBX,ECX           ; EBX = d(a) - d(b)
        clc                      ; clear carry
LaddB:
		mov    EBP,[EDX]         ; EBP = b_i
        mov    EAX,[EDI]         ; EAX = a_i
        lea    EDX,[EDX+4]
        adc    EAX,EBP           ; EAX = a_i + b_i + carry 
        dec    ECX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LaddB             ; More digits
        lahf                     ; Carry flag to AH
;
        cmp    EBX,0 
        jbe    LaddX             ; d(a) = d(b)
        sar    EBP,31 
        and    EBP,Zsg           ; EBP = b sign bits
        sahf                     ; Restore carry
LaddD:
		mov    EAX,[EDI]         ; EAX = a_i
        adc    EAX,EBP           ; EAX = a_i + b sign bits + carry 
        dec    EBX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LaddD             ; More sign bits
        lahf                     ; Carry flag to AH
LaddX:
		mov    EDI,Zdi
        mov    EBP,Zbp
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
		;ret
	}
	
	return result;
}

// a = a + b (mod 2^(32*d(a)))
// a = a + b (mod 2^(32*d(a))), b sign-extended
//
// f = xadd  (&a, &b, d(a), d(b));
// f = xadds (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a) >= d(b) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xadds(long* a, long* b, long da, long db)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		mov    Zsg,0FFFFFFFFh
		mov    Zbp,EBP
        mov    Zdi,EDI
;
        mov    EDI,EAX           ; EDI = &a
        sub    EBX,ECX           ; EBX = d(a) - d(b)
        clc                      ; clear carry
LaddB:
		mov    EBP,[EDX]         ; EBP = b_i
        mov    EAX,[EDI]         ; EAX = a_i
        lea    EDX,[EDX+4]
        adc    EAX,EBP           ; EAX = a_i + b_i + carry 
        dec    ECX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LaddB             ; More digits
        lahf                     ; Carry flag to AH
;
        cmp    EBX,0 
        jbe    LaddX             ; d(a) = d(b)
        sar    EBP,31 
        and    EBP,Zsg           ; EBP = b sign bits
        sahf                     ; Restore carry
LaddD:
		mov    EAX,[EDI]         ; EAX = a_i
        adc    EAX,EBP           ; EAX = a_i + b sign bits + carry 
        dec    EBX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LaddD             ; More sign bits
        lahf                     ; Carry flag to AH
LaddX:
		mov    EDI,Zdi
        mov    EBP,Zbp
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
		;ret
	}
	
	return result;
}

// a = a + k (mod 2^(32*d)), k = 32-bit integer
//
// xaddk (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d = d(a)
// Assumptions: k >= 0, d(a) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xaddk(long* a, long k, long da)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zdi,EDI
;
        mov    EDI,EAX           ; EDI = &a
        mov    EAX,[EDI]
        add    EAX,EDX  
        mov    [EDI],EAX         ; a_0 = a_0 + k
        jnc    LaddkX
        dec    EBX
        jz     LaddkX            ; Only one digit
LaddkK:
		mov    EAX,[EDI+4]        ; a_i
        add    EAX,1             
        mov    [EDI+4],EAX        ; Move to a
        jnc    LaddkX
        dec    EBX
        lea    EDI,[EDI+4]
        jg     LaddkK            ; More
LaddkX:
		lahf    
        mov    EDI,Zdi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
		;ret
	}
	
	return result;
}

// a = a - b (mod 2^(32*d(a)))
//
// xsubf (&a, &b, d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(b)
// Assumptions: d(a) = d(b) > 0
void xsubf(long* a, long* b, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zbp,EBP
;
        clc                      ; clear carry
LsubfB:
		mov    EBP,[EDX]         ; EBP = b_i
        mov    ECX,[EAX]         ; ECX = a_i
        lea    EDX,[EDX+4]
        sbb    ECX,EBP           ; ECX = a_i - b_i - borrow
        dec    EBX
        mov    [EAX],ECX         ; Move to a
        lea    EAX,[EAX+4]
        jg     LsubfB            ; More digits
;
        mov    EBP,Zbp
        mov    ECX,Zcx
        
		mov    edx,Zdx
		mov    ebx,Zbx
		;ret
	}	
}

// a = a - b (mod 2^(32*d(a)))
// a = a - b (mod 2^(32*d(a))), b sign-extended
//
// f = xsub  (&a, &b, d(a), d(b));
// f = xsubs (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a) >= d(b) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xsub(long* a, long* b, long da, long db)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		mov    Zsg,0
		mov    Zbp,EBP
        mov    Zdi,EDI
;
        mov    EDI,EAX           ; EDI = &a
        sub    EBX,ECX           ; EBX = d(a) - d(b)
        clc                      ; clear carry
LsubB:
		mov    EBP,[EDX]         ; EBP = b_i
        mov    EAX,[EDI]         ; EAX = a_i
        lea    EDX,[EDX+4]
        sbb    EAX,EBP           ; EAX = a_i - b_i - carry
        dec    ECX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LsubB             ; More digits
        lahf                     ; Carry flag to AH
;
        cmp    EBX,0 
        jbe    LsubX             ; d(a) = d(b)
        sar    EBP,31 
        and    EBP,Zsg           ; EBP = b sign bits
        sahf                     ; Restore carry flag
LsubD:
		mov    EAX,[EDI]         ; EAX = a_i
        sbb    EAX,EBP           ; EAX = a_i - b sign bits - carry 
        dec    EBX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LsubD             ; More sign bits
        lahf                     ; Carry flag to AH
LsubX:
		mov    EDI,Zdi
        mov    EBP,Zbp
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
		;ret
	}
	
	return result;
}

// a = a - b (mod 2^(32*d(a)))
// a = a - b (mod 2^(32*d(a))), b sign-extended
//
// f = xsub  (&a, &b, d(a), d(b));
// f = xsubs (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a) >= d(b) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xsubs(long* a, long* b, long da, long db)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		mov    Zsg,0FFFFFFFFh
		mov    Zbp,EBP
        mov    Zdi,EDI
;
        mov    EDI,EAX           ; EDI = &a
        sub    EBX,ECX           ; EBX = d(a) - d(b)
        clc                      ; clear carry
LsubB:
		mov    EBP,[EDX]         ; EBP = b_i
        mov    EAX,[EDI]         ; EAX = a_i
        lea    EDX,[EDX+4]
        sbb    EAX,EBP           ; EAX = a_i - b_i - carry
        dec    ECX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LsubB             ; More digits
        lahf                     ; Carry flag to AH
;
        cmp    EBX,0 
        jbe    LsubX             ; d(a) = d(b)
        sar    EBP,31 
        and    EBP,Zsg           ; EBP = b sign bits
        sahf                     ; Restore carry flag
LsubD:
		mov    EAX,[EDI]         ; EAX = a_i
        sbb    EAX,EBP           ; EAX = a_i - b sign bits - carry 
        dec    EBX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LsubD             ; More sign bits
        lahf                     ; Carry flag to AH
LsubX:
		mov    EDI,Zdi
        mov    EBP,Zbp
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
		;ret
	}
	
	return result;
}

// a = a - k (mod 2^(32*d)), k = 32-bit integer
//
// xsubk (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: k >= 0, d(a) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xsubk(long* a, long k, long da)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zdi,EDI
;
        mov    EDI,EAX           ; EDI = &a
        mov    EAX,[EDI]
        sub    EAX,EDX  
        mov    [EDI],EAX         ; a_0 = a_0 - k
        jnc    LsubkX
        dec    EBX
        jz     LsubkX            ; Only one digit
LsubkK:
		mov    EAX,[EDI+4]        ; a_i
        sub    EAX,1             
        mov    [EDI+4],EAX        ; Move to a
        jnc    LsubkX
        dec    EBX
        lea    EDI,[EDI+4]
        jg     LsubkK            ; More
LsubkX:
		lahf    
        mov    EDI,Zdi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    result,eax
		;ret
	}
	
	return result;
}

// a = 2^(32*d(a)) - a
//
// xneg (&a, d(a));
//
// Registers:   EAX = &a, EDX = d(a)
// Assumptions: d(a) > 0
void xneg(long* a, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    eax,a
		mov    edx,da
		
		mov    Zbx,EBX
        mov    EBX,[EAX]         ; EBX = a_0
        not    EBX
        add    EBX,1             ; EBX = -a_0
        dec    EDX
        mov    [EAX],EBX
        jz     LnegX
LnegB:
		mov    EBX,[EAX+4]        ; EBX = a_i
        not    EBX               ; Flip a_i bits
        adc    EBX,0             ; Add carry
        dec    EDX
        mov    [EAX+4],EBX
        lea    EAX,[EAX+4]
        jg     LnegB
LnegX:
		mov    EBX,Zbx
        
		mov    edx,Zdx
		;ret
	}
	
}

// flags(a - b)
// flags(a - b), shorter of a and b is sign-extended
//
// flags = xcmp  (&a, &b, d(a), d(b));
// flags = xcmps (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a), d(b) > 0
// Returns:     EAX = SignFlag    0x00008000
//                    ZeroFlag    0x00004000
//                    AuxFlag     0x00001000
//                    ParityFlag  0x00000200
//                    CarryFlag   0x00000100
long xcmp(long* a, long* b, long da, long db)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		mov    Zsg,0
;
		mov    Zsi,ESI
        mov    Zdi,EDI
        lea    ESI,[EAX+EBX*4-4] ; ESI = &a_{d(a)-1}
        lea    EDI,[EDX+ECX*4-4] ; EDI = &b_{d(b)-1}
        sub    ECX,EBX
        je     LcmpK             ; same size
        jb     LcmpF             ; b shorter than a
;
; a is shorter than b, EBX = d(a), ECX = d(b) - d(a)
        mov    EAX,[ESI]
        sar    EAX,31
        and    EAX,Zsg           ; EAX = a sign
LcmpD:
		mov    EDX,[EDI]         ; Pick up b_i
        cmp    EAX,EDX           ; Compare a sign with b_i
        lea    EDI,[EDI-4]
        jne    LcmpX
        dec    ECX
        jnz    LcmpD
        jmp    LcmpK
;
; b is shorter than a, EBX = d(a), ECX = -(d(a) - d(b))
LcmpF:
		add    EBX,ECX           ; EBX = d(a)
        mov    EDX,[EDI]
        sar    EDX,31
        and    EDX,Zsg           ; EDX = b sign
LcmpH:
		mov    EAX,[ESI]         ; Pick up a_i
        cmp    EAX,EDX           ; Compare a_i with b sign
        lea    ESI,[ESI-4]
        jne    LcmpX
        inc    ECX
        jnz    LcmpH
LcmpK:
		mov    EAX,[ESI]         ; same size
        mov    EDX,[EDI]
        lea    ESI,[ESI-4]
        cmp    EAX,EDX           ; Compare a_i with b_i
        lea    EDI,[EDI-4]
        jne    LcmpX
        dec    EBX
        jg     LcmpK
        cmp    EAX,EDX           ; Equal
LcmpX:
		lahf   
        mov    EDI,Zdi
        mov    ESI,Zsi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
		;ret
	}
	
	return result;
}

// flags(a - b)
// flags(a - b), shorter of a and b is sign-extended
//
// flags = xcmp  (&a, &b, d(a), d(b));
// flags = xcmps (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a), d(b) > 0
// Returns:     EAX = SignFlag    0x00008000
//                    ZeroFlag    0x00004000
//                    AuxFlag     0x00001000
//                    ParityFlag  0x00000200
//                    CarryFlag   0x00000100
long xcmps(long* a, long* b, long da, long db)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		mov    Zsg,0FFFFFFFFh
;
		mov    Zsi,ESI
        mov    Zdi,EDI
        lea    ESI,[EAX+EBX*4-4] ; ESI = &a_{d(a)-1}
        lea    EDI,[EDX+ECX*4-4] ; EDI = &b_{d(b)-1}
        sub    ECX,EBX
        je     LcmpK             ; same size
        jb     LcmpF             ; b shorter than a
;
; a is shorter than b, EBX = d(a), ECX = d(b) - d(a)
        mov    EAX,[ESI]
        sar    EAX,31
        and    EAX,Zsg           ; EAX = a sign
LcmpD:
		mov    EDX,[EDI]         ; Pick up b_i
        cmp    EAX,EDX           ; Compare a sign with b_i
        lea    EDI,[EDI-4]
        jne    LcmpX
        dec    ECX
        jnz    LcmpD
        jmp    LcmpK
;
; b is shorter than a, EBX = d(a), ECX = -(d(a) - d(b))
LcmpF:
		add    EBX,ECX           ; EBX = d(a)
        mov    EDX,[EDI]
        sar    EDX,31
        and    EDX,Zsg           ; EDX = b sign
LcmpH:
		mov    EAX,[ESI]         ; Pick up a_i
        cmp    EAX,EDX           ; Compare a_i with b sign
        lea    ESI,[ESI-4]
        jne    LcmpX
        inc    ECX
        jnz    LcmpH
LcmpK:
		mov    EAX,[ESI]         ; same size
        mov    EDX,[EDI]
        lea    ESI,[ESI-4]
        cmp    EAX,EDX           ; Compare a_i with b_i
        lea    EDI,[EDI-4]
        jne    LcmpX
        dec    EBX
        jg     LcmpK
        cmp    EAX,EDX           ; Equal
LcmpX:
		lahf   
        mov    EDI,Zdi
        mov    ESI,Zsi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
		;ret
	}
	
	return result;
}

// high-order digit of a
//
// hdigit = xsig (&a, d(a));
//
// Registers:   EAX = &a, EDX = d(a)
// Assumptions: d(a) > 0
// Returns:     EAX = high order digit of a 
//              (a = 0 iff EAX = 0)
long xsig(long* a, long da)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    eax,a
		mov    edx,da
		
		mov    Zbx,EBX
        lea    EBX,[EAX+EDX*4-4] ; EBX = &a_{d-1}
Lsig2:
		mov    EAX,[EBX]
        lea    EBX,[EBX-4]
        cmp    EAX,0
        jne    Lsig8
        dec    EDX
        jg     Lsig2
Lsig8:
		mov    EBX,Zbx
        
		mov    edx,Zdx
		mov    result,eax
		;ret
	}
	
	return result;
}

// Number of significant digits of a
//
// n = xdig (d(a), &a);
//
// Registers:   EAX = d(a), EDX = &a
// Assumptions: d(a) > 0
// Returns:     EAX = number of significant digits of a 
long xdig(long da, long* a)
{
	long result;
	
	__asm
	{		
		mov    Zdx,edx
		mov    eax,da
		mov    edx,a
xdig_:
		cmp    EAX,1
        jbe    Ldig8
        cmp    dword ptr [EDX+EAX*4-4],0
        jne    Ldig8
        dec    EAX
        jmp    xdig_
Ldig8:
		mov    edx,Zdx
		mov    result,eax
		;ret
	}
	
	return result;
}

// Add one to [EAX] and propogate ad infinitum
void xcar(long* a)
{
	__asm
	{
		mov    eax,a
xcar_:
		add    dword ptr[EAX],1
        lea    EAX,[EAX+4]
        jc     xcar_
        ;ret
	}
}

// Subtract one from [EAX] and propogate ad infinitum
void xbor(long* a)
{
	__asm
	{
		mov    eax,a
xbor_:
		sub    dword ptr[EAX],1
        lea    EAX,[EAX+4]
        jc     xbor_
        ;ret
	}
}

// a = a^2
//
// xsqu (&a, d(a));
//
// Registers:   EAX = &a, EDX = d(a)
// Assumptions: d(a) > 0, c(a) >= 2*d(a)
void xsqu(long* a, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    eax,a
		mov    edx,da
		
		mov    Zbx,EBX
        mov    Zcx,ECX
        mov    Zbp,EBP
        mov    Zsi,ESI
        mov    Zdi,EDI
;
        mov    EBP,EAX            ; EBP = &a
        lea    EDI,[EBP+EDX*8-4]  ; EDI = &a + 8*d(a) - 4
Lsqub:
		mov    dword ptr[EDI],0   ; Zero out high order half
        sub    EDI,4              ;      of product area,
        dec    EDX                ;      leaving EDI correclty 
        jg     Lsqub              ;      pointing to a_{d-1}
Lsquc:
		mov    ECX,[EDI]          ; ECX = a_i
        cmp    ECX,0
        je     Lsquu              ; Don't bother if a_i = 0
        mov    EBX,EDI            ; EBX = &a_i 
        sub    EBX,EBP            ; EBX = &a_i - &a
        mov    dword ptr[EDI],0   ; Zap a_i in a
        mov    EAX,ECX 
        mul    ECX                ; EAX:EDX = a_i^2
        add    [EDI+EBX+0],EAX
        adc    [EDI+EBX+4],EDX
        adc    dword ptr [EDI+EBX+8],0
        jc     Lsque
Lsquf:
		sub    EBX,4              ; EBX = &a_{i-1} - &a
        jb     Lsquz              ; i = 0, all done
Lsqui:
		mov    EAX,[EBP+EBX]      ; EAX = a_j
        mul    ECX                ; EAX:EDX = a_i*a_j
        add    EAX,EAX
        adc    EDX,EDX
        adc    dword ptr [EDI+EBX+8],0
        jc     Lsqum
Lsqun:
		add    [EDI+EBX+0],EAX
        adc    [EDI+EBX+4],EDX
        adc    dword ptr [EDI+EBX+8],0
        jc     Lsqup
Lsquq:
		sub    EBX,4              ; Next a_j
        jnb    Lsqui   
Lsquu:
		sub    EDI,4              ; Next a_i
        cmp    EDI,EBP
        jnb    Lsquc
Lsquz:
		mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
        mov    ECX,Zcx
        mov    EBX,Zbx
        ;ret
		jmp    Lsqur
Lsque:
		lea    EAX,[EDI+EBX+12]
        ;call   xcar
xcar1_:
		add    dword ptr[EAX],1
		lea    EAX,[EAX+4]
		jc     xcar1_
        jmp    Lsquf
Lsqum:
		mov    Zsg,EAX
        lea    EAX,[EDI+EBX+12]
        ;call   xcar
xcar2_:
		add    dword ptr[EAX], 1
		lea    EAX,[EAX+4]
		jc     xcar2_
        mov    EAX,Zsg
        jmp    Lsqun
Lsqup:
		lea    EAX,[EDI+EBX+12]
        ;call   xcar
xcar3_:
		add    dword ptr[EAX], 1
		lea    EAX, [EAX + 4]
		jc     xcar3_
        jmp    Lsquq
Lsqur:
		mov    edx,Zdx
	}
}

// a = a * b
//
// xmul (&a, &b, d(a), d(b));
// 
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a), d(b) > 0, c(a) >= d(a) + d(b)
//
//                      EDI ->
//                       |
//   |xxx|xxx|xxx|xxx|a_4|a_3|a_2|a_1|a_0|
//
//   *                   |b_3|b_2|b_1|b_0|
//                                       |
//                                   <- ESI
//                                         
//   |ppp|ppp|ppp|ppp|ppp|                  = a_4 * b
//   |ccc|ppp|ppp|ppp|ppp|ppp|              + a_3 * b
//   |ccc|ccc|ppp|ppp|ppp|ppp|ppp|          + a_2 * b
//   |ccc|ccc|ccc|ppp|ppp|ppp|ppp|ppp|      + a_1 * b
//   |ccc|ccc|ccc|ccc|ppp|ppp|ppp|ppp|ppp|  + a_0 * b
void xmul(long* a, long* b, long da, long db)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		mov    Zbp,EBP
        mov    Zsi,ESI
        mov    Zdi,EDI
;
        mov    Zmulda,EBX         ; d(a)
        mov    Zmulb0,EDX         ; &b
        mov    Zmuldb,ECX         ; d(b)
;
; Compute a_{d(a)-1} * b and move into a
        lea    EDI,[EAX+EBX*4-4]  ; EDI = &a{d(a)-1}
        mov    ESI,EDX            ; ESI = &b
        mov    EBX,[EDI]          ; EBX = a_{d(a)-1}
        xor    EBP,EBP            ; EBP = carry
Lmuld:
		mov    EAX,[ESI]          ; ESI = b_j
        lea    ESI,[ESI+4]
        mul    EBX                ; EDX:EAX = a_{d(a)-1}*b_j
        add    EAX,EBP            ; Add carry
        mov    EBP,EDX            ; Save new carry
        mov    [EDI],EAX          ; Move product into a
        adc    EBP,0
        lea    EDI,[EDI+4]
        dec    ECX
        jg     Lmuld
        mov    [EDI],EBP          ; Final carry
        dec    Zmulda
        jz     Lmulx
;
; Compute a_i * b and accumulate into a
Lmulh:
		mov    ECX,Zmuldb         ; ECX = d(b)
        xor    EBP,EBP            ; EBP = carry
        lea    EAX,[ECX*4+4]       ; EAX = 4*d(b) + 4
        sub    EDI,EAX            ; EDI = &a_i
        mov    ESI,Zmulb0         ; ESI = &b
        mov    EBX,[EDI]          ; EBX = a_i
        mov    dword ptr [EDI],0  ; New a_i = 0
;
; Compute a_i * b_j and add to a_{i+j}
Lmulk:
		mov    EAX,[ESI]          ; EAX = b_j
        lea    ESI,[ESI+4]
        mul    EBX                ; EDX:EAX = a_i * b_j
        add    EAX,EBP            ; Add carry
        mov    EBP,EDX            ; Save new carry
        mov    EDX,[EDI]          ; EDX = a_{i+j}
        adc    EBP,0
        add    EAX,EDX            ; EAX = a_i*b_j + a_{i+j}
        adc    EBP,0
        mov    [EDI],EAX          ; New a_{i+j}
        lea    EDI,[EDI+4]
        dec    ECX
        jg     Lmulk
        add    [EDI],EBP          ; Add final carry
        adc    dword ptr [EDI+4],0
        jc     Lmulp
;
; Next a_i
Lmulq:
		dec    Zmulda
        jg     Lmulh
Lmulx:
		mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
        ;ret
		jmp    Lmulr
Lmulp:
		lea    EAX,[EDI+8]
        call   xcar
        jmp    Lmulq
Lmulr:
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
	}
}

// a = a * k,  (k = 32-bit integers)
//
// xmulk (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: d(a) > 0, c(a) > d(a)
void xmulk(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zbp,EBP
        mov    Zsi,ESI
;
        mov    ESI,EAX            ; ESI = &a
        mov    ECX,EDX            ; ECX = k
        xor    EBP,EBP
Lmulk2:
		mov    EAX,[ESI]          ; EAX = a_i
        mul    ECX                ; Multiply by k
        add    EAX,EBP            ; Add carry
        adc    EDX,0  
        mov    EBP,EDX            ; Save carry
        mov    [ESI],EAX
        lea    ESI,[ESI+4]
        dec    EBX
        jg     Lmulk2
        mov    [ESI],EDX          ; Final high order digit
;
        mov    ESI,Zsi
		
        mov    EBP,Zbp
		mov    edx,Zdx
		mov    ebx,Zbx
        ;ret
	}
}

// a = a * 2^k       (k = 32-bit integer)
//
// xmul2k (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: d(a) > 0, c(a) >= d(a) + [(k+31)/32]
//
// Example: d=2, k=136
//
//        ECX = (k mod 32) = 8
//        |                                               EAX
//        ||<---------EDX = [k/32]-------->|<---EBX = d--->|
//        ||       |       |       |       |       |       |
// |0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|a a a a|a a a a|
//        a|a a a a|a a a 0| 
void xmul2k(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    ECX,EDX
        shr    EDX,5              ; EDX = [k/32]
        and    ECX,31             ; ECX = k mod 32
        jz     Lmul2k8            ; k = 0 (mod 32): use xmul2d_
;
        mov    Zsi,ESI
        mov    Zdi,EDI
        lea    EAX,[EAX+EBX*4]    ; EAX = &a_d
        mov    ESI,0              ; a_d = 0
Lmul2k2:
		mov    EDI,[EAX-4]         ; EDI = a_i
        shld   ESI,EDI,CL         ; multiply by 2^(k mod 32)
        mov    [EAX+EDX*4],ESI    ; Store shifted a_{i+1}
        dec    EBX
        mov    ESI,EDI            ; Next a_{i+1}
        lea    EAX,[EAX-4]
        jg     Lmul2k2
;
        shl    ESI,CL             ; ESI = a_0
        mov    [EAX+EDX*4],ESI    ; Store shifted a_0
;
        mov    EDI,Zdi
        mov    ESI,Zsi
        mov    ECX,Zcx
        ;jmp    xmovz_             ; ZAP EDX words from [EAX]
		cmp    EDX,0
        jle    Lmovz8
Lmovz2: mov    dword ptr [EAX],0 
        dec    EDX
        lea    EAX,[EAX+4]
        jg     Lmovz2
Lmovz8:
		;ret
Lmul2k8:
		mov    ECX,Zcx            ; Restore ECX
        ;jmp    xmul2d_            ; Drop into xmul2d_
		cmp    EDX,0
        jle    Lmul2d8            ; d = 0: nothing to do
        mov    Zcx,ECX
        lea    EAX,[EAX+EBX*4]    ; EAX = &a_d
Lmul2d4:
		mov    ECX,[EAX-4]        ; EAX = &a{i+1}, ECX = a_i
        mov    [EAX+4*EDX-4],ECX  ; Multiply by 2^(32*d)
        dec    EBX
        lea    EAX,[EAX-4]
        jg     Lmul2d4
;
        mov    ECX,Zcx
        ;jmp    xmovz_             ; ZAP EDX words from [EAX]
		cmp    EDX,0
        jle    Lmovz8a
Lmovz2a:
		mov    dword ptr [EAX],0 
        dec    EDX
        lea    EAX,[EAX+4]
        jg     Lmovz2a
Lmovz8a:
		;ret
Lmul2d8:
		;ret
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}

// a = a * 2^(32*d)  (d = 32-bit integer)
//
// xmul2d (&a, d, d(a));
//
// Registers:   EAX = &a, EDX = d, EBX = d(a)
// Assumptions: d(a) > 0, c(a) >= d(a) + d
void xmul2d(long* a, long d, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,d
		mov    ebx,da
		
		cmp    EDX,0
        jle    Lmul2d8            ; d = 0: nothing to do
        mov    Zcx,ECX
        lea    EAX,[EAX+EBX*4]    ; EAX = &a_d
Lmul2d4:
		mov    ECX,[EAX-4]        ; EAX = &a{i+1}, ECX = a_i
        mov    [EAX+4*EDX-4],ECX  ; Multiply by 2^(32*d)
        dec    EBX
        lea    EAX,[EAX-4]
        jg     Lmul2d4
;
        mov    ECX,Zcx
        ;jmp    xmovz_             ; ZAP EDX words from [EAX]
		cmp    EDX,0
        jle    Lmovz8a
Lmovz2a:
		mov    dword ptr [EAX],0 
        dec    EDX
        lea    EAX,[EAX+4]
        jg     Lmovz2a
Lmovz8a:
		;ret
Lmul2d8:
		;ret
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}

// a = (a mod b), q = [a / b]
//
// xdiv (&a, &b, d(a), d(b), &q);
// xmod (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
//              4[ESP] = &q (xdiv only)
// Assumptions: b has no leading zero digits
//              d(a) >= d(b) >= 2,
//              c(a) >= d(a) + 1,
//              c(q) >= d(a) - d(b) + 1
//              Quotient [a/b] has d(a) - d(b) + 1 digits
//
// Example: d(a) = 6, d(b) = 4, d(q) = 3
//
// extra zero digit                    EDI = &a_i
//         |                               | 
// |0 0 0 0|0 a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
//     :       :       :       :           |
//     |  u_0  |  u_1  |  u_2  |           |
//                                         |
//                                         |
//         |0 0 b b|b b b b|b b b b|b b b b|
//             :       :       :
//             |  v_1  |  v_2  |  
//
// Initial choice for q_i is min(2^32-1, [(u_0*2^32 + u_1)/v_1])
void xdiv(long* a, long* b, long da, long db, long* q)
{
	__asm
	{		
		mov    eax,q
		mov    Zbp,ebp
		mov    Zdivq0,eax
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		;mov    Zbp,EBP
        ;mov    EBP,[ESP+4]
        ;mov    Zdivq0,EBP             ; Save &q for xdiv_

		mov    Zsi,ESI
        mov    Zdi,EDI
;
; Initialise pointers
        mov    Zdiva0,EAX             ; Save &a
        mov    Zdivb0,EDX             ; Save &b
        mov    Zdivda,EBX             ; Save d(a)
        mov    Zdivdb,ECX             ; Save d(b)
        mov    dword ptr[EAX+EBX*4],0 ; Extra zero for a
        sub    EBX,ECX                ; EBX = i = d(a) - d(b)
        lea    EDI,[EAX+EBX*4]        ; EDI = &a_i
        mov    Zdivii,EBX             ; Save i
        mov    Zdivai,EDI             ; Save &a_i
;
; Determine number of high order bits in b, and hence v_1 and v_2
        mov    EBP,[EDX+ECX*4-4]    ; High order b
        mov    ESI,[EDX+ECX*4-8]    ; Next b
        mov    EDI,0
        cmp    ECX,2
        jbe    Ldivc
        mov    EDI,[EDX+ECX*4-12]   ; Next b if d(b) > 2
Ldivc:
		mov    ECX,0                ; ECX = b_z
Ldivd:
		test   EBP,80000000h        ; Test high order bit
        jnz    Ldive                ; EBP = v_1
        inc    ECX                  ; b_z++
        shld   EBP,ESI,1            ; Shift out zero bit
        shld   ESI,EDI,1
        shl    EDI,1    
        jmp    Ldivd
Ldive:
		mov    Zdivbz,ECX           ; b_z = number of high order bits
        mov    Zdivv1,EBP           ; v_1 = high order 32 bits of b
        mov    Zdivv2,ESI           ; v_2 = next 32 bits of b
;          
; for (i = d(a) - d(b); i >= 0; i--)
;
; Calculation of q_i is based on Algorithm D in 
; 4.3.1 of D. Knuth's "Semi-numerical Algorithms"
Ldivf:
		mov    EDI,Zdivai           ; EDI = &a_i
        mov    ECX,Zdivdb           ; ECX = d(b)
        lea    ESI,[EDI+ECX*4-12]   ; ESI = &a_{i+d(b)-3}
        mov    EDX,[ESI+12]          ; EDX = a_{i+d(b)}
        mov    EAX,[ESI+8]           ; EAX = a_{i+d(b)-1}
        mov    EBX,0
        cmp    ESI,Zdiva0
        jb     Ldivg                ; i+d(b) < 3
        mov    EBX,[ESI]            ; EBX = a_{i+d(b)-3}
Ldivg:
		mov    ECX,Zdivbz           ; ECX = b_z
        mov    EBP,[ESI+4]           ; EBP = a_{i+d(b)-2}
        shld   EDX,EAX,CL           ; EDX = u_0
        shld   EAX,EBP,CL           ; EAX = u_1
        shld   EBP,EBX,CL           ; EBP = u_2
;
; q_i = 2^32 - 1 if u_0 = v_1
        mov    EBX,Zdivv1           ; EBX = v_1
        mov    Zdivqi,0FFFFFFFFh    ; Set q_i = 2^32-1
        cmp    EDX,EBX              ; u_0 = v_1 ?
        jae    Ldivm                ; q_i = 2^32 - 1
;
; q_i = [(u_0*2^32 + u_1) / v_1]
        mov    ECX,Zdivv2           ; ECX = v_2
        div    EBX                  ; EAX = q_i
        mov    Zdivqi,EAX           ; Save q_i
        mov    ESI,EDX              ; ESI = u_0*2^32 + u_1 - q_i*v_1
;
; If v_2*q_i > (u_0*2^32 + u_1 - q_i*v_1)*2^32 + u_2 then q_i--
        mul    ECX                  ; EDX:EAX = v_2*q_i
;                                   ; ESI     = u_0*2^32 + u_1 - q_i*v_1
;                                   ; EBX     = v_1
;                                   ; ECX     = v_2
;                                   ; EBP     = u_2
Ldivk:
		cmp    EDX,ESI              ; Compare high order
        jb     Ldivm                ; OK
        ja     Ldivl
        cmp    EAX,EBP              ; Compare low order
        jbe    Ldivm                ; OK
Ldivl:
		dec    Zdivqi               ; q_i--
        add    ESI,EBX              ; u_0*2^32 + u_1 - (q_i - 1)*v_1
        jc     Ldivm                ; OK if carry
        sub    EAX,ECX              ; v_2*(q_i - 1)
        sbb    EDX,0 
        jmp    Ldivk                ; Repeat test
;
; Subtract q_i*b from a_i
Ldivm:
		mov    ESI,Zdivb0           ; ESI = &b
        mov    EBX,Zdivdb           ; EBX = d(b)
;       inc    Zdivqi               ; For testing "negative result" code
        mov    ECX,Zdivqi           ; ECX = q_i
        xor    EBP,EBP              ; Zero high order digit
Ldivp:
		mov    EAX,[ESI]            ; EAX = b_j
        mul    ECX                  ; Multiply by q_i
        add    EAX,EBP              ; Add last high order digit
        adc    EDX,0                
        mov    EBP,EDX              ; Save high order digit
        mov    EDX,[EDI]            ; Subtract from a_{i+j}
        lea    ESI,[ESI+4]
        sub    EDX,EAX              ; 
        mov    [EDI],EDX
        adc    EBP,0                ; Add borrow from subtract
        lea    EDI,[EDI+4]
        dec    EBX
        jg     Ldivp
        sub    dword ptr [EDI+0],EBP ; Subtract final digit
        jnc    Ldivt                ; Non-negative result
;
; Negative result: we must add b to a_i
        mov    EDI,Zdivai           ; EDI = &a_i
        mov    ESI,Zdivb0           ; ESI = &b
        mov    EBX,Zdivdb           ; EBX = d(b)
        clc                         ; clear carry
Ldivr:
		mov    EAX,[ESI]            ; b_j
        mov    EDX,[EDI]            ; a_{i+j}
        lea    ESI,[ESI+4]
        adc    EDX,EAX              ; a_{i+j} + b_j + carry 
        dec    EBX
        mov    [EDI],EDX            ; Move to a
        lea    EDI,[EDI+4]
        jg     Ldivr                ; More digits
        adc    dword ptr [EDI],0    ; Final carry
        dec    Zdivqi
;
; Store quotient digit for xdiv
Ldivt:
		mov    ESI,Zdivq0           ; ESI = &q_0
        cmp    ESI,1
        je     Ldivv                ; No quotient
        mov    EBX,Zdivii           ; EBX = i
        lea    ESI,[ESI+EBX*4]      ; ESI = &q_i
        mov    EAX,Zdivqi
        mov    [ESI],EAX            ; Store quotient
;
; Next place (xdiv)
        sub    Zdivai,4             ; &a_{i-1}
        dec    Zdivii               ; i--
        jge    Ldivf                ; More places
;
        mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
		
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
        ;ret    4                    ; One item on stack
		jmp    Ldivz
;
; Next place (xmod)
Ldivv:
		sub    Zdivai,4             ; &a_{i-1}
        dec    Zdivii               ; i--
        jge    Ldivf                ; More places
;
        mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
Ldivz:        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		;retn                        ; Nothing on stack
	}
}

// a = [a / k]  (k = 32-bit integer)
//
// r = xdivk (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: k > 0, d(a) > 0
// Returns:     (a mod k) in EAX
long xdivk(long* a, long k, long da)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zdi,EDI
;
        lea    EDI,[EAX+EBX*4-4]  ; EDI = &a_{d-1}
        mov    ECX,EDX            ; ECX = k
        xor    EDX,EDX            ; EDX = 0
Ldivk2:
		mov    EAX,[EDI]          ; EAX = a_i
        div    ECX                ; EAX = [(2^32*EDX + a_i)/k]
        mov    [EDI],EAX          ; EDX = (2^32*EDX + a_i) mod k]
        sub    EDI,4
        dec    EBX
        jg     Ldivk2
        mov    EAX,EDX            ; Remainder
;
        mov    ECX,Zcx
        mov    EDI,Zdi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    result,eax
		;ret
	}
	
	return result;
}

// a = [a/2^k]  (k = 32-bit integer)
//
// xdiv2k (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: 0 <= k < 32*d(a), d(a) > 0
//
// Example: d=6, k=104  
//                       ECX = (k mod 32) = 8
//                        |
// |a a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
// |       |       |       |       |       |       |
// |<---EBX = d - [k/32]-->|<-----EDX = [k/32]---->|
//                                                EAX
void xdiv2k(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zsi,ESI
        mov    ECX,EDX
        mov    Zdi,EDI
        and    ECX,31             ; ECX = k mod 32
        shr    EDX,5              ; EDX = [k/32]
        sub    EBX,EDX            ; EBX = d - [k/32]
        dec    EBX
        jz     Ldiv2k6            ; Only 1 digit
Ldiv2k4:
		mov    ESI,[EAX+EDX*4+0]   ; ESI = a_i
        mov    EDI,[EAX+EDX*4+4]   ; EDI = a_{i+1}
        shrd   ESI,EDI,CL         ; Divide by 2^(k mod 32)
        mov    [EAX],ESI          ; Store quotient in a
        dec    EBX
        lea    EAX,[EAX+4]
        jg     Ldiv2k4
Ldiv2k6:
		mov    ESI,[EAX+EDX*4+0]   ; ESI = last a_i
        mov    EDI,Zdi
        shr    ESI,CL             ; Divide by 2^(k mod 32)
        mov    ECX,Zcx
        mov    [EAX],ESI          ; Store last quotient in a
        mov    ESI,Zsi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		;ret
	}
}

// a = a * 2^(32*d)  (d = 32-bit integer)
//
// xdiv2d (&a, d, d(a));
//
// Registers:   EAX = &a, EDX = d, EBX = d(a)
// Assumptions: 0 <= d < d(a), d(a) > 0
void xdiv2d(long* a, long d, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,d
		mov    ebx,da
		
		cmp    EDX,0
        jle    Ldiv2d8            ; d = 0: nothing to do
        mov    Zcx,ECX
        sub    EBX,EDX            ; EBX = d(a) - d
Ldiv2d4:
		mov    ECX,[EAX+EDX*4]    ; EAX = &a, ECX = a_i
        mov    [EAX],ECX          ; Divide by 2^(32*d)
        dec    EBX
        lea    EAX,[EAX+4]
        jg     Ldiv2d4
;
        mov    ECX,Zcx
Ldiv2d8:
		
		mov    edx,Zdx
		mov    ebx,Zbx
		;ret
	}
}

// a = (a mod b), q = [a / b]
//
// xdiv (&a, &b, d(a), d(b), &q);
// xmod (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
//              4[ESP] = &q (xdiv only)
// Assumptions: b has no leading zero digits
//              d(a) >= d(b) >= 2,
//              c(a) >= d(a) + 1,
//              c(q) >= d(a) - d(b) + 1
//              Quotient [a/b] has d(a) - d(b) + 1 digits
//
// Example: d(a) = 6, d(b) = 4, d(q) = 3
//
// extra zero digit                    EDI = &a_i
//         |                               | 
// |0 0 0 0|0 a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
//     :       :       :       :           |
//     |  u_0  |  u_1  |  u_2  |           |
//                                         |
//                                         |
//         |0 0 b b|b b b b|b b b b|b b b b|
//             :       :       :
//             |  v_1  |  v_2  |  
//
// Initial choice for q_i is min(2^32-1, [(u_0*2^32 + u_1)/v_1])
void xmod(long* a, long* b, long da, long db)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db
		
		mov    Zbp,EBP
        mov    Zdivq0,1               ; Set &q = 1 for xmod_
		
		mov    Zsi,ESI
        mov    Zdi,EDI
;
; Initialise pointers
        mov    Zdiva0,EAX             ; Save &a
        mov    Zdivb0,EDX             ; Save &b
        mov    Zdivda,EBX             ; Save d(a)
        mov    Zdivdb,ECX             ; Save d(b)
        mov    dword ptr[EAX+EBX*4],0 ; Extra zero for a
        sub    EBX,ECX                ; EBX = i = d(a) - d(b)
        lea    EDI,[EAX+EBX*4]        ; EDI = &a_i
        mov    Zdivii,EBX             ; Save i
        mov    Zdivai,EDI             ; Save &a_i
;
; Determine number of high order bits in b, and hence v_1 and v_2
        mov    EBP,[EDX+ECX*4-4]    ; High order b
        mov    ESI,[EDX+ECX*4-8]    ; Next b
        mov    EDI,0
        cmp    ECX,2
        jbe    Ldivc
        mov    EDI,[EDX+ECX*4-12]   ; Next b if d(b) > 2
Ldivc:
		mov    ECX,0                ; ECX = b_z
Ldivd:
		test   EBP,80000000h        ; Test high order bit
        jnz    Ldive                ; EBP = v_1
        inc    ECX                  ; b_z++
        shld   EBP,ESI,1            ; Shift out zero bit
        shld   ESI,EDI,1
        shl    EDI,1    
        jmp    Ldivd
Ldive:
		mov    Zdivbz,ECX           ; b_z = number of high order bits
        mov    Zdivv1,EBP           ; v_1 = high order 32 bits of b
        mov    Zdivv2,ESI           ; v_2 = next 32 bits of b
;          
; for (i = d(a) - d(b); i >= 0; i--)
;
; Calculation of q_i is based on Algorithm D in 
; 4.3.1 of D. Knuth's "Semi-numerical Algorithms"
Ldivf:
		mov    EDI,Zdivai           ; EDI = &a_i
        mov    ECX,Zdivdb           ; ECX = d(b)
        lea    ESI,[EDI+ECX*4-12]   ; ESI = &a_{i+d(b)-3}
        mov    EDX,[ESI+12]          ; EDX = a_{i+d(b)}
        mov    EAX,[ESI+8]           ; EAX = a_{i+d(b)-1}
        mov    EBX,0
        cmp    ESI,Zdiva0
        jb     Ldivg                ; i+d(b) < 3
        mov    EBX,[ESI]            ; EBX = a_{i+d(b)-3}
Ldivg:
		mov    ECX,Zdivbz           ; ECX = b_z
        mov    EBP,[ESI+4]           ; EBP = a_{i+d(b)-2}
        shld   EDX,EAX,CL           ; EDX = u_0
        shld   EAX,EBP,CL           ; EAX = u_1
        shld   EBP,EBX,CL           ; EBP = u_2
;
; q_i = 2^32 - 1 if u_0 = v_1
        mov    EBX,Zdivv1           ; EBX = v_1
        mov    Zdivqi,0FFFFFFFFh    ; Set q_i = 2^32-1
        cmp    EDX,EBX              ; u_0 = v_1 ?
        jae    Ldivm                ; q_i = 2^32 - 1
;
; q_i = [(u_0*2^32 + u_1) / v_1]
        mov    ECX,Zdivv2           ; ECX = v_2
        div    EBX                  ; EAX = q_i
        mov    Zdivqi,EAX           ; Save q_i
        mov    ESI,EDX              ; ESI = u_0*2^32 + u_1 - q_i*v_1
;
; If v_2*q_i > (u_0*2^32 + u_1 - q_i*v_1)*2^32 + u_2 then q_i--
        mul    ECX                  ; EDX:EAX = v_2*q_i
;                                   ; ESI     = u_0*2^32 + u_1 - q_i*v_1
;                                   ; EBX     = v_1
;                                   ; ECX     = v_2
;                                   ; EBP     = u_2
Ldivk:
		cmp    EDX,ESI              ; Compare high order
        jb     Ldivm                ; OK
        ja     Ldivl
        cmp    EAX,EBP              ; Compare low order
        jbe    Ldivm                ; OK
Ldivl:
		dec    Zdivqi               ; q_i--
        add    ESI,EBX              ; u_0*2^32 + u_1 - (q_i - 1)*v_1
        jc     Ldivm                ; OK if carry
        sub    EAX,ECX              ; v_2*(q_i - 1)
        sbb    EDX,0 
        jmp    Ldivk                ; Repeat test
;
; Subtract q_i*b from a_i
Ldivm:
		mov    ESI,Zdivb0           ; ESI = &b
        mov    EBX,Zdivdb           ; EBX = d(b)
;       inc    Zdivqi               ; For testing "negative result" code
        mov    ECX,Zdivqi           ; ECX = q_i
        xor    EBP,EBP              ; Zero high order digit
Ldivp:
		mov    EAX,[ESI]            ; EAX = b_j
        mul    ECX                  ; Multiply by q_i
        add    EAX,EBP              ; Add last high order digit
        adc    EDX,0                
        mov    EBP,EDX              ; Save high order digit
        mov    EDX,[EDI]            ; Subtract from a_{i+j}
        lea    ESI,[ESI+4]
        sub    EDX,EAX              ; 
        mov    [EDI],EDX
        adc    EBP,0                ; Add borrow from subtract
        lea    EDI,[EDI+4]
        dec    EBX
        jg     Ldivp
        sub    dword ptr [EDI+0],EBP ; Subtract final digit
        jnc    Ldivt                ; Non-negative result
;
; Negative result: we must add b to a_i
        mov    EDI,Zdivai           ; EDI = &a_i
        mov    ESI,Zdivb0           ; ESI = &b
        mov    EBX,Zdivdb           ; EBX = d(b)
        clc                         ; clear carry
Ldivr:
		mov    EAX,[ESI]            ; b_j
        mov    EDX,[EDI]            ; a_{i+j}
        lea    ESI,[ESI+4]
        adc    EDX,EAX              ; a_{i+j} + b_j + carry 
        dec    EBX
        mov    [EDI],EDX            ; Move to a
        lea    EDI,[EDI+4]
        jg     Ldivr                ; More digits
        adc    dword ptr [EDI],0    ; Final carry
        dec    Zdivqi
;
; Store quotient digit for xdiv
Ldivt:
		mov    ESI,Zdivq0           ; ESI = &q_0
        cmp    ESI,1
        je     Ldivv                ; No quotient
        mov    EBX,Zdivii           ; EBX = i
        lea    ESI,[ESI+EBX*4]      ; ESI = &q_i
        mov    EAX,Zdivqi
        mov    [ESI],EAX            ; Store quotient
;
; Next place (xdiv)
        sub    Zdivai,4             ; &a_{i-1}
        dec    Zdivii               ; i--
        jge    Ldivf                ; More places
;
        mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
		
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
        ;ret    4                    ; One item on stack
		jmp    Ldivz
;
; Next place (xmod)
Ldivv:
		sub    Zdivai,4             ; &a_{i-1}
        dec    Zdivii               ; i--
        jge    Ldivf                ; More places
;
        mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
Ldivz:        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		;retn                        ; Nothing on stack
	}
}

// a = (a mod k)  (k = 32-bit integer)
//
// xmod (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: k > 0, d(a) > 0
void xmodk(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zdi,EDI
;
        lea    EDI,[EAX+EBX*4-4]  ; EDI = &a_{d-1}
        mov    ECX,EDX            ; ECX = k
        xor    EDX,EDX            ; EDX = 0
Lmodk2:
		mov    EAX,[EDI]          ; EAX = a_i
        div    ECX                ; EAX = [(2^32*EDX + a_i)/k]
        sub    EDI,4
        dec    EBX
        jg     Lmodk2
        mov    [EDI+4],EDX         ; Remainder
;
        mov    ECX,Zcx
        mov    EDI,Zdi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		;ret
	}
}

// a = a mod 2^k,  q = [a/2^k]  (k = 32-bit integer)
//
// xsep2k (&a, &q, k, d(a));
//
// Registers:   EAX = &a, EDX = &q, EBX = k, ECX = d(a)
// Assumptions: 0 <= k < 32*d, d(a) > 0, c(q) >= d(a) - [k/32]
//
// Example: d=6, k=104
//                       (k mod 32) = 8
//                        |
// |h h h h|h h h h|h h h l|l l l l|l l l l|l l l l|
// |       |       |       |       |       |       a
// |<----- d - [k/32] ---->|<------ [k/32] ------->|
//                        
//                 |0 0 0 l|l l l l|l l l l|l l l l|
//                 |       |       |       |       a
//
//                         |0 h h h|h h h h|h h h h|
//                         |       |       |       q
void xsep2k(long* a, long* q, long k, long d)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,q
		mov    ebx,k
		mov    ecx,d
		
		mov    Zbp,EBP
        mov    Zsi,ESI
        mov    Zdi,EDI
;
        mov    EDI,EDX            ; EDI = &q
        mov    EBP,ECX            ; EBP = d
        mov    ECX,EBX
        and    ECX,31             ; ECX = k mod 32
        shr    EBX,5              ; EBX = [k/32]
        lea    ESI,[EAX+EBX*4]    ; ESI = &a + 4*[k/32]
        sub    EBP,EBX            ; EBP = d - [k/32]
;
        mov    EDX,1
        mov    EAX,[ESI]          ; EAX = a_[32/k]
        shl    EDX,CL 
        dec    EDX                ; EDX = 2^(k mod 32) - 1
        and    EDX,EAX            ; EDX = a_[32/k] & 2^(k mod 32) - 1
        mov    [ESI],EDX          ; Put back
        dec    EBP
        jz     Lsep8
        mov    EDX,[ESI+4]         ; EDX = a_[32/k + 1]
        shrd   EAX,EDX,CL         ; Divide by 2^(k mod 32)
        mov    [EDI],EAX          ; Store quotient in q
        dec    EBP
        lea    ESI,[ESI+4]
        lea    EDI,[EDI+4]
        jz     Lsep6
Lsep4:
		mov    EAX,[ESI+0]         ; EAX = next a_i
        mov    EDX,[ESI+4]         ; EDX = a_{i+1}
        shrd   EAX,EDX,CL         ; Divide by 2^(k mod 32)
        mov    dword ptr [ESI],0  ; Zap a_i
        mov    [EDI],EAX          ; Store quotient in q
        dec    EBP
        lea    ESI,[ESI+4]
        lea    EDI,[EDI+4]
        jg     Lsep4
Lsep6:
		mov    EAX,[ESI]          ; EAX = last a_i
        mov    dword ptr [ESI],0  ; Zap a_i
Lsep8:
		shr    EAX,CL             ; Divide by 2^(k mod 32)
        mov    [EDI],EAX          ; Store last quotient in q
;
        mov    EBP,Zbp
        mov    EDI,Zdi
        mov    ESI,Zsi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		;ret
	}
}

// nxtprm = xnxtprmk(k);
//
// Registers:   EAX = k
// Assumptions: k >= 0
// Returns:     EAX = next prime after k if k < 2^32
//              EAX = 1 otherwise
long xnxtprmk(long k)
{
	long result;
	
	__asm
	{
		mov    eax,k
		
		cmp    EAX,2
        jb     Lnxtp2
        je     Lnxtp3
        mov    Zdx,EDX
        mov    Zbx,EBX
        mov    Zcx,ECX
        mov    EBX,EAX
        dec    EBX
        or     EBX,1
Lnxtpa:
		add    EBX,2      ; next k
        jc     Lnxtpx     ; Return 1 if overlflow
        mov    ECX,1      ; ECX = c
Lnxtpb:
		add    ECX,2      ; Next c
        mov    EAX,EBX
        xor    EDX,EDX
        div    ECX        ; EAX = [k/c], EDX = k mod c
        or     EDX,EDX
        jz     Lnxtpa     ; No remainder
        cmp    ECX,EAX
        jb     Lnxtpb     ; c < quotient
Lnxtpx:
		mov    EAX,EBX    ; Return k
        mov    ECX,Zcx
        mov    EBX,Zbx
        mov    EDX,Zdx
        ;ret
		jmp    Lnxtp4
Lnxtp2:
		mov    EAX,2
        ;ret
		jmp    Lnxtp4
Lnxtp3:
		mov    EAX,3
        ;ret
Lnxtp4:
		mov    result,eax
	}
	
	return result;
}

/* Unformatted print of an array */
void xdump(long* a, long d)
{
	static long i;
	printf("%d ", d);
	for (i = 0; i < d; i++) { printf("%d ", (unsigned long)a[i]); }
	printf("\n");
}

/* Unformatted print of an array */
void xdumps(short* a, long d)
{
	static long i;
	printf("%d ", d);
	for (i = 0; i < d; i++) { printf("%d ", (unsigned short)a[i]); }
	printf("\n");
}

/* Unformatted print of an array */
void xdumpd(double* a, long d)
{
	static long i;
	printf("%d ", d);
	for (i = 0; i < d; i++) { printf("%e ", a[i]); }
	printf("\n");
}
