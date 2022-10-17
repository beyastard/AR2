#include <assert.h>

#include "intmath.h"

// Return max{log(|a|), 0}.
double ilog(ireg* a)
{
	static double u;
	static double v;
	static double w;

	if (a->value == NULL || a->digits < 1)
		iinvparm("ilog");

	utrim(a);
	assert(a->value != NULL);
	u = a->value[a->digits - 1];

	if (a->digits > 1)
		v = a->value[a->digits - 2];
	else
		v = 0;

	if (u < 0)
		u += 4294967296.0;

	if (v < 0)
		v += 4294967296.0;

	w = u + v / 4294967296.0;

	return w <= 0 ? 0.0 : log(w) + 32.0 * (a->digits - 1) * ilog2;
}
