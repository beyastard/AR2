#include "primes.h"

// Return next prime after p
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
		k = iwd0(rx) >> 1; // k = bit address in table
		ar2p_es = ar2p_es_lo + (k >> 5); // es -> word in table
		r = k & 31; // r = bit in word

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
