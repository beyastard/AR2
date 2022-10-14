/* AR2P.C  2.2.1F  TONY FORBES  January 2000 */

#include <stdio.h>
#include <stdlib.h>

#include "ar2x.h"
#include "ar2u.h"
#include "ar2i.h"
#include "ar2p.h"

#define pr_tab_dim 80000
#define es_tab_dim 40000

static unsigned char ar2p_pr_tab[pr_tab_dim];
static unsigned char *ar2p_pr;
static unsigned char *ar2p_pr_lo = ar2p_pr_tab;
static unsigned char *ar2p_pr_hi = ar2p_pr_tab + pr_tab_dim;
static unsigned long ar2p_es_tab[es_tab_dim];
static unsigned long *ar2p_es;
static unsigned long *ar2p_es_lo = ar2p_es_tab;
static unsigned long *ar2p_es_hi = ar2p_es_tab + es_tab_dim;
static ireg ar2p_rp_lo;
static ireg ar2p_rp_hi;
static ireg* ar2p_ep_lo = &ar2p_rp_lo;
static ireg* ar2p_ep_hi = &ar2p_rp_hi;
static long ar2p_f = 0;

/*
Use the Sieve of Eratosthenes to set up es_tab for the
next batch of primes, p_lo <= p < p_lo + 64*w.
*/
void ar2p_get_primes(void)
{
	static unsigned long f;
	static unsigned long k;
	static unsigned long k_lo;
	static unsigned long k_hi;
	static unsigned long q;
	static unsigned long q_hi;

	static ireg rrx;
	static ireg* rx = &rrx;

	k_hi = es_tab_dim << 5;

	imov(ar2p_ep_hi, ar2p_ep_lo);
	iaddk(ar2p_ep_hi, k_hi + k_hi);
	isqrt(rx, ar2p_ep_hi);

	q_hi = iwd0(rx) + 2;
	f = 0;

	for (ar2p_es = ar2p_es_lo; ar2p_es < ar2p_es_hi; ar2p_es++)
		*ar2p_es = 0;

	for (q = 3, ar2p_pr = ar2p_pr_lo; q < q_hi; q += *ar2p_pr, ar2p_pr++)
	{
		imov(rx, ar2p_ep_lo);
		imodk(rx, q);
		k_lo = q - iwd0(rx); /* k_lo = q - p_lo % q */

		if (k_lo == q)
			k_lo = 0;
		else
		{
			if (!(k_lo & 1))
				k_lo >>= 1;
			else
				k_lo = (k_lo + q) >> 1; /* k_lo = -p/2 (mod q) */
		}

		for (k = k_lo; k < k_hi; k += q)
		{
			*(ar2p_es_lo + (k >> 5)) |= (1 << (k & 31));
			f++;
		}
	}

	printf("AR2P: %lu bits set in [", f);
	idisp(ar2p_ep_lo);
	printf(" ");
	idisp(ar2p_ep_hi);
	printf(") with sieve limit %lu \n", q_hi);

	ar2p_f = 1;
}

void ar2p_check_pr_tab(void)
{
	static unsigned long p;
	static unsigned long q;
	static unsigned char* pr;

	printf("AR2P: Checking pr_tab\n");

	for (pr = ar2p_pr_lo, p = 3, q = 3; pr < ar2p_pr_hi; pr++)
	{
		p += *pr;
		q = xnxtprmk(q);

		if (p != q)
		{
			printf("AR2P: Error at %d in pr_tab: %lu should be %ld\n", pr - ar2p_pr_lo, p, q);
			stop();
		}
	}
}

/*
Load pr_tab with Delta(q) for q = 5, 7, 11, ..., 1020401.

H. Cohen, CCANT, page 413: Delta(p) < 256 for p < 1872851947
and Delta(p) < 512 for p < 1999066711391.
*/
void ar2p_load_pr_tab(void)
{
	static unsigned long q;
	static unsigned long q0;
	static unsigned long w;
	static unsigned long r;
	static unsigned char* qr;

	printf("AR2P: Loading pr_tab\n");

	for (qr = ar2p_pr_lo, q0 = 3; q0 < 1609; qr++)
	{
		q = xnxtprmk(q0);
		*qr = q - q0;
		q0 = q;
	}

	/* Use Sieve of Eratosthenes for larger primes. */
	q += 2;
	imovk(ar2p_ep_lo, q);
	ar2p_get_primes();

	for (ar2p_es = ar2p_es_lo; ar2p_es < ar2p_es_hi; ar2p_es++)
	{
		for (r = 0, w = *ar2p_es; r < 32; r++, w >>= 1, q += 2)
		{
			if (!(w & 1))
			{
				*qr = q - q0;
				qr++;

				if (qr >= ar2p_pr_hi)
				{
					printf("AR2P: base=3 p[0]=%d p[1]=%d p[2]=%d ... p[%d]=%lu p[%d]=%lu \n",
					       3 + *ar2p_pr_lo, 3 + *ar2p_pr_lo + *(ar2p_pr_lo + 1),
					       3 + *ar2p_pr_lo + *(ar2p_pr_lo + 1) + *(ar2p_pr_lo + 2),
					       ar2p_pr_hi - ar2p_pr_lo - 2, q0,
					       ar2p_pr_hi - ar2p_pr_lo - 1, q);
					//ar2p_check_pr_tab();
					return;
				}

				q0 = q;
			}
		}
	}
}

/* Return next prime after p */
void inxtprm(ireg* p)
{
	static ireg rrx;
	static ireg* rx = &rrx;

	static unsigned long k;
	static unsigned long r;
	static unsigned long w;

	if (icmpk(p, 1609) <= 0)
	{
		imovk(p, xnxtprmk(iwd0(p)));
		return;
	}

	if (ar2p_f == 0)
		ar2p_load_pr_tab();

	iaddk(p, 1);
	iwd0(p) |= 1;

	for (;;)
	{
		if (icmp(p, ar2p_ep_lo) < 0 || icmp(p, ar2p_ep_hi) >= 0)
		{
			imov(ar2p_ep_lo, p);
			ar2p_get_primes();
		}

		imov(rx, p);
		isub(rx, ar2p_ep_lo);
		k = iwd0(rx) >> 1; /* k = bit address in table */
		ar2p_es = ar2p_es_lo + (k >> 5); /* es -> word in table */
		r = k & 31; /* r = bit in word */

		for (w = *ar2p_es >> r; r < 32; r++, w >>= 1, iaddk(p, 2))
		{
			if (!(w & 1))
				return;
		}

		for (ar2p_es++; ar2p_es < ar2p_es_hi; ar2p_es++)
		{
			for (r = 0, w = *ar2p_es; r < 32; r++, w >>= 1, iaddk(p, 2))
			{
				if (!(w & 1))
					return;
			}
		}
	}
}
