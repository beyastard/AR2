#include <stdio.h>
#include <conio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "fastmath.h"
#include "regmem.h"
#include "intmath.h"
#include "eval.h"
#include "wmod.h"

#include "vfypr32.h"

/* Test keyboard "Q" pressed */
long test_quit(void)

{
	if (!_kbhit())
		return 0;

	if (_getch() != 'Q')
		return 0;

	quit = 1;

	return 1;
}

/* Temporarily or permanently halt the program */
void halt(void)
{
	while (_kbhit())
		_getch();

	fclose(dat_file);
	dat_file = NULL;

	fclose(jnl_file);
	jnl_file = NULL;

	printf("VFYPR stopped: Press X to terminate, anything else to continue. \n");

	if (_getch() == 'X')
	{
		printf("VFYPR terminated \n");
		exit(0);
	}

	printf("VFYPR restarted \n");
	quit = 0;
}

/* Disaster */
void gen_fail(void)
{
	printf("The program is not functioning correctly for reasons stated above;\n");
	printf("Please email details to <bryankreinhart@gmail.com>.\n");
	halt();
}

/* General file failure */
void file_fail(char* f, char* t)
{
	printf("Unable to %s file vfypr.%s.\n", t, f);
	printf("Stopped: Press ENTER to try again.\n");
	halt();
}

/* Return index of first b in a. Otherwise return -1. */
long instr(char* a, char b)
{
	static long i;

	for (i = 0; a[i] != 0; i++)
	{
		if (a[i] == b)
			return i;
	}

	return -1;
}

/* Return min{max{a, b}, c}. */
long iparm(char* a, long b, long c)
{
	imovd(rw1, a);

	if (icmpk(rw1, b) < 0)
		return b;

	if (icmpk(rw1, c) > 0)
		return c;

	return iwd0(rw1);
}

/* Set res20 = a (mod 10^20) */
void get_res20(ireg* a)
{
	imov(res20, a);
	imovd(rw1, "100000000000000000000");
	imod(res20, rw1);
}

/* Set res20 = trace(j(0, p, q)^[N/p^k] mod N) mod 10^20 */
void get_APRCL_res20(void)
{
	static long i;

	imov(res20, rJS[0]);
	imulk(res20, kPL); /* (p - 1)*p^(k - 1)*JS_0 */

	for (i = kPM; i < kPK; i += kPM)
		iadd(res20, rJS[i]);

	imod(res20, rN);
	imovd(rw1, "100000000000000000000");
	imod(res20, rw1);
}

/* Print and log things. */
void lprint(char* s, ...)
{
	static va_list arglist;

	if (dat_file == NULL)
		while ((dat_file = fopen("vfypr.dat", "a")) == NULL)
			file_fail("dat", "open");

	va_start(arglist, s);
	vprintf(s, arglist);
	va_end(arglist);

	va_start(arglist, s);
	vfprintf(dat_file, s, arglist);
	va_end(arglist);
}

/* Print and log an integer register and possibly other things. */
void iprint(char* s, ireg* a, ...)
{
	static va_list arglist;

	if (dat_file == NULL)
		while ((dat_file = fopen("vfypr.dat", "a")) == NULL)
			file_fail("dat", "open");

	va_start(arglist, a);
	vprintf(s, arglist);
	va_end(arglist);
	idisp(a);

	va_start(arglist, a);
	vfprintf(dat_file, s, arglist);
	va_end(arglist);
	idispf(dat_file, a);
}

/* Print and log an integer register, possibly other things and a carriage-return. */
void iprtcr(char* s, ireg* a, ...)
{
	static va_list arglist;

	if (dat_file == NULL)
		while ((dat_file = fopen("vfypr.dat", "a")) == NULL)
			file_fail("dat", "open");

	va_start(arglist, a);
	vprintf(s, arglist);
	va_end(arglist);
	idisp(a);
	printf("\n");

	va_start(arglist, a);
	vfprintf(dat_file, s, arglist);
	va_end(arglist);
	idispf(dat_file, a);
	fprintf(dat_file, "\n");
}

/* Write c, a and possibly other things to the journal. */
void jprint(char* c, ireg* a, ...)
{
	static va_list arglist;

	if (jnl_file == NULL)
		while ((jnl_file = fopen("vfypr.jnl", "a")) == NULL)
			file_fail("jnl", "open");

	va_start(arglist, a);
	vfprintf(jnl_file, c, arglist);
	va_end(arglist);

	if (a != r0)
		idispf(jnl_file, a);

	fprintf(jnl_file, "\n");
}

/* Write a long vector v[a] to v[b] to the journal. */
void jprint_vector(char* c, long* v, long a, long b)
{
	static long i;

	for (i = a; i <= b; i++)
		jprint("%s %d %d", r0, c, i, v[i]);
}

/* Write a Jacobi vector to the journal. */
void jprint_jacobi(char* c, ireg* v)
{
	static long i;

	for (i = 0; i < kPK; i++)
		jprint("%s %d ", &v[i], c, i);
}

/* Write many APRCL things to the journal. */
void jprint_aprcl_main(long a)
{
	jprint("APRCL-M %d %d %d %d %d %d %d %d %d %d",
	       r0, a, kP, kQ, kK, kG, kHj, kPK, kPL, kPM, aprcl_Lp);

	jprint_jacobi("J0", (ireg*)rJ0);
	jprint_jacobi("J1", (ireg*)rJ1);
	jprint_jacobi("J2", (ireg*)rJ2);

	jprint_jacobi("JW", (ireg*)rJW);
	jprint_jacobi("JS", (ireg*)rJS);
	jprint_jacobi("JX", (ireg*)rJX);

	jprint_jacobi("J[0]", (ireg*)rJ[0]);
	jprint_jacobi("J[1]", (ireg*)rJ[1]);
}

/*
Return the smallest prime divisor < prmdiv_hi of ireg a.
If a has no prime divisor < prmdiv_hi, return 0.
*/
long iprmdiv(ireg* a)
{
	static long p;

	for (p = 2; p < prmdiv_hi; p = xnxtprmk(p))
	{
		imov(rw3, a);
		imodk(rw3, p);

		if (isgn(rw3) == 0)
			return p;
	}

	return 0;
}

/*
Return the smallest prime divisor < prmdiv_hi of a.
If a has no prime divisor < prmdiv_hi, return 0.
0 < a < 2^31.
*/
long prmdiv(long a)
{
	static long p;

	for (p = 2; p < prmdiv_hi; p = xnxtprmk(p))
	{
		if (a % p == 0)
			return p;
	}

	return 0;
}

/*
Return (a0/b0).

This is the algorithm in H. Cohen, CCANT, page 29.
*/
long kro(long a0, long b0)
{
	static long a;
	static long b;
	static long k;
	static long v;
	static long tab[] = {0, 1, 0, -1, 0, -1, 0, 1};

	a = a0;
	b = b0;

	/* Test b equal to 0 */
	if (b == 0)
	{
		if (abs(a) == 1)
			return 1; /* b = 0, |a| = 1 */
		else
			return 0; /* b = 0, |a| != 1 */
	}

	if (!((a | b) & 1))
		return 0; /* a, b even */

	/* Remove 2's from b */
	v = 0;

	while (!(b & 1))
	{
		v ^= 1;
		b >>= 1;
	}

	if (!v)
		k = 1; /* if v even, k = 1 */
	else
		k = tab[a & 7]; /* If v odd, k = (-1)^((a^2 - 1)/8) */

	if (b < 0)
	{
		b = -b;

		if (a < 0)
			k = -k;
	}

	/* Finished? (here b is odd and b > 0) */
test_finished:
	if (a == 0)
	{
		if (b > 1)
			return 0;
		else
			return k;
	}

	v = 0;

	while (!(a & 1))
	{
		v ^= 1;
		a >>= 1;
	}

	/* Both a and b are now odd */
	if ((v) && (tab[b & 7] == -1))
		k = -k; /* k *= (-1)^((b^2 - 1)/8) */

	if (a & b & 2)
		k = -k; /* Quadratic reciprocity */

	v = abs(a);
	a = b % v;
	b = v;

	goto test_finished;
}

/* Return (a/N), assuming that both a and N are odd. */
long q_r_mod_N(long a)
{
	imov(rw1, rN);
	imodk(rw1, a); /* w1 = N mod a */

	if (a & iwd0(rN) & 2)
		return -kro(iwd0(rw1), a);
	else
		return kro(iwd0(rw1), a);
}

/* Return 1 if |a| is probably an integer square. Otherwise 0. */
long prob_square(ireg* a)
{
	static long i;

	if (qres11[0] == 0)
	{
		for (i = 0; i < 11; i++)
			qres11[i] = 0;

		for (i = 0; i < 63; i++)
			qres63[i] = 0;

		for (i = 0; i < 65; i++)
			qres65[i] = 0;

		for (i = 0; i < 256; i++)
			qres256[i] = 0;

		for (i = 0; i <= 5; i++)
			qres11[(i * i) % 11] = 1;

		for (i = 0; i <= 31; i++)
			qres63[(i * i) % 63] = 1;

		for (i = 0; i <= 32; i++)
			qres65[(i * i) % 65] = 1;

		for (i = 0; i <= 63; i++)
			qres256[(i * i) % 256] = 1; /* (64 + x)^2 == (64 - x)^2 (mod 256) */
	}

	if (qres256[iwd0(a) & 255] == 0)
		return 0;

	imov(rw1, a);
	imodk(rw1, 45045);

	if (qres65[iwd0(rw1) % 65] == 0)
		return 0;

	if (qres63[iwd0(rw1) % 63] == 0)
		return 0;

	if (qres11[iwd0(rw1) % 11] == 0)
		return 0;

	return 1;
}

void arithmetic_check(void)
{
	static long a;
	static long b;
	static long c;
	static long d;
	static long h;
	static long i;
	static long j;

	lprint("wexp %d %d %d, %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
	       0, 5, wexp(0, 5), 1, 5, wexp(1, 5), 2, 5, wexp(2, 5), 2, 0, wexp(2, 0), 2, 1, wexp(2, 1));

	lprint("wmod %d %d %d, %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
	       0, 5, wmod(0, 5), 1, 5, wmod(1, 5), 19, 5, wmod(19, 5), 5, 5, wmod(5, 5), 4, 1, wmod(4, 1));

	lprint("wmods %d %d %d, %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
	       0, 5, wmods(0, 5), 1, 5, wmods(1, 5), 19, 5, wmods(19, 5), 5, 5, wmods(5, 5), 4, 1, wmods(4, 1));

	lprint("wmods %d %d %d, %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
	       0, 5, wmods(0, 5), -1, 5, wmods(-1, 5), -19, 5, wmods(-19, 5), -5, 5, wmods(-5, 5), -4, 1, wmods(-4, 1));

	lprint("kro %d %d %d, %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
	       0, 5, kro(0, 5), 1, 5, kro(1, 5), -1, 5, kro(-1, 5), 19, 5, kro(19, 5), -19, 5, kro(-19, 1));

	lprint("kro %d %d %d, %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
	       20, 35, kro(20, 35), 91, 109, kro(91, 109), -7, 7, kro(-7, 7), 19, 7, kro(19, 7), 79, 97, kro(79, 97));

	for (i = 1; i <= 10; i++)
	{
		for (j = -3; j <= 3; j++)
		{
			imovd(rw2, "100000000000000000000");
			iaddk(rw2, i);
			isqu(rw2);
			iaddk(rw2, j);

			h = prob_square(rw2);

			imov(rw3, rw2);
			imodk(rw3, 11);
			a = iwd0(rw3);

			imov(rw3, rw2);
			imodk(rw3, 63);
			b = iwd0(rw3);

			imov(rw3, rw2);
			imodk(rw3, 65);
			c = iwd0(rw3);

			imov(rw3, rw2);
			imodk(rw3, 256);
			d = iwd0(rw3);

			lprint("prob_square (10^20 + %d)^2 + %d: %d %d%d%d%d\n",
			       i, j, h, qres11[a], qres63[b], qres65[c], qres256[d]);
		}
	}
}

