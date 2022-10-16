#include "intmath.h"

// Return max{log(|a|), 0}.
double ilog(ireg* a)
{
	static double u;
	static double v;
	static double w;

	if (a->value == NULL)
		iinvparm("ilog");

	if (a->digits < 1)
		iinvparm("ilog");

	utrim(a);
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

	if (w <= 0)
		return 0.0;

	return log(w) + 32.0 * (a->digits - 1) * ilog2;
}
