#include <assert.h>

#include "regmem.h"

// Split a into two parts, a and b, at 2^k
// 
// |a| =  (|a| mod 2^k), |b| = [|a| / 2^k]
void usep2k(ireg* a, ireg* b, long k)
{
	static long db;

	db = a->digits - (k >> 5); // quotient length

	if (db > 0)
	{
		if (b->value == NULL)
			ualloc(b, db, db);

		if (b->digits < db)
			uextend(b, db);

		xsep2k(a->value, b->value, k, a->digits);
		b->digits = db;
	}
	else
	{
		if (b->value == NULL)
			ualloc(b, 8, 1);

		assert(b->value != NULL);
		b->value[0] = 0;
		b->digits = 1;
	}

	b->flags = a->flags;
}