void save_parameters(long r, ireg* s, long t, long i, long j, long k)
{
	if (pcount == -1)
		return; /* Ignore if restart has been disabled */

	remove("vfypr.bak");
	rename("vfypr.ini", "vfypr.bak");

	while ((ini_file = fopen("vfypr.ini", "w")) == NULL)
		file_fail("ini", "open");

	fprintf(ini_file, "%ld\n", r);

	idispf(ini_file, rN);
	fprintf(ini_file, "\n");

	idispf(ini_file, rF1);
	fprintf(ini_file, "\n");

	idispf(ini_file, rF2);
	fprintf(ini_file, "\n");

	idispf(ini_file, s);
	fprintf(ini_file, "\n");

	fprintf(ini_file, "%ld \n%ld %ld %ld \n", t, i, j, k);
	fclose(ini_file);

	if (r == 0)
	{
		pcount = -1;
		return;
	} /* Disable restart */

	pcount++;

	psecs = difftime(time(NULL), ptime);
	ptime = time(NULL);

	lprint("Parameters saved: %d %d %d %d %d %d %0.0f\n", r, t, i, j, k, pcount, psecs);

	fclose(dat_file);
	dat_file = NULL;
}

/*
Compute (mod M) W = W_{j*k}, U = U_{j*k} from W_j and U_j in
the Lucas sequence defined by x^2 - p*x + q. Write

x^i = W_i + U_i*x (mod x^2 - p*x + q).

Then W_i = (V_i - p*U_i)/2, and U_i and V_i are the usual
Lucas sequences defined by

U_0 = 0,  U_1 = 1,  U_{i+2} = p*U_{i+1} - q*U_i,
V_0 = 2,  V_1 = p,  U_{i+2} = p*V_{i+1} + q*V_i.

Thus
W_0 = 1,  W_1 = 0,
U_0 = 0,  U_1 = 1.

The discriminant is p^2 - 4*q.

We use the recursion formulae:

W_{2*i} = W_i^2     - q*U_i^2,
U_{2*i} = 2*W_i*U_i + p*U_i^2,

W_{i+j} = W_i*W_j - q*U_i*U_j,
U_{i+j} = U_i*W_j + W_i*U_j + p*U_i*U_j.

Everything is modulo M.
*/
void expmod_Lucas(ireg* xW, ireg* xU, ireg* xWj, ireg* xUj, ireg* xk, long p, long q, ireg* xM)
{
	static long i;
	static unsigned long w;

	imov(rWj, xWj);
	imov(rUj, xUj);

	imovk(xW, 1);
	imovk(xU, 0);

	utrim(xk);

	for (w = 0x80000000; w > 0; w >>= 1)
	{
		if (xk->value[xk->digits - 1] & w)
			break;
	}

	for (i = xk->digits - 1; i >= 0; i--)
	{
		for (; w > 0; w >>= 1)
		{
			imov(rw1, xU);
			isqu(rw1); /* x = U^2 */

			imov(rw2, rw1);
			imulk(rw2, q); /* y = U^2*q */

			imov(rw3, xW); /* z = W */

			isqu(xW);
			isub(xW, rw2);
			imod(xW, xM); /* W = z^2 - U^2*q (mod M) */

			imulk(rw1, p); /* x = U^2*p */

			imul(xU, rw3);
			iadd(xU, xU);
			iadd(xU, rw1);
			imod(xU, xM); /* U = 2*z*U + U^2*p (mod M) */

			if (xk->value[i] & w)
			{
				imov(rw3, xW); /* z = W */

				imov(rw1, xU);
				imul(rw1, rUj); /* x = U*Uj */

				imov(rw2, rw1);
				imulk(rw2, q); /* y = U*Uj*q */

				imov(xW, rw3);
				imul(xW, rWj);
				isub(xW, rw2);
				imod(xW, xM); /* W = z*Wj - U*Uj*q (mod M) */

				imulk(rw1, p); /* x = U*Uj*p */

				imov(rw2, rw3);
				imul(rw2, rUj); /* y = z*Uj */

				imul(xU, rWj);
				iadd(xU, rw1);
				iadd(xU, rw2);
				imod(xU, xM); /* U = U*Wj + z*Uj + U*Uj*q (mod M) */
			}
		}

		w = 0x80000000;
	}
}

/* Print and log results of the Lucas powering algorithm test. */
void expmod_Lucas_check_print(char* a, ireg* rw, ireg* ru, long p, long q, long k, ireg* rm)
{
	static ireg rrv;
	static ireg* rv = &rrv;

	imov(rv, ru);
	imulk(rv, p);
	iadd(rv, rw);
	iadd(rv, rw);
	imod(rv, rm); /* v = (2*w + u*p) mod m */

	iprint("%s p=%d q=%d k=%d U=", ru, a, p, q, k);
	iprtcr(" V=", rv);
}

/* Do a single test of the Lucas powering algorithm. */
void expmod_Lucas_check_single(long k1, long k2, long k3, long p, long q)
{
	static ireg rrk;
	static ireg rrm;
	static ireg rru;
	static ireg rrw;

	static ireg* rk = &rrk;
	static ireg* rm = &rrm;
	static ireg* ru = &rru;
	static ireg* rw = &rrw;

	imovd(rm, "10000000000000000000000000000000000000000");

	imovk(rk, k1);

	expmod_Lucas(rw, ru, r0, r1, rk, p, q, rm);
	expmod_Lucas_check_print("A", rw, ru, p, q, k1, rm);

	imovk(rk, k2);

	expmod_Lucas(rw, ru, rw, ru, rk, p, q, rm);
	expmod_Lucas_check_print("B", rw, ru, p, q, k1 * k2, rm);

	imovk(rk, k3);

	expmod_Lucas(rw, ru, rw, ru, rk, p, q, rm);
	expmod_Lucas_check_print("C", rw, ru, p, q, k1 * k2 * k3, rm);

	imovk(rk, k1 * k2 * k3);

	expmod_Lucas(rw, ru, r0, r1, rk, p, q, rm);
	expmod_Lucas_check_print("D", rw, ru, p, q, k1 * k2 * k3, rm);
}

/*
Test the Lucas powering algorithm. Expected results:

A p=1 q=-1 k=1 U=1 V=1
B p=1 q=-1 k=19 U=4181 V=9349
C p=1 q=-1 k=38 U=39088169 V=87403803
D p=1 q=-1 k=38 U=39088169 V=87403803
A p=1 q=-1 k=6 U=8 V=18
B p=1 q=-1 k=18 U=2584 V=5778
C p=1 q=-1 k=36 U=14930352 V=33385282
D p=1 q=-1 k=36 U=14930352 V=33385282
A p=2 q=-1 k=4 U=12 V=34
B p=2 q=-1 k=12 U=13860 V=39202
C p=2 q=-1 k=36 U=21300003689580 V=60245508192802
D p=2 q=-1 k=36 U=21300003689580 V=60245508192802
A p=2 q=-1 k=1 U=1 V=2
B p=2 q=-1 k=17 U=1136689 V=3215042
C p=2 q=-1 k=34 U=3654502875938 V=10336495061766
D p=2 q=-1 k=34 U=3654502875938 V=10336495061766
A p=4 q=3 k=2 U=4 V=10
B p=4 q=3 k=6 U=364 V=730
C p=4 q=3 k=30 U=102945566047324 V=205891132094650
D p=4 q=3 k=30 U=102945566047324 V=205891132094650
A p=4 q=3 k=1 U=1 V=4
B p=4 q=3 k=11 U=88573 V=177148
C p=4 q=3 k=33 U=2779530283277761 V=5559060566555524
D p=4 q=3 k=33 U=2779530283277761 V=5559060566555524
A p=3 q=2 k=2 U=3 V=5
B p=3 q=2 k=12 U=4095 V=4097
C p=3 q=2 k=36 U=68719476735 V=68719476737
D p=3 q=2 k=36 U=68719476735 V=68719476737
A p=3 q=2 k=13 U=8191 V=8193
B p=3 q=2 k=143 U=372599265311570767859136324180752990207 V=372599265311570767859136324180752990209
C p=3 q=2 k=1001 U=3084335320859663305248773674411336138751 V=3084335320859663305248773674411336138753
D p=3 q=2 k=1001 U=3084335320859663305248773674411336138751 V=3084335320859663305248773674411336138753
*/
void expmod_Lucas_check(void)
{
	lprint("Checking Lucas algorithm\n");

	expmod_Lucas_check_single(1, 19, 2, 1, -1); /*  Fibonacci (Ribenboim, LBBP, p50) */
	expmod_Lucas_check_single(6, 3, 2, 1, -1); /*  Fibonacci (Ribenboim, LBBP, p50) */

	expmod_Lucas_check_single(4, 3, 3, 2, -1); /*  Pell (Ribenboim, LBBP, p 52) */
	expmod_Lucas_check_single(1, 17, 2, 2, -1); /*  Pell (Ribenboim, LBBP, p 52) */

	expmod_Lucas_check_single(2, 3, 5, 4, 3); /*  (Ribenboim, LBBP, p 53) */
	expmod_Lucas_check_single(1, 11, 3, 4, 3); /*  (Ribenboim, LBBP, p 53) */

	expmod_Lucas_check_single(2, 6, 3, 3, 2); /*  2^n-1 and 2^n+1 */
	expmod_Lucas_check_single(13, 11, 7, 3, 2); /*  2^n-1 and 2^n+1 */

	expmod_Lucas_check_single(1, 19, 2, 5, 5); /*  d = p = q = 5 */
	expmod_Lucas_check_single(1, 19, 2, 7, 11); /*  d = 5, p = 7, q = 11 */

	expmod_Lucas_check_single(6, 3, 2, 5, 5); /*  d = p = q = 5 */
	expmod_Lucas_check_single(6, 3, 2, 7, 11); /*  d = 5, p = 7, q = 11 */
}

/*
See if we have sufficient parameters for a primality proof.

Start with G = max(F1*F2/2, 1).

If q > 1 and does not divide G, multiply S by q and again
by the highest power of q that divides t; and once more if
q = 2. Then multiply G by S.

Finally, compute H = G*max(F1, F2)^2.

The condition we need is G^2 > N, or G^3 > N and m*H > N.
*/
long test_sufficient(long q, long t, long m)
{
	static long u;

	imov(rG, rF1);
	imul(rG, rF2);
	idiv2k(rG, 1);

	if (isgn(rG) <= 0)
		imovk(rG, 1); /* G = max(F1*F2/2, 1) */

	if (q > 1)
	{
		imov(rx, rG);
		imodk(rx, q);

		if (isgn(rx))
		{
			for (u = t; ; u /= q) /* q does not divide G */
			{
				imulk(rS, q); /* Multiply S by q */

				if (u % q)
					break; /* Until u not == 0 (mod q) */
			}

			if (q == 2)
				iadd(rS, rS); /* One more 2 */
		}
	}

	imul(rG, rS); /* G = F1*F2*S/2 */

	if (icmp(rG, rsqrtN) > 0)
		return 1; /* G > sqrt(N) */

	if (aprcl_only)
		return 0;

	imov(rH, rG);

	if (icmp(rF1, rF2) >= 0)
	{
		imul(rH, rF1);
		imul(rH, rF1);
	}
	else
	{
		imul(rH, rF2);
		imul(rH, rF2);
	}

	imov(rx, rG);
	isqu(rx); /* x = G^2 */
	imul(rx, rG); /* x = G^3 */

	if (icmp(rx, rN) <= 0)
		return 0; /* G^3 <= N */

	imov(rx, rH);
	imulk(rx, m);

	if (icmp(rx, rN) > 0)
		return 1; /* G^3 > N and m*H > N */

	return 0;
}

