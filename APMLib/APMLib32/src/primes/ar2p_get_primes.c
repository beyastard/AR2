#include "primes.h"

//Use the Sieve of Eratosthenes to set up es_tab for the
//next batch of primes, p_lo <= p < p_lo + 64*w.
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
		k_lo = q - iwd0(rx); // k_lo = q - p_lo % q

		if (k_lo == q)
			k_lo = 0;
		else
		{
			if (!(k_lo & 1))
				k_lo >>= 1;
			else
				k_lo = (k_lo + q) >> 1; // k_lo = -p/2 (mod q)
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
