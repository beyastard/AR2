#include "exp.h"

// Fermat test for q = m*2^k + e, using squaring function fsqu
// 
// Assumes m*2^k + e > 0
// Returns 1 if g^q == g (mod q)
// 0 if g^q != g (mod q)
long ffermatm2ke(long m, long k, ireg* e, long g, void (fsqu)(ireg* a))
{
	static ireg ra;
	static ireg rb;
	static ireg rq;

	static ireg* a = &ra;
	static ireg* b = &rb;
	static ireg* q = &rq;

	imovk(q, m);
	imul2k(q, k);
	iadd(q, e);
	imovk(b, g);
	fexpmodm2ke(a, b, q, q, m, k, e, fsqu);

	return icmp(a, b) ? 0 : 1;
}
