#include "primes.h"

// Load pr_tab with Delta(q) for q = 5, 7, 11, ..., 1020401.
// 
// H. Cohen, CCANT, page 413: Delta(p) < 256 for p < 1872851947
// and Delta(p) < 512 for p < 1999066711391.
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

	// Use Sieve of Eratosthenes for larger primes.
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
