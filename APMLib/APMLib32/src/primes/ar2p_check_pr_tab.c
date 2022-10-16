#include "primes.h"

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