long BLS_check(ireg* rF, ireg* rR, ireg* sF, long num_fac_F, long t)
{
	imov(rw1, rF);
	imul(rw1, rR);
	isub(rw1, rN);
	isubk(rw1, t);
	imov(rx, rF);
	imov(ry, rR);
	igcd(rx, ry);

	if (isgn(rw1) || /* w1 = F*R - (N + t) */
		iodd(rF) || ieven(rR) ||
		icmp(rx, r1) != 0 || /* x = gcd(F, R) */
		icmp(rF, sF) != 0 || num_fac_F)
	{
		lprint("Error in computation of F%d: t=%d num_fac=%d\n", (t + 3) / 2, t, num_fac_F);

		iprtcr("rF=", rF);
		iprtcr("sF=", sF);
		iprtcr("rR=", rR);

		iprtcr("w1=", rw1);
		iprtcr("w2=", rw2);
		iprtcr("w3=", rw3);

		gen_fail();
		return 0;
	}

	return 1;
}

/*
If f|R1, save the factor, update F1 and R1.
If f|R2, save the factor, update F2 and R2.
Check to see if enough factors have been found for a
primality proof.
*/
long BLS_test_factor(void)
{
	static long e1;
	static long e2;

	for (e1 = 0; ; e1++)
	{
		imov(rx, rR1);
		idiv(rx, rf);

		if (isgn(res))
			break;

		imov(rR1, rx);
		imul(rF1, rf);
	}

	for (e2 = 0; ; e2++)
	{
		imov(rx, rR2);
		idiv(rx, rf);

		if (isgn(res))
			break;

		imov(rR2, rx);
		imul(rF2, rf);
	}

	if (e1)
	{
		iprint("Factor: ", rf);

		if (e1 > 1)
			lprint("^%d", e1);

		lprint(" divides N - 1\n");
		idispf(fac_file, rf);
		fprintf(fac_file, " %ld -1 \n", e1);

		num_fac_F1++;
	}

	if (e2)
	{
		iprint("Factor: ", rf);

		if (e2 > 1)
			lprint("^%d", e2);

		lprint(" divides N + 1\n");
		idispf(fac_file, rf);
		fprintf(fac_file, " %ld 1 \n", e2);

		num_fac_F2++;
	}

	if (e1 || e2)
		return test_sufficient(1, 1, 1);

	return 0;
}

/*
Get next p and q for Lucas sequence. Note that
(p + 2)^2 - 4*(q + p + 1) = p^2 - 4*q = d.

We need gcd(N, 2*q*d) = 1. We already have gcd(N, 2) = 1
and gcd(N, d) = 1 (because (d/N) = -1). Therefore we only
really have to check gcd(N, q) = 1.
*/
void get_next_p_q(void)
{
	for (;;)
	{
		q += p + 1;
		p += 2;

		imov(rw1, rN);
		imovk(rw2, q);
		imulk(rw2, p);
		igcd(rw1, rw2); /* w1 = gcd(N, p*q) */

		if (icmp(rw1, r1) == 0)
			break;
	}
}

/*
Given N, Either prove that N is prime, or obtain F1 and F2
such that

(i)    N - 1 = F1*R1, 2|F1 and gcd(F1, R1) = 1.

(ii)   F1 is completely factorized into verified primes.

(iii)  For each prime factor f of F1, there exists a number
a such that 1 < a < N - 1, a^(N - 1) = 1 (mod N) and
gcd(a^((N - 1)/f) - 1, N) = 1.

(iv)   N + 1 = F2*R2, 2|F2 and gcd(F2, R2) = 1.

(v)    F2 is completely factorized into verified primes.

(vi)   There is an integer D such that (D/N) = -1 and for
each prime factor f of F, there exists a Lucas
sequence {U_k} with discriminant D = P^2 - 4*Q,
where gcd(N, Q) = 1, for which U_(N + 1) = 0 (mod N)
and gcd(U_((N + 1)/f), N) = 1.

Pocklington's Theorem. If d is a prime divisor of N and if
(i), (ii) and (iii) hold, then d == 1 (mod F1).

Morrison's Theorem. If d is a prime divisor of N and if
(vi), (v) and (vi) hold, then d == +/-1 (mod F2).

See Brillhart, Lehmer & Selfridge's paper.

Return value

-1   Nothing meaningful was done.

0   N is composite.

1   N is prime.

3   All BLS tests have passed but N^2 - 1 was not
sufficiently factorized to prove that N is prime.
Returns F1 and F2, the factorized parts of N - 1
and N + 1.
*/
long BLS_test(void)
{
	static long a;
	static long a0;
	static long d;
	static long e;
	static long i;
	static long p0;
	static long q0;
	static long t;

	/* Look for factors of N - 1 and N + 1 */
	if (restart == 1)
	{
		num_fac_F1 = restart_i;
		imov(rR1, rN);
		isubk(rR1, 1);
		idiv(rR1, rF1);
		num_fac_F2 = restart_j;
		imov(rR2, rN);
		iaddk(rR2, 1);
		idiv(rR2, rF2);

		goto StartVerify;
	}

	imovk(rF1, 1);
	imov(rR1, rN);
	isubk(rR1, 1);
	imovk(rF2, 1);
	imov(rR2, rN);
	iaddk(rR2, 1);
	remove("vfypr.fac");

	while ((fac_file = fopen("vfypr.fac", "w")) == NULL)
		file_fail("FAC", "open");

	num_fac_F1 = 0;
	num_fac_F2 = 0;

	printf("Searching 'vfypr.prm' for factors\n");
	prm_file = fopen("vfypr.prm", "r");

	while (fgets(buffer, buffer_dim, prm_file) != NULL)
	{
		if (test_quit())
			halt();

		imov(rf, ieval(buffer));

		if (*eval_err != 0)
		{
			printf("vfypr.prm: %s: %s", eval_err, buffer);
			halt();

			continue;
		}

		if (isgn(rf) == 0)
			break;

		if (icmpk(rf, 1) <= 0)
		{
			printf("vfypr.prm: This number is too small: %s", buffer);
			halt();

			continue;
		}

		BLS_test_factor(); /* Search all of VFYPR.PRM */
	}

	printf("Searching for small factors\n");
	imovk(rf, 2);

	if (BLS_test_factor())
		goto EndFactorSearch;

	for (p = 3; p <= F_max; p = xnxtprmk(p))
	{
		if (test_quit())
			halt();

		imov(rx, rN);
		iaddk(rx, 1);
		imodk(rx, p); /* x = N + 1 mod p */

		if (icmpk(rx, 2) > 0)
			continue;

		imovk(rf, p);

		if (BLS_test_factor())
			goto EndFactorSearch;
	}

EndFactorSearch:
	fclose(prm_file);
	fclose(fac_file);

	lprint("Factorization results: F1=%0.4f F2=%0.4f\n", ilog(rF1) / ilogN, ilog(rF2) / ilogN);

	iprtcr("F1=", rF1);
	iprtcr("F2=", rF2);

StartVerify:
	imovk(sF1, 1);
	imovk(sF2, 1);

	printf("Verifying BLS conditions\n");

	/* Choose a such that (a/N) = -1 */
	a0 = 0;

	for (a = 3; q_r_mod_N(a) != -1; a += 2);

	/* Choose d = p^2 - 4*q such that (d/N) = -1 */
	p0 = 0;
	q0 = 0;

	for (d = 5; q_r_mod_N(d) != -1; d += 4);

	p = -1;
	q = (p * p - d) / 4;

	get_next_p_q();

	while ((fac_file = fopen("VFYPR.FAC", "r")) == NULL)
		file_fail("FAC", "open");

	while (fscanf(fac_file, "%s %d %d ", buffer, &e, &t) != EOF)
	{
		imovd(rf, buffer);

		if (t == -1)
		{
			for (i = 0; i < e; i++)
				imul(sF1, rf);
		}
		else
		{
			for (i = 0; i < e; i++)
				imul(sF2, rf);
		}

		if (restart == 1)
		{
			if (!(icmp(rf, restart_s) == 0 && t == restart_t))
				continue; /* Wind on to restart factor */

			iprtcr("Restarting BLS tests from factor ", restart_s);
			restart = 0;
		}

		psecs = difftime(time(NULL), ptime);

		if (test_quit() || psecs > 1800.0)
		{
			save_parameters(1, rf, t, num_fac_F1, num_fac_F2, 0);

			if (quit)
				halt();
		}

		imovk(ry, 3);
		iexpmod(rx, ry, rf, rf);

		if (icmpk(rx, 3) != 0 && icmpk(rf, 3) > 0)
		{
			printf("VFYPR.PRM: THIS NUMBER IS NOT PRIME: %s", buffer);
			halt();
			fclose(fac_file);
			return -1;
		}

		if (t == -1)
		{
			/* Factors of N - 1 */
			num_fac_F1--;

			if (icmpk(rF1, 2) <= 0)
				continue; /* Do nothing if F1 <= 2 */

			for (;;)
			{
				if (test_quit())
					halt();

				imov(rx, rN);
				isubk(rx, 1);
				idiv(rx, rf); /* x = (N-1)/f */

				imovk(ra, a);
				iexpmod(rW, ra, rx, rN); /* W = a^((N-1)/f) (mod N) */

				imov(rx, rW);
				isubk(rx, 1);
				get_res20(rx); /* Save res20 for message */

				imov(ry, rN);
				igcd(rx, ry); /* x = gcd(a^((N-1)/f) - 1, N) */

				if (icmp(rx, r1) == 0)
					break;

				iprint("Fail: gcd(%d^((N-1)/", rf, a);
				iprtcr(") - 1, N) not = 1: R20=", res20);

				if (icmp(r1, rx) < 0 && icmp(rx, rN) < 0)
				{
					imov(rW, rx);
					return 0;
				} /* Composite */

				for (a += 2; q_r_mod_N(a) != -1; a += 2);
			}

			iprint("Pass: gcd(%d^((N-1)/", rf, a);
			iprtcr(") - 1, N) = 1: R20=", res20);

			if (a == a0)
				continue;

			a0 = a;
			iexpmod(rx, rW, rf, rN); /* x = a^(N-1) (mod N) */
			get_res20(rx);

			if (icmp(rx, r1) != 0)
			{
				iprtcr("Fail: %d^(N-1) not = 1 (mod N): R20=", res20, a);
				fclose(fac_file);
				return 0; /* Composite */
			}

			iprtcr("Pass: %d^(N-1) = 1 (mod N): R20=", res20, a);
		}
		else
		{
			/* Factors of N + 1 */
			num_fac_F2--;

			if (icmpk(rF2, 4) <= 0)
				continue; /* Do nothing if F2 <= 4 */

			for (;;)
			{
				if (test_quit())
					halt();

				imov(rx, rN);
				iaddk(rx, 1);
				idiv(rx, rf); /* x = (N + 1)/f */

				expmod_Lucas(rW, rU, r0, r1, rx, p, q, rN);
				get_res20(rU);

				imov(rx, rU);
				imov(ry, rN);
				igcd(rx, ry); /* x = gcd(U, N) */

				if (icmp(rx, r1) == 0)
					break;

				iprint("Fail: gcd(U{(N+1)/", rf);
				iprtcr("}, N) not = 1: d=%d p=%d q=%d R20=", res20, d, p, q);

				if (icmp(r1, rx) < 0 && icmp(rx, rN) < 0)
				{
					imov(rW, rx);
					return 0;
				} /* Composite */

				get_next_p_q();
			}

			iprint("Pass: gcd(U{(N+1)/", rf);
			iprtcr("}, N) = 1: d=%d p=%d q=%d R20=", res20, d, p, q);

			if (p == p0 && q == q0)
				continue;

			p0 = p;
			q0 = q;

			expmod_Lucas(rW, rU, rW, rU, rf, p, q, rN);
			get_res20(rU);

			if (isgn(rU))
			{
				iprtcr("Fail: U{N+1} not = 0 (mod N): d=%d p=%d q=%d R20=", res20, d, p, q);
				fclose(fac_file);
				return 0; /* Composite */
			}

			iprtcr("Pass: U{N+1} = 0 (mod N): d=%d p=%d q=%d R20=", res20, d, p, q);
		}
	}

	if (!BLS_check(rF1, rR1, sF1, num_fac_F1, -1))
		return -1;

	if (!BLS_check(rF2, rR2, sF2, num_fac_F2, 1))
		return -1;

	lprint("BLS tests passed: F1=%0.4f F2=%0.4f\n", ilog(rF1) / ilogN, ilog(rF2) / ilogN);
	fclose(fac_file);

	imov(rx, rF1);
	iaddk(rx, 1); /* x = F1 + 1 */

	isqu(rx); /* x = (F1 + 1)^2 */

	if (icmp(rx, rN) > 0)
	{
		get_res20(rx);
		iprtcr("Pass: (F1+1)^2 > N: R20=", res20);
		return 1; /* Prime */
	}

	imov(rx, rF2);
	isqu(rx);
	isubk(rx, 1); /* x = (F2 - 1)*(F2 + 1) */

	if (icmp(rx, rN) > 0)
	{
		get_res20(rx);
		iprtcr("Pass: (F2-1)*(F2+1) > N: R20=", res20);
		return 1; /* Prime */
	}

	return 3;
}

