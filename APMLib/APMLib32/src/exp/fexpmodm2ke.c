#include "exp.h"

// a = (b^c mod q), q = m*2^k + e, using squaring function fsqu
// 
// Assumes b, c >= 0, q = m*2^k + e > 0
void fexpmodm2ke(ireg* a, ireg* b, ireg* c, ireg* q, long m, long k, ireg* e, void (fsqu)(ireg* a))
{
	static long i;
	static long f;
	static unsigned long j;

	if (b->value == NULL)
		iinvparm("fexpmodm2ke");

	if (b->digits < 1)
		iinvparm("fexpmodm2ke");

	if (c->value == NULL)
		iinvparm("fexpmodm2ke");

	if (c->digits < 1)
		iinvparm("fexpmodm2ke");

	if (q->value == NULL)
		iinvparm("fexpmodm2ke");

	if (q->digits < 1)
		iinvparm("fexpmodm2ke");

	if (isgn(q) <= 0)
		iinvparm("fexpmodm2ke");

	imovk(a, 1);

	if (isgn(c) <= 0)
		return;

	f = 0;

	for (i = c->digits - 1; i >= 0; i--)
	{
		for (j = 0x80000000; j >= 1; j >>= 1)
		{
			if (f)
			{
				fsqu(a);
				usep2k(a, res, k);  // a = a_0, res = a_1
				idivk(res, m);      // res = a_11, kres = a_10
				imul(res, e);       // res = e*a_11
				isub(a, res);
				imovk(res, kres);
				imul2k(res, k);     // res = a_10*2^k
				iadd(a, res);
			}

			if (c->value[i] & j)
			{
				imul(a, b);
				usep2k(a, res, k);  // a = a_0, res = a_1
				idivk(res, m);      // res = a_11, kres = a_10
				imul(res, e);       // res = e*a_11
				isub(a, res);
				imovk(res, kres);
				imul2k(res, k);     // res = a_10*2^k
				iadd(a, res);
				f++;
			}

			utrim(a);

			if (a->digits > q->digits + 8)
				imod(a, q);
		}
	}

	imod(a, q);
}
