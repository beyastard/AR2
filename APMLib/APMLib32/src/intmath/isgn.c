#include "intmath.h"

// sgn(a) = -1 if a < 0, 0 if a = 0, 1 if a > 0
long isgn(ireg* a)
{
	if (a->value == NULL)
		iinvparm("isgn");

	if (a->digits < 1)
		iinvparm("isgn");

	if (!xsig(a->value, a->digits))
		return 0;

	if (a->flags & NegativeReg)
		return -1;

	return 1;
}