/*
Load APRCL prime table with 2 and all the q's that we may need.
Used to generate a set of q's for compiling into the program.
*/
void load_pr_tab(long maxQ)
{
	static long n;
	static long q;

	lprint("long pr_tab[kNQdim] = \n  {0, 2");

	for (n = 1, q = 3; q <= maxQ; q = xnxtprmk(q))
	{
		if (test_quit())
			halt();

		if (kTmax % (q - 1) == 0)
		{
			n++;

			if (n % 10 == 1)
				lprint(",\n  %d", q);
			else
				lprint(", %d", q);
		}
	}

	lprint("};   /* %d primes */\n", n);
}

/*
Return ind_g(x) (mod q) using vIndx table.
0 < x < q.
*/
long ind(long x)
{
	if (x <= kQH)
		return vIndx[x];

	return (kQH + vIndx[kQ - x]) % kQM;
}

/*
Return a primitive root mod p.
0 < p < 2^31, p prime.
*/
long Genshi(long p)
{
	static long gen;
	static long n;
	static long w;
	static long d;
	static long s;

	gen = 1;
	n = p - 1;

	do
	{
		gen++;

		w = n;
		s = 1;

		do
		{
			d = prmdiv(w);

			while (w % d == 0)
			{
				w /= d;
			};

			if (wexpmod(gen, n / d, p) == 1)
			{
				s = 0;
				w = 1;
			}
		}
		while (w != 1);
	}
	while (s == 0);

	return gen;
}

/*
Return the exponent of the highest power of p that divides a.
0 < p < 2^31, 0 <= a <= 2^31.
*/
long PowerIndex(long p, long a)
{
	static long k;

	k = 0;

	while (a % p == 0)
	{
		k++;
		a /= p;
	}

	return k;
}

/* Normalize the coefficient of JW */
void NormalizeJW(void)
{
	static long i;
	static long j;

	for (i = kPL; i < kPK; i++)
	{
		if (isgn(rJW[i]))
		{
			imov(rx, rJW[i]);

			for (j = 1; j < kP; j++)
				isub(rJW[i - j * kPM], rx);

			imovk(rJW[i], 0);
		}
	}
}

/* Normalize the coefficient of JS */
void NormalizeJS(void)
{
	static long i;
	static long j;

	for (i = kPL; i < kPK; i++)
	{
		if (isgn(rJS[i]))
		{
			imov(rx, rJS[i]);

			for (j = 1; j < kP; j++)
				isub(rJS[i - j * kPM], rx);

			imovk(rJS[i], 0);
		}
	}
}

/* JS = JS^2 */
void JS_2(void)
{
	static long i;
	static long j;
	static long k;

	for (i = 0; i < kPL; i++)
	{
		k = (i + i) % kPK;

		imov(rx, rJS[i]);
		isqu(rx);

		iadd(rJX[k], rx);
		imod(rJX[k], rN);
	}

	for (i = 0; i < kPL; i++)
	{
		for (j = i + 1; j < kPL; j++)
		{
			k = (i + j) % kPK;

			imov(rx, rJS[i]);
			imul(rx, rJS[j]);
			iadd(rx, rx);

			iadd(rJX[k], rx);
			imod(rJX[k], rN);
		}
	}

	for (i = 0; i < kPK; i++)
		imov(rJS[i], rJX[i]);

	for (i = 0; i < kPK; i++)
		imovk(rJX[i], 0);

	NormalizeJS();
}

/* JS = JS*JW */
void JS_JW(void)
{
	static long i;
	static long j;
	static long k;

	for (i = 0; i < kPL; i++)
	{
		for (j = 0; j < kPL; j++)
		{
			k = (i + j) % kPK;

			imov(rx, rJS[i]);
			imul(rx, rJW[j]);

			iadd(rJX[k], rx);
			imod(rJX[k], rN);
		}
	}

	for (i = 0; i < kPK; i++)
		imov(rJS[i], rJX[i]);

	for (i = 0; i < kPK; i++)
		imovk(rJX[i], 0);

	NormalizeJS();
}

/* JS = JS^E */
void JS_E(void)
{
	static long i;
	static unsigned long w;

	if (icmp(rE, r1) == 0)
		return;

	for (i = 0; i < kPL; i++)
		imov(rJW[i], rJS[i]);

	utrim(rE);

	for (w = 0x80000000; w > 0; w >>= 1)
	{
		if (rE->value[rE->digits - 1] & w)
			break;
	}

	w >>= 1;

	for (i = rE->digits - 1; i >= 0; i--)
	{
		for (; w > 0; w >>= 1)
		{
			JS_2();

			if (rE->value[i] & w)
				JS_JW();
		}

		w = 0x80000000;
	}
}

/* Table of inverses mod P^k: Inv(x)*x mod PK = 1 */
void GetInverseTable(void)
{
	static long i;

	for (i = 1; i < kPK; i++)
	{
		if (i % kP)
			vInv[i] = winvmod(i, kPK);
		else
			vInv[i] = 0;
	}
}

/*
Jacobi sum
0 <= a, b < 2^31.
*/
void JacobiSum(long a, long b)
{
	static long i;
	static long j;
	static long k;

	for (i = 0; i < kPL; i++)
		imovk(rJ0[i], 0);

	for (i = 1; i <= kQ - 2; i++)
	{
		/* j = a^i + b^ind_g(1 - g^i) mod p^k */
		j = (wmulmod(a, i, kPK) + wmulmod(b, ind((kQ + 1 - wexpmod(kG, i, kQ)) % kQ), kPK)) % kPK;

		if (j < kPL)
			iaddk(rJ0[j], 1);
		else
		{
			for (k = 1; k < kP; k++)
				isubk(rJ0[j - k * kPM], 1);
		}
	}
}

/* Table of index: x = G^Indx[x] (mod Q). */
void GetIndexTable(void)
{
	static long i;
	static long w;

	for (i = 1, w = 1; i < kQ; i++)
	{
		w = wmulmod(w, kG, kQ);

		if (w <= kQH)
			vIndx[w] = i;
	}
}

/*
If the Jacobi sum computation results in a p^k-th root of
unity, return it; otherwise return -1.
*/
long unity_root_J(void)
{
	static long i;
	static long j;

	imovk(rW, 0);

	for (i = 0; i < kPL; i++)
		iadd(rW, rJW[i]);

	if (icmp(rW, r1) == 0)
	{
		for (i = 0; i < kPL; i++)
		{
			if (icmp(rJW[i], r1) == 0)
				return i;
		}
	}

	imov(rx, rNm1);
	imulk(rx, kP - 1); /* x = (N - 1)*(P - 1) */

	if (icmp(rW, rx) != 0)
		return -1;

	for (i = 0; i < kPM; i++)
	{
		if (icmp(rJW[i], rNm1) == 0)
			goto lab3180;
	}

	return -1;

lab3180:
	for (j = 1; j <= kP - 2; j++)
	{
		kS = i + j * kPM;

		if (icmp(rJW[kS], rNm1) != 0)
			return -1;
	}

	return i + kPL; /* = i + (P - 1)*P^(K-1) */
}

/*
Return the exponent of zeta_p^k in the right hand side of
the (*beta) condition.
*/
long unity_root_C(void)
{
	imov(ry, rN);
	imodk(ry, kQ);
	imovk(rx, ind(iwd0(ry))); /* chi(N) = (zeta_p^k)^x */

	if (kP > 2)
	{
		imovk(ry, 1);
		imul2k(ry, kPL);
		isubk(ry, 1);
		idivk(ry, kPK);
		iadd(ry, ry); /* y = c = 2*(2^(p - 1)*p^(k - 1)) - 1)/p^k */

		ineg(ry);
		imul(rx, ry);
		imul(rx, rN); /* chi(N)^(-c*N) = (zeta_p^k)^x */
	}
	else /* p == 2 */
	{
		if (kK >= 3)
		{
			imovk(rw1, 3);
			imovk(rw2, kPK / 4);
			iexp(ry, rw1, rw2);
			isubk(ry, 1);
			idivk(ry, kPK);
			imulk(ry, 3); /* y = c = 3*(3^(2^(k - 2)) - 1)/2^k */

			ineg(ry);
			imul(rx, ry);
			imul(rx, rN); /* chi(N)^(-c*N)  if N == 1 or 3 (mod 8) */

			if ((iwd0(rN) & 7) > 4)
				iaddk(rx, kPM); /* -chi(N)^(-c*N) if N == 5 or 7 (mod 8) */
		}

		if (kK == 2)
		{
			if ((iwd0(rN) & 3) == 1)
				ineg(rx); /* chi(N)^(-1) if N == 1 (mod 4) */
			else
				iaddk(rx, 2); /* -chi(N)     if N == 3 (mod 4) */
		}

		if (kK == 1)
		{
			if ((iwd0(rN) & 3) == 3)
				iaddk(rx, 1); /* -chi(N)     if N == 3 (mod 4) */
		}
	}

	imodk(rx, kPK);

	return iwd0(rx);
}

/* J(p,q,v)                 ( J(v,i) ) */
void CalcJacobiPQVs(void)
{
	static long h;
	static long i;

	for (h = 0; h < 2; h++)
	{
		for (i = 1; i < kPK; i++)
			imovk(rJ[h][i], 0);
	}

	for (h = 0; h < 2; h++)
		imovk(rJ[h][0], 1);

	imov(rx, rN);
	imodk(rx, kPK);
	kVK = iwd0(rx);

	GetInverseTable();

	if (kP == 2)
		goto lab2290;

	for (kIV = 0; kIV <= 1; kIV++)
	{
		for (kX = 1; kX < kPK; kX++)
		{
			for (i = 0; i < kPK; i++)
				imov(rJS[i], rJ0[i]);

			if (kX % kP == 0)
				continue;

			if (kIV == 0)
			{
				imovk(rE, kX);
				goto lab2150;
			}

			imovk(rE, (kVK * kX) / kPK);

			if (isgn(rE) == 0)
				continue;

		lab2150:
			JS_E();

			for (i = 0; i < kPK; i++)
				imovk(rJW[i], 0);

			kInvX = vInv[kX];

			for (i = 0; i < kPK; i++)
			{
				kS = (i * kInvX) % kPK;
				iadd(rJW[kS], rJS[i]);
			}

			NormalizeJW();

			for (i = 0; i < kPK; i++)
				imov(rJS[i], rJ[kIV][i]);

			JS_JW();

			for (i = 0; i < kPK; i++)
				imov(rJ[kIV][i], rJS[i]);
		}
	}

	return;

lab2290:
	if (kK == 1)
	{
		imovk(rJ[0][0], kQ);
		imovk(rJ[1][0], 1);

		return;
	}

	if (kK > 2)
		goto lab2500;

	/*
	J(2,q,v) k=2             ( J(v,i) )
	*/
	if (kVK == 1)
		imovk(rJ[1][0], 1);

	for (i = 0; i <= 1; i++)
		imov(rJS[i], rJ0[i]);

	JS_2();

	if (kVK != 3)
		goto lab2430;

	for (i = 0; i <= 1; i++)
		imov(rJ[1][i], rJS[i]);

lab2430:
	for (i = 0; i <= 1; i++)
	{
		imov(rJ[0][i], rJS[i]);
		imulk(rJ[0][i], kQ);
	}

	return;

	/*
	J(2,q,v) k>2             ( J(v,i) )
	*/
lab2500:
	for (kIV = 0; kIV <= 1; kIV++)
	{
		for (kX = 1; kX < kPK; kX += 2)
		{
			for (i = 0; i <= kPM; i++)
				imov(rJS[i], rJ1[i]);

			if ((kX & 7) == 5 || (kX & 7) == 7)
				continue;

			if (kIV == 0)
				imovk(rE, kX);
			else
			{
				imovk(rE, (kVK * kX) / kPK);

				if (isgn(rE) == 0)
					continue;
			}

			JS_E();

			for (i = 0; i < kPK; i++)
				imovk(rJW[i], 0);

			kInvX = vInv[kX];

			for (i = 0; i < kPK; i++)
			{
				kS = (i * kInvX) % kPK;
				iadd(rJW[kS], rJS[i]);
			}

			NormalizeJW();

			for (i = 0; i < kPK; i++)
				imov(rJS[i], rJ[kIV][i]);

			NormalizeJS();
			JS_JW();

			for (i = 0; i < kPK; i++)
				imov(rJ[kIV][i], rJS[i]);
		} /* next kX */

		if (kIV == 0 || (kVK & 7) == 1 || (kVK & 7) == 3)
			continue;

		for (i = 0; i < kPK; i++)
			imovk(rJW[i], 0);

		for (i = 0; i < kPK; i++)
			imovk(rJS[i], 0);

		for (i = 0; i < kPM; i++)
			imov(rJW[i], rJ2[i]);

		for (i = 0; i < kPM; i++)
			imov(rJS[i], rJ[kIV][i]);

		JS_JW();

		for (i = 0; i < kPM; i++)
			imov(rJ[kIV][i], rJS[i]);
	} /* next kIV */

	return;
}

/* J(0,p,q)^u*J(v,p,q) */
void CalcJPQ0uJPQv(void)
{
	static long i;
	static long j;

	for (i = 0; i < kPK; i++)
		imovk(rJS[i], 0);

	for (i = 0; i < kPL; i++)
		imov(rJS[i], rJ[0][i]);

	imov(rE, rN);
	idivk(rE, kPK);

	JS_E();
	get_APRCL_res20();

	for (i = 0; i < kPK; i++)
		imovk(rJW[i], 0);

	for (i = 0; i < kPL; i++)
	{
		for (j = 0; j < kPL; j++)
		{
			kS = (i + j) % kPK;

			imov(rx, rJS[i]);
			imul(rx, rJ[1][j]);

			iadd(rJW[kS], rx);
			imod(rJW[kS], rN);
		}
	}

	NormalizeJW();

	for (i = 0; i < kPL; i++)
		imod(rJW[i], rN);
}

void PrepareJacobis(void)
{
	static long i;

	for (i = 0; i < kPK; i++)
		imovk(rJ0[i], 0);

	for (i = 0; i < kPK; i++)
		imovk(rJ1[i], 0);

	if (kP > 2)
	{
		JacobiSum(1, 1);
		return;
	}

	if (kK == 1)
		return;

	JacobiSum(1, 1);

	for (i = 0; i < kPK; i++)
		imovk(rJW[i], 0);

	if (kK == 2)
		return;

	for (i = 0; i < kPM; i++)
		imov(rJW[i], rJ0[i]);

	JacobiSum(2, 1);

	for (i = 0; i < kPM; i++)
		imov(rJS[i], rJ0[i]);

	JS_JW();
	NormalizeJS();

	for (i = 0; i < kPM; i++)
		imov(rJ1[i], rJS[i]);

	JacobiSum(3 * wexp(2, kK - 3), wexp(2, kK - 3));

	for (i = 0; i < kPK; i++)
		imovk(rJW[i], 0);

	for (i = 0; i < kPM; i++)
		imov(rJS[i], rJ0[i]);

	JS_2();
	NormalizeJS();

	for (i = 0; i < kPM; i++)
		imov(rJ2[i], rJS[i]);
}

void GetQsGs(long t)
{
	static long i;
	static long q;

	for (i = 2; i <= kNQall; i++)
	{
		q = vQmem[i];

		if (q != 0 && t % (q - 1) == 0)
		{
			kTestingQs++;

			vQ[kTestingQs] = q;
			vG[kTestingQs] = Genshi(q);

			vQmem[i] = 0;
		}
	}
}

void GetAllQs(long t)
{
	static long i;
	static long q;

	for (i = 2; ; i++)
	{
		q = pr_tab[i];

		if (t % (q - 1) == 0)
		{
			kNQall++;
			vQmem[kNQall] = q;
		}

		if (q >= kQmax)
			break;
	}
}

long GetPrimesToTest(void)
{
	for (kI = 1; kI <= kLEVELmax; kI++)
	{
		imovk(rS, 1);

		for (kJ = 1; kJ <= vNQ[kI]; kJ++)
		{
			if (test_sufficient(vQ[kJ], vT[kI], 1)) /* Try H > N */
			{
				kLEVELnow = kI;
				kTestingQs = kJ;

				kT = vT[kLEVELnow];
				kNP = vNP[kLEVELnow];

				return 1; /* Fine */
			}
		}
	}

	if (test_sufficient(1, 1, 1000000000)) /* Try 10^9*H > N */
	{
		kLEVELnow = kLEVELmax;
		kTestingQs = vNQ[kLEVELnow];

		kT = vT[kLEVELnow];
		kNP = vNP[kLEVELnow];

		return 1; /* OK, but only just */
	}

	return 0; /* Not enough q's */
}

void GetAllPrimes(void)
{
	static long p;

	for (kI = 1, p = 2; kI <= kNPmax; kI++, p = xnxtprmk(p))
		vP[kI] = p;

	kNQall = 1;
	vQmem[1] = 2;

	kT = kTmax;
	GetAllQs(kT);

	kTestingQs = 1;

	vQ[1] = 2;
	vG[1] = 1;

	vT[1] = 4 * 3; // 2^2 * 3
	vNP[1] = 2;
	GetQsGs(vT[1]);
	vNQ[1] = kTestingQs;

	vT[2] = 4 * 3 * 5; // 2^2 * 3 * 5
	vNP[2] = 3;
	GetQsGs(vT[2]);
	vNQ[2] = kTestingQs;

	vT[3] = 4 * 9 * 5; // 2^2 * 3^2 * 5
	vNP[3] = 3;
	GetQsGs(vT[3]);
	vNQ[3] = kTestingQs;

	vT[4] = 4 * 9 * 5 * 7; // 2^2 * 3^2 * 5 * 7
	vNP[4] = 4;
	GetQsGs(vT[4]);
	vNQ[4] = kTestingQs;

	vT[5] = 8 * 9 * 5 * 7; // 2^3 * 3^2 * 5 * 7
	vNP[5] = 4;
	GetQsGs(vT[5]);
	vNQ[5] = kTestingQs;

	vT[6] = 16 * 9 * 5 * 7; // 2^4 * 3^2 * 5 * 7
	vNP[6] = 4;
	GetQsGs(vT[6]);
	vNQ[6] = kTestingQs;

	vT[7] = 16 * 27 * 5 * 7; // 2^4 * 3^3 * 5 * 7
	vNP[7] = 4;
	GetQsGs(vT[7]);
	vNQ[7] = kTestingQs;

	vT[8] = 16 * 27 * 5 * 7 * 11; // 2^4 * 3^3 * 5 * 7 * 11
	vNP[8] = 5;
	GetQsGs(vT[8]);
	vNQ[8] = kTestingQs;

	vT[9] = 16 * 27 * 25 * 7 * 11; // 2^4 * 3^3 * 5^2 * 7 * 11
	vNP[9] = 5;
	GetQsGs(vT[9]);
	vNQ[9] = kTestingQs;

	vT[10] = 32 * 27 * 25 * 7 * 11; // 2^5 * 3^3 * 5^2 * 7 * 11
	vNP[10] = 5;
	GetQsGs(vT[10]);
	vNQ[10] = kTestingQs;

	vT[11] = 32 * 27 * 25 * 7 * 11 * 13; // 2^5 * 3^3 * 5^2 * 7 * 11 * 13
	vNP[11] = 6;
	GetQsGs(vT[11]);
	vNQ[11] = kTestingQs;

	vT[12] = 64 * 27 * 25 * 7 * 11 * 13; // 2^6 * 3^3 * 5^2 * 7 * 11 * 13
	vNP[12] = 6;
	GetQsGs(vT[12]);
	vNQ[12] = kTestingQs;

	vT[13] = 64 * 27 * 25 * 7 * 11 * 13 * 17; // 2^6 * 3^3 * 5^2 * 7 * 11 * 13 * 17
	vNP[13] = 7;
	GetQsGs(vT[13]);
	vNQ[13] = kTestingQs;
}

/* Set L_p condition and say that we have done so. */
void set_aprcl_Lp(void)
{
	aprcl_Lp = 1;
	lprint("APRCL L_%d condition satisfied\n", kP);
}

void aprcl_main_init(void)
{
	static long i;

	utrim(rN);

	if (rN->digits <= 87)
	{
		/* For N up to 838 digits at level 11 (same as UBASIC APRT-CLE) */
		kNPmax = 6;
		kPWmax = 32;
		kQmax = 30241;
		kLEVELmax = 11;
		kTmax = 32 * 27 * 25 * 7 * 11 * 13;
	}
	else if (rN->digits <= 173)
	{
		/* For N up to 1672 digits at level 12 */
		kNPmax = 6;
		kPWmax = 64;
		kQmax = 4324321;
		kLEVELmax = 12;
		kTmax = 64 * 27 * 25 * 7 * 11 * 13;
	}
	else
	{
		/* For N up to 3317 digits at level 13 */
		kNPmax = 7;
		kPWmax = 64;
		kQmax = 6283201;
		kLEVELmax = 13;
		kTmax = 64 * 27 * 25 * 7 * 11 * 13 * 17;
	}

	for (i = 0; i < kPWmax; i++)
	{
		imovk(rJ0[i], 0);
		imovk(rJ1[i], 0);
		imovk(rJ2[i], 0);

		imovk(rJW[i], 0);
		imovk(rJS[i], 0);
		imovk(rJX[i], 0);

		imovk(rJ[0][i], 0);
		imovk(rJ[1][i], 0);
	}
}

/*
Function:

Perform the main part of the APRCL primality test.

The test is based on Theorem 9.1.12 of H. Cohen, CCANT.

If Theorem 9.1.12 holds for N, if S^2 > N, and if for every
i = 1, 2, ..., T, we have that (N^i mod S) is not a non-trivial
divisor of N, then N is prime.

Return value

-1 : The test did not perform any meaningful function.

0 : The number is not prime. If a divisor is known, it is
returned in rW.

2 : The number is too large for this APRCL test. The test
is abandoned. The values of S and T that would have
been used are returned in rS and kT, respectively.

3 : The number has passed the main part of the APRCL test.
The values of S and T used by the program are returned
in rS and kT, respectively.
*/
long aprcl_main_test(void)
{
	aprcl_main_init();
	GetAllPrimes();

	if (!GetPrimesToTest())
		return 2; /* Too large */

	lprint("APRCL test\n");
	lprint("T=%d\n", kT);
	iprtcr("S=", rS);

	/*  Main test */
MainStart:
	kI = 1;

	if (restart == 2)
	{
		if (!(icmp(rS, restart_s) == 0 && kT == restart_t))
		{
			iprtcr("Error in restarting APRCL main test: Computed T=%d S=", rS, kT);

			gen_fail();
			return -1;
		}

		kI = restart_i;

		lprint("Restarting APRCL main test at (%d %d)\n", restart_i, restart_j);
	}
	for (; kI <= kNP; kI++)
	{
		kP = vP[kI];
		aprcl_Lp = 0;
		kTestedQs = 0;

		lprint("APRCL main test (%d) at level %d for p=%d\n", kI, kLEVELnow, kP);

		if (kP > 2)
		{
			imovk(rx, kP - 1);
			imovk(ry, kP * kP);
			iexpmod(rz, rN, rx, ry);

			if (icmp(rz, r1) != 0)
				set_aprcl_Lp(); /* N^(P-1) mod P^2 not = 1 */
		}

		kJ = 1;

		if (restart == 2)
		{
			if (kI == restart_i)
			{
				kJ = restart_j;
				aprcl_Lp |= restart_k;
			}
		}

	lab530:
		for (; kJ <= kTestingQs; kJ++)
		{
			kQ = vQ[kJ];
			kG = vG[kJ];

			kQM = kQ - 1;
			kQH = kQM >> 1;

			kK = PowerIndex(kP, kQM);

			if (kK == 0)
				continue; /* P does not divide Q - 1 */

			if (aprcl_Lp) /* Check L_p condition */
			{
				imov(rx, rS); /* If L_P is OK then we do not have */
				imodk(rx, kQ); /* to do q's that do not divide S */

				if (isgn(rx))
				{
					lprint("APRCL main test (%d %d) for p=%d q=%d not needed\n", kI, kJ, kP, kQ);
					continue;
				}
			}

			psecs = difftime(time(NULL), ptime);

			if (test_quit() || psecs > 1800.0)
			{
				save_parameters(2, rS, kT, kI, kJ, aprcl_Lp);

				if (quit)
					halt();
			}

			kPK = wexp(kP, kK);
			kPL = (kP - 1) * wexp(kP, kK - 1);
			kPM = wexp(kP, kK - 1);

			GetIndexTable();
			PrepareJacobis();

			CalcJacobiPQVs(); /* J(v,p,q) */
			CalcJPQ0uJPQv(); /* J(0,p,q)^u*J(v,p,q) */

			kHj = unity_root_J(); /* J = zeta^Hj% */

			if (journal)
				jprint_aprcl_main(650);

			if (kHj == -1)
			{
				lprint("APRCL Fail: Result for p=%d q=%d k=%d g=%d is not a power of zeta_%d\n",
				       kP, kQ, kK, kG, kPK);

				return 0; /* Composite */
			}

			kHc = unity_root_C(); /* J = zeta^Hc% */

			if (kHj != kHc)
			{
				lprint("APRCL Fail: zeta_%d exponent Hj=%d for p=%d q=%d k=%d g=%d is not equal to Hc=%d\n",
				       kPK, kHj, kP, kQ, kK, kG, kHc);

				if (!aprcl_Lp)
					return 0; /* Composite if L_p not set */

				gen_fail();
				return -1;
			}

			if (aprcl_Lp)
				goto Aprcl_p_q_done;

			if (kHj % kP == 0)
				goto Aprcl_p_q_done;

			if (kP != 2)
			{
				set_aprcl_Lp();
				goto Aprcl_p_q_done;
			}

			if (kK == 1)
			{
				if ((iwd0(rN) & 3) == 1)
					set_aprcl_Lp(); /* N = 1 (mod 4) */

				goto Aprcl_p_q_done;
			}

			imovk(rx, kQ);
			imov(ry, rNm1);
			idiv2k(ry, 1); /* y = (N - 1)/2 */

			iexpmod(rz, rx, ry, rN); /* z = Q^((N - 1)/2) (mod N) */

			if (icmp(rz, rNm1) != 0) /* Q^((N - 1)/2) != N - 1 (mod N) */
			{
				get_res20(rz);
				iprtcr("APRCL Fail: %d^((N - 1)/2) != -1 (mod N): R20=", res20, kQ);
				return 0; /* Composite */
			}

			set_aprcl_Lp();

		Aprcl_p_q_done:
			iprtcr("APRCL main test (%d %d) done: p=%d q=%d k=%d g=%d h=%d R20=",
			       res20, kI, kJ, kP, kQ, kK, kG, kHj);
		} /* next J */

		if (aprcl_Lp == 0)
		{
			save_parameters(0, rS, kT, 0, 0, 0); /* Disable restart */
			kTestedQs = kTestingQs;

			/* Test more Qs */
			if (kTestingQs < vNQ[kLEVELnow])
			{
				lprint("Retry\n");
				kTestingQs++;

				kQ = vQ[kTestingQs];
				kJ = kTestedQs + 1;

				goto lab530;
			}

			lprint("Retry from the beginning\n");

			if (kLEVELnow == kLEVELmax)
				return 2; /* Give up if we are at the top level */

			kLEVELnow++;

			kT = vT[kLEVELnow];
			kNP = vNP[kLEVELnow];

			imovk(rS, 1);

			for (kJ = 1; kJ <= vNQ[kLEVELnow]; kJ++)
			{
				if (test_sufficient(vQ[kJ], kT, 1))
				{
					kTestingQs = kJ;
					goto MainStart;
				}
			}

			lprint("Error: We have fallen out of the 'Retry from the beginning' loop!\n");

			gen_fail();
			return -1;
		}

		lprint("APRCL tests for p=%d completed\n", kP);

		if (single_segment)
			return 2;
	} /* next I */

	return 3; /* The APRCL main test was successful */
}

/*
The APRCL main test is finished. Let W be a divisor of N. Then

W == N^i (mod S) for some i, 0 <= i <= T.
*/
long aprcl_divisor_test(void)
{
	static long i;

	iprtcr("APRCL divisor test\nT=%d\nS=", rS, kT);

	i = 1;

	if (restart == 3)
	{
		i = restart_i;
		lprint("Restarting APRCL divisor test at %d/%d\n", i, kT);
	}

	imovk(rx, i - 1);
	imov(rNmS, rN);
	imod(rNmS, rS);
	iexpmod(rW, rNmS, rx, rS);

	for (; i <= kT; i++)
	{
		psecs = difftime(time(NULL), ptime);

		if (test_quit() || psecs > 1800.0)
		{
			save_parameters(3, rS, kT, i, 0, 0);

			if (quit)
				halt();
		}

		imul(rW, rNmS);
		imod(rW, rS);

		if (i < 3200)
			if (journal)
				jprint("APRCL-D ", rW);

		if ((i % 10000) == 0)
			printf("APRCL divisor test %d/%d\n", i, kT);

		if (icmp(rW, r1) == 0) /* Stop when we reach 1 (mod S) */
		{
			lprint("APRCL divisor test passed: %d/%d\n", i, kT);
			imov(rx, rS);
			isqu(rx);

			if (icmp(rx, rN) > 0)
				return 1; /* If S^2 > N, N is prime */

			return 2; /* Otherwise cannot tell */
		}

		if (icmp(rW, rsqrtN) <= 0) /* Try W if W^2 <= N */
		{
			imov(rx, rN);
			imod(rx, rW);

			if (isgn(rx) == 0)
			{
				if (icmp(rW, rN) < 0)
					return 0; /* Non-trivial divisor found */
			}
		}
	}

	iprtcr("Error: We have fallen out of the 'APRCL divisor test' loop!: N^T mod S = ", rW);

	gen_fail();
	return -1;
}

/*
Double check our CRT computation. We check that 0 < W < G,
W == N^i (mod S), W == 1 (mod F1), W == e (mod F2).
*/
long main_divisor_check(long i, long e)
{
	if (debug)
		iprtcr("Main divisor check: i=%d e=%d W=", rW, i, e);

	imovk(rw1, i);
	iexpmod(rw2, rN, rw1, rS);

	imov(rw3, rW);
	isub(rw3, rw2); /* w3 = W - N^i (mod S) */
	imod(rw3, rS);

	imov(rw1, rW);
	isubk(rw1, 1); /* w1 = W - 1 (mod F1) */
	imod(rw1, rF1);

	imov(rw2, rW);
	isubk(rw2, e); /* w2 = W - e (mod F2) */
	imod(rw2, rF2);

	if (isgn(rw1) || isgn(rw2) || isgn(rw3) ||
		!(icmp(r0, rW) < 0 && icmp(rW, rG) < 0))
	{
		lprint("Error in CRT computation: %d %d %d e=%d\n",
		       isgn(rw1), isgn(rw2), isgn(rw3), e);

		iprtcr(" f=", rf);
		iprtcr(" W=", rW);
		iprtcr(" G=", rG);

		iprtcr(" cF1=", rcF1);
		iprtcr(" cF2p=", rcF2p);

		iprtcr(" cF2m=", rcF2m);
		iprtcr(" cS=", rcS);

		gen_fail();
		return 0;
	}

	return 1;
}

/*
We start with N, F1, F2, S and G; F1 divides N - 1, F2
divides N + 1, F1 and F2 are even, gcd(F1*F2, S) = 1 and
G = F1*F2*S/2.

Divide whichever of F1 and F2 is == 2 (mod 4) by 2. Then
F1, F2 and S are pairwise coprime and G = F1*F2*S.
Let d be a prime divisor of N. Upon arrival here we have

d == 1 (mod F1)                  by Pocklington's Theorem,

d == +/-1 (mod F2)               by Morrison's Theorem,

d == N^i (mod S) for some i < T  by Lenstra's Theorem.

Hence, by the Chinese Remainder Theorem,

d == (1/(F2*S) mod F1)*F2*S + (e/(F1*S) mod F2)*F1*S +
(N^i/(F1*F2) mod S)*F1*F2  (mod G)

for some e = +/-1 and i, 0 < i < T.

Return value

-1 : The test did not perform any meaningful function.

0 : N is composite. A divisor is returned in rW.

1 : N is prime.

3 : The number has passed the main divisor test. However,
more work needs to be done to complete the primality-
proof.
*/
long main_divisor_test(void)
{
	static long i;

	imov(sF1, rF1);
	imov(sF2, rF2);

	if (iwd0(sF1) & 2)
		idiv2k(sF1, 1);
	else
		idiv2k(sF2, 1);

	lprint("Main divisor test: F1=%0.4f F2=%0.4f G=%0.4f S=%0.4f T=%d\n",
	       ilog(sF1) / ilogN, ilog(sF2) / ilogN, ilog(rG) / ilogN, ilog(rS) / ilogN, kT);

	iprtcr("G=", rG);

	imov(rx, sF2);
	imul(rx, rS);

	igcdext(rcF1, rz, rx, sF1);
	imul(rcF1, sF2);
	imul(rcF1, rS);
	imod(rcF1, rG); /* cF1 = (1/(F2*S) mod F1)*F2*S mod G */

	imov(rx, sF1);
	imul(rx, rS); /* x = F1*S */

	igcdext(rcF2p, rz, rx, sF2);
	imov(rcF2m, rcF2p);
	imul(rcF2p, rx);
	imod(rcF2p, rG); /* cF2p = (1/(F1*S) mod F2)*F1*S mod G */

	ineg(rcF2m);
	imod(rcF2m, sF2);
	imul(rcF2m, rx);
	imod(rcF2m, rG); /* cF2m = (-1/(F1*S) mod F2)*F1*S mod G */

	imov(rU, sF1);
	imul(rU, sF2); /* U = F1*F2 */

	igcdext(rcS, rz, rU, rS); /* cS = 1/(F1*F2) mod S */

	if (debug)
	{
		iprtcr("F1=", sF1);
		iprtcr("F2=", sF2);
		iprtcr("S=", rS);
	}

	i = 1;

	if (restart == 4)
	{
		i = restart_i;
		lprint("Restarting main divisor test at %d/%d\n", i, kT);
	}

	imovk(rx, i - 1);
	imov(rNmS, rN);
	imod(rNmS, rS); /* N mod S */

	iexpmod(rf, rNmS, rx, rS); /* Start with f = N^(i-1) (mod S) */

	for (; i <= kT; i++)
	{
		psecs = difftime(time(NULL), ptime);

		if (test_quit() || psecs > 1800.0)
		{
			save_parameters(4, rS, kT, i, 0, 0);

			if (quit)
				halt();
		}

		if (i % 10000 == 0)
			printf("%d/%d\n", i, kT);

		imul(rf, rNmS);
		imod(rf, rS); /* f = N^i (mod S) */

		imov(rx, rcS);
		imul(rx, rf);
		imod(rx, rS); /* x = ((1/F1*F2) mod S)*N^i */

		imul(rx, rU);
		imod(rx, rG); /* x = (N^i/(F1*F2) mod S)*F1*F2 (mod G) */

		iadd(rx, rcF1); /*     + (1/(F2*S) mod F1)*F2*S          */

		imov(rW, rx);
		iadd(rW, rcF2m);
		imod(rW, rG); /* W == 1 (mod F1), -1 (mod F2), N^i (mod S) */

		if (i < 20000)
		{
			if (!main_divisor_check(i, -1))
				return -1;
		}

		if (icmp(rW, rsqrtN) <= 0) /* Try W if W^2 <= N */
		{
			imov(ry, rN);
			imod(ry, rW);

			if (isgn(ry) == 0)
			{
				if (icmp(r1, rW) < 0 && icmp(rW, rN) < 0)
					return 0;
			}
		}

		imov(rW, rx);
		iadd(rW, rcF2p);
		imod(rW, rG); /* W == 1 (mod F1*F2), N^i (mod S) */

		if (i < 20000)
		{
			if (!main_divisor_check(i, 1))
				return -1;
		}

		if (icmp(rW, rsqrtN) <= 0) /* Try W if W^2 <= N */
		{
			imov(ry, rN);
			imod(ry, rW);

			if (isgn(ry) == 0)
			{
				if (icmp(r1, rW) < 0 && icmp(rW, rN) < 0)
					return 0;
			}
		}

		if (icmp(rW, r1) == 0) /* Stop when W = 1 (mod G) */
		{
			lprint("Main divisor test passed: %d/%d\n", i, kT);
			imov(rx, rG);
			isqu(rx); /* x = G^2 */

			if (icmp(rx, rN) >= 0)
				return 1; /* G^2 > N -- Prime! */

			return 3; /* More work required */
		}
	}

	iprtcr("Error: We have dropped out of the bottom of the main divisor test! N^T mod G = ", rW);

	gen_fail();
	return -1;
}

long final_divisor_check(void)
{
	static long t;

	/* Double-check that isqrt() returned the correct value of D. */
	imov(rw3, res); /* Save Dsqu - D^2 from isqrt */

	imov(rw1, rD);
	isqu(rw1); /* w1 = D^2 */

	iadd(rw3, rw1); /* w3 = Dsqu - D^2 + D^2 */

	imov(rw2, rD);
	iaddk(rw2, 1);
	isqu(rw2); /* w2 = (D + 1)^2 */

	if (!(icmp(rw1, rDsqu) <= 0 && /* Check D^2 <= Dsqu < (D+1)^2 */
			icmp(rDsqu, rw2) < 0) ||
		icmp(rw3, rDsqu) != 0 || /* and w3 = Dsqu */
		ieven(rDsqu)) /* and D^2 is odd */
	{
		iprtcr("Error in isqrt: D=", rD);
		iprtcr(" D^2=", rDsqu);

		gen_fail();
		return 0;
	}

	/* Double-check the computation of D^2 */
	if (icmp(rF1, rF2) > 0)
		t = -1;
	else
		t = 1;

	imov(rw1, sR); /* Compute D^2 the hard way */

	imov(rw3, rV);
	imul(rw3, sF); /* w3 = V*F */

	isub(rw1, rw3);
	isqu(rw1); /* w1 = (R - V*F)^2 */

	imov(rw3, rV);
	imulk(rw3, 4 * t);
	iadd(rw1, rw3); /* w1 = (R - V*F2)^2 + 4*t*V */

	if (icmp(rw1, rDsqu) != 0)
	{
		iprint("Error in computation of D^2: ", rw1);
		iprtcr(" not =", rDsqu);

		gen_fail();
		return 0;
	}

	return 1;
}

/*
See if one of |c|*F + 1 or |c|*F - 1 is a non-trivial
divisor of N and if so, put it in rW and return 1.
Otherwise return 0.
*/
long final_divisor_factor(void)
{
	if (isgn(rc) == 0)
		return 0;

	imov(rW, rc);
	iabs(rW); /* Make c >= 0 */
	imul(rW, sF);
	iaddk(rW, 1); /* Try c*F + 1 */
	imov(rw1, rN);
	imod(rw1, rW);

	if (isgn(rw1) == 0)
	{
		if (icmp(r1, rW) < 0 && icmp(rW, rN) < 0)
			return 1;
	}

	isubk(rW, 2); /* Try c*F - 1 */
	imov(rw1, rN);
	imod(rw1, rW);

	if (isgn(rw1) == 0)
	{
		if (icmp(r1, rW) < 0 && icmp(rW, rN) < 0)
			return 1;
	}

	return 0;
}

/*
Let d be a prime divisor of N. When we arrive here we know
that d > G > N^(1/3). However, G < N^(1/2), for otherwise N
would be prime.

We split into cases: If F1 > F2, we use Theorem 1; if F2 > F1,
Theorem 2. We revert to the original (both even) F1 and F2.

1.  Let F = F1, R = (N - 1)/F = 2*F*s + r, 0 < r < 2*F. Let

a = (N/G + G - (r*F + 2))/(2*F^2) + 1

and suppose N = (c*F + 1)*(d*F + 1). By Theorem 1 we have
2*(s - a) < c*d <= 2*s. Now let V = c*d and note that R is
odd. Then

R = c*d*F + c + d = V*F + c + V/c,

and we look for solutions c = (R - V*F +/- D)/2, where
D = sqrt((R - V*F)^2 - 4*V) and 2*(s - a) <= V <= 2*s.

2.  Let F = F2, R = (N + 1)/F = 2*F*s + r, |r| < F. Let

a = (N/G - G + |r*F - 2|)/(2*F^2) + 1

and suppose N = (c*F +/- 1)*(d*F -/+ 1). By Theorem 2 we
have 2*(s - a) < c*d <= 2*(s + a). Now let V = c*d and
note that R is odd. Then

R = c*d*F +/- (c - d) = V*F +/- (c - V/c),

and we look for c = +/-(R - V*F +/- D)/2, where
D = sqrt((R - V*F)^2 + 4*V) and 2*(s - a) <= V <= 2*(s + a).

Return value

-1 : The test did not perform any meaningful function.

0 : N is composite. A divisor is returned in rW.

1 : N is prime.

2 : Cannot tell. (Impossible? - I must check.)
*/
long final_divisor_test(void)
{
	static long a;
	static long h;
	static long t;

	if (icmp(rF1, rF2) > 0)
	{
		t = -1;
		sF = rF1;
		sR = rR1;
	} /* t = -1, F -> F1, R -> R1 */
	else
	{
		t = 1;
		sF = rF2;
		sR = rR2;
	} /* t =  1, F -> F2, R -> R2 */

	imov(sR, rN);
	iaddk(sR, t);
	idiv(sR, sF); /* R = (N + t)/F */

	if (isgn(res))
	{
		imov(rx, res);
		iprtcr("Error: N + t (mod F) = ", rx);

		gen_fail();
		return -1;
	}

	imov(s2F, sF);
	iadd(s2F, s2F); /* 2*F */

	imov(s2Fsqu, s2F);
	imul(s2Fsqu, sF); /* 2*F^2 */

	imov(sRs, sR);
	idiv(sRs, s2F); /* s = [R/(2*F)] */

	imov(sRr, res); /* r = R mod 2*F */

	if (t == 1 && icmp(sRr, sF) >= 0)
	{
		isub(sRr, s2F); /* Adjust to -F < r < F */
		iaddk(sRs, 1); /* and increment s */
	}

	imov(ra, sRr);
	imul(ra, sF); /* a(t) = r*F */

	if (t == -1)
	{
		iaddk(ra, 2);
		ineg(ra);
		iadd(ra, rG); /* a(-1) = G - (r*F + 2) */
	}
	else
	{
		isubk(ra, 2);
		iabs(ra);
		isub(ra, rG); /* a(1) = |r*F - 2| - G */
	}

	imov(rx, rN);
	idiv(rx, rG);
	iadd(ra, rx); /* a = N/G + a(i)) */

	if (isgn(ra) < 0)
		imovk(ra, 0);

	idiv(ra, s2Fsqu);
	iaddk(ra, 1); /* a = max([N/G + a(i))/(2*F^2)], 0) + 1 */

	iprtcr("Final divisor test: F=%0.4f G=%0.4f H=%0.4f t=%d a=",
	       ra, ilog(sF) / ilogN, ilog(rG) / ilogN, ilog(rH) / ilogN, t);

	imovk(rx, 2000000000);

	if (icmp(ra, rx) > 0)
		return 2; /* Give up if too much to do */

	a = iwd0(ra);

	imov(rV, sRs);
	isubk(rV, a);
	iadd(rV, rV); /* V = 2*(s - a) */

	cd_num = a * (t + 3) + 1; /* 2*a + 1 (t = -1) or 4*a + 1 (t = 1) */

	cd_tot = 0;
	cd_real = 0;
	cd_int = 0;

	if (restart == 5)
	{
		if (cd_num != restart_t)
		{
			lprint("Error in restarting final divisor test: cd_num=%d\n", cd_num);

			gen_fail();
			return -1;
		}

		cd_tot = restart_i;
		cd_real = restart_j;
		cd_int = restart_k;

		lprint("Restarting final divisor test at %d/%d\n", cd_tot, cd_num);
	}

	iaddk(rV, cd_tot);

	/*
	Compute D^2 recursively.

	Start with (R - (V0 - 1)*F)^2 + 4*t*(V0 - 1) and add
	U = (2*V - 1)*F^2 - 2*(N - t) which we compute recursively
	by starting with (2*(V0 - 1) - 1)*F^2 - 2(N - t) and adding 2*F^2.
	*/
	imov(rz, rV);
	isubk(rz, 1); /* z = V0 - 1 */

	imov(rU, rz);
	iadd(rU, rU);
	isubk(rU, 1);
	imul(rU, sF);
	imul(rU, sF); /* U = (2*(V0 - 1) - 1)*F^2 */

	imov(rx, rN);
	isubk(rx, t);
	iadd(rx, rx); /* x = 2*(N - t) */

	isub(rU, rx); /* U = (2*(V0 - 1) - 1)*F^2 - 2(N - t) */

	imov(rDsqu, sR);
	imov(rx, rz);
	imul(rx, sF); /* x = (V0 - 1)*F */

	isub(rDsqu, rx);
	isqu(rDsqu); /* D^2 = (R - (V0 - 1))^2 */

	imov(rx, rz);
	imulk(rx, 4 * t);
	iadd(rDsqu, rx); /* D^2 = (R - (V0 - 1)*F)^2 + 4*t*(V0 - 1) */

	for (; cd_tot < cd_num; cd_tot++, iaddk(rV, 1))
	{
		psecs = difftime(time(NULL), ptime);

		if (test_quit() || psecs > 1800.0)
		{
			save_parameters(5, rS, cd_num, cd_tot, cd_real, cd_int);

			if (quit)
				halt();
		}

		if (cd_tot % 10000 == 0 && cd_tot > 0)
		{
			printf("%d/%d r=%d i=%d\n", cd_tot, cd_num, cd_real, cd_int);
		}

		iadd(rU, s2Fsqu); /* U += 2*F^2 */

		iadd(rDsqu, rU); /* Dsqu += U */

		if (isgn(rDsqu) < 0)
			continue; /* D is imaginary */

		cd_real++;
		h = prob_square(rDsqu);

		if (!h && !debug)
			continue; /* D is irrational */

		isqrt(rD, rDsqu); /* D = [Dsqu^(1/2)] */

		if (isgn(res) && !debug)
			continue; /* D is irrational */

		if (!final_divisor_check())
			return -1;

		cd_int++;

		imov(rx, rV);
		imul(rx, sF); /* x = V*F */

		imov(rc, sR);
		isub(rc, rx);
		isub(rc, rD);
		idiv2k(rc, 1); /* c = (R - V*F - D)\2 */

		if (debug)
		{
			iprtcr("F=", sF);
			iprtcr("s=", sRs);
			iprtcr("r=", sRr);
			iprtcr("G=", rG);
			iprtcr("a=%d=", ra, a);
			iprtcr("V=", rV);
			iprtcr("D=", rD);
			iprtcr("c=", rc);
		}
		if (final_divisor_factor())
			return 0;

		iadd(rc, rD); /* c = (R - V*F + D)\2 */

		if (final_divisor_factor())
			return 0;
	}

	lprint("Final divisor test passed: %d/%d r=%d i=%d\n", cd_tot, cd_num, cd_real, cd_int);

	return 1;
}

long preliminary_test(void)
{
	static long i;

	if (icmp(rN, r1) <= 0)
	{
		printf("Silly!\n");
		return -1;
	}

	if (debug)
		return 3;

	imovk(rW, iprmdiv(rN)); /* W is a prime divisor of N */

	if (icmp(rW, rN) == 0)
		return 1; /* If W = N, N is prime */

	if (icmp(rW, r1) > 0)
		return 0; /* If W > 1, N is composite */

	imovk(rx, prmdiv_hi);
	isqu(rx);

	if (icmp(rN, rx) < 0)
		return 1; /* If N < (prmdiv_hi)^2, N is prime */

	for (i = 1; i < kNQdim; i++) /* Check APRCL q's as well */
	{
		imovk(rW, pr_tab[i]);
		imov(rx, rN);
		imod(rx, rW);

		if (isgn(rx))
			continue;

		if (icmp(rW, rN) == 0)
			return 1; /* If W = N, N is prime */

		return 0; /* If W|N, N is composite */
	}

	if (journal || debug)
		return 3;

	imovk(rW, 3);
	iexpmod(ry, rW, rNm1, rN);

	if (icmp(ry, r1) != 0) /* 3^(N-1) not = 1 (mod N) */
	{
		get_res20(ry);
		iprtcr("Fail: 3^(N-1) not = 1 (mod N): R20=", res20);

		return 0;
	}

	return 3;
}

long vfypr_init(char * exp, char * parameters)
{
	static long i;

	imovk(r0, 0);
	imovk(r1, 1);

	imovk(rF1, 1);
	imovk(rF2, 1);

	imovk(rS, 1);

	kT = 1;

	ilog2 = log(2);
	ilog10 = log(10);

	F_max = 100000;

	i = instr(parameters, 'F');

	if (i >= 0)
		F_max = iparm(parameters + i + 1, 2, 2000000000);

	S_min = 100000;

	i = instr(parameters, 'S');

	if (i >= 0)
		S_min = iparm(parameters + i + 1, 1, 2000000000);

	halt_not_prime = 0;

	if (instr(parameters, 'h') >= 0)
		halt_not_prime = 1;

	aprcl_only = 0;

	if (instr(parameters, 'a') >= 0)
		aprcl_only = 1;

	journal = 0;

	if (instr(parameters, 'J') >= 0)
		journal = 1;

	single_segment = 0;

	if (instr(parameters, 'C') >= 0)
		single_segment = 1;

	debug = 0;

	if (instr(parameters, 'D') >= 0)
		debug = 1;

	ttime = time(NULL);

	lprint("*** VFYPR %s  F_max=%d S_min=%d h=%d a=%d C=%d J=%d D=%d \n",
	       version, F_max, S_min, halt_not_prime, aprcl_only, single_segment, journal, debug);

	restart = 0;

	if (instr(parameters, 'r') >= 0)
	{
		ini_file = fopen("vfypr.ini", "r");

		if (ini_file == NULL)
		{
			lprint("Error: No restart file\n");
			return -1;
		}

		fscanf(ini_file, "%d ", &restart);

		fscanf(ini_file, "%s ", buffer);
		imovd(rN, buffer);

		fscanf(ini_file, "%s ", buffer);
		imovd(rF1, buffer);

		fscanf(ini_file, "%s ", buffer);
		imovd(rF2, buffer);

		fscanf(ini_file, "%s %d %d %d %d",
		       buffer, &restart_t, &restart_i, &restart_j, &restart_k);
		imovd(restart_s, buffer);

		fclose(ini_file);

		if (!(restart >= 1 && restart <= 5))
		{
			lprint("Error: No unfinished work\n");
			return -1;
		}

		if (!(icmp(rN, r1) > 0 && icmp(rF1, r1) >= 0 && icmp(rF2, r1) >= 0))
		{
			lprint("Error in restart file\n");
			return -1;
		}

		if (restart >= 3)
		{
			imov(rS, restart_s);
			kT = restart_t;
		}

		if (icmp(rF1, r1) == 0)
			aprcl_only = 1;

		if (aprcl_only == 1 && icmp(rF1, r1) != 0 && icmp(rF2, r1) != 0 &&
			!(restart == 2 || restart == 3))
		{
			lprint("Error in restart file\n");
			return -1;
		}

		iprtcr("N=", rN);
	}
	else
	{
		lprint("N=%s", buffer);
		imov(rN, ieval(exp));

		if (*eval_err != 0)
		{
			lprint("%s\n", eval_err);
			return -1;
		}

		iabs(rN);

		if (eval_exp)
			iprtcr("N=", rN);

		if (journal)
			jprint("N ", rN);
	}

	imov(rNm1, rN);
	isubk(rNm1, 1); /* N - 1 */

	ilogN = ilog(rN); /* [log(N)] */

	isqrt(rsqrtN, rN); /* [N^(1/2)] */

	ptime = time(NULL);
	pcount = 0;

	if (debug)
		expmod_Lucas_check();

	if (debug)
		arithmetic_check();

	return 3;
}

long vfypr(char* exp, char* parameters)
{
	static long r;

	r = vfypr_init(exp, parameters);

	if (r == -1)
		goto Abandoned;

	/* Get the silly cases out of the way first. */
	r = preliminary_test();

	if (r == 0)
		goto Composite;

	if (r == 1)
		goto Prime;

	if (r == -1)
		goto Abandoned;

	test_sufficient(1, 1, 1);

	if (restart == 2)
		goto AprclMainTest;

	if (restart == 3)
		goto AprclDivisorTest;

	if (restart == 4)
		goto MainDivisorTest;

	if (restart == 5)
		goto FinalDivisorTest;

	if (aprcl_only)
		goto AprclMainTest;

	/* Attempt to factorize N - 1 and N + 1 */
	r = BLS_test();

	if (r == 0)
		goto Composite;

	if (r == 1)
		goto Prime;

	if (r == 2)
		goto CannotTell;

	if (r == -1)
		goto Abandoned;

	if (test_sufficient(1, 1, S_min))
		goto MainDivisorTest;

	/* Invoke the APRCL main test if we do not have enough factors */
AprclMainTest:
	r = aprcl_main_test();

	if (r == 0)
		goto Composite;

	if (r == 2)
		goto CannotTell;

	if (r == -1)
		goto Abandoned;

	if (!aprcl_only)
		goto MainDivisorTest;

AprclDivisorTest:
	r = aprcl_divisor_test();

	if (r == 0)
		goto Composite;

	if (r == 1)
		goto MaybePrime;

	if (r == 2)
		goto CannotTell;

	goto Abandoned;

MainDivisorTest:
	r = main_divisor_test();

	if (r == 0)
		goto Composite;

	if (r == 1)
		goto MaybePrime;

	if (r == -1)
		goto Abandoned;

FinalDivisorTest:
	r = final_divisor_test();

	if (r == 0)
		goto Composite;

	if (r == 1)
		goto MaybePrime;

	if (r == 2)
		goto CannotTell;

	goto Abandoned;

	/* Test abandoned */
Abandoned:
	lprint("*** Test abandoned\n");
	goto GoBack;

	/* N is prime */
MaybePrime:
	if (single_segment)
		goto CannotTell;

Prime:
	lprint("*** N is prime!\n");
	goto GoBack;

	/* N is composite. Possibly W is a divisor. */
Composite:
	if (icmp(r1, rW) < 0 && icmp(rW, rN) < 0)
	{
		imov(rx, rN);
		imod(rx, rW);

		if (isgn(rx) == 0)
		{
			iprtcr("*** N is divisible by ", rW);
			goto GoBack;
		}
	}

	lprint("*** N is composite\n");
	goto GoBack;

	/* Cannot tell */
CannotTell: r = 2;
	lprint("*** Unable to determine whether N is prime or composite\n");

GoBack:
	restart = 0;

	if (r >= 0)
		save_parameters(0, rS, kT, 0, 0, r);

	tsecs = difftime(time(NULL), ttime);

	lprint("Time: %0.0f sec\n", tsecs);

	if (jnl_file != NULL)
	{
		fclose(jnl_file);
		jnl_file = NULL;
	}

	if (dat_file != NULL)
	{
		fclose(dat_file);
		dat_file = NULL;
	}

	if (r != 1 && halt_not_prime)
		halt();

	return r;
}

int main(int argc, char**argv)
{
	static char* parameters;
	static char argk[256];

	inp_file = NULL;
	dat_file = NULL;
	jnl_file = NULL;
	ini_file = NULL;
	prm_file = NULL;
	fac_file = NULL;

	if (argc > 1)
		parameters = argv[1];
	else
	{
		printf("Enter optional parameters\n");
		printf("  r : Restart\n");
		printf("  k : Input from keyboard\n");
		printf("  a : APRCL only\n");
		printf("  h : Halt if not prime\n");
		printf("  C : Do current segment only\n");
		printf("  F# : Limit for small factors of N^2 - 1\n");
		printf("  S# : Minimum value of S for APRCL test\n");

		fgets(argk, 256, stdin);
		parameters = argk;
	}

	if (instr(parameters, 'X') >= 0)
		halt();

	if (instr(parameters, 'r') >= 0)
	{
		vfypr("", parameters);
		goto AllDone;
	}

	if (instr(parameters, 'k') < 0 && (inp_file = fopen("vfypr.inp", "r")) != NULL)
	{
		while (fgets(buffer, buffer_dim, inp_file) != NULL)
		{
			if (*buffer == '0')
				goto AllDone;

			vfypr(buffer, parameters);
		}

		fclose(inp_file);
		goto AllDone;
	}

	for (;;) /* Get input from keyboard */
	{
		printf("Enter N: ");
		fgets(buffer, buffer_dim, stdin);

		if (*buffer == '0')
			goto AllDone;

		vfypr(buffer, parameters);
	}

AllDone:
	printf("All done\n");

	_fcloseall();
	exit(0);
}
