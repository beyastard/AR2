#include "intmath.h"

// a =  [ a / b], res =  ( a mod k), a >= 0,
// a = -[ a / b], res = -(-a mod k), a <  0;  b > 0
// 
// (b is trimmed by this routine)
void idiv(ireg* a, ireg* b)
{
	if (a->value == NULL)
		iinvparm("idiv");

	if (a->digits < 1)
		iinvparm("idiv");

	if (b->value == NULL)
		iinvparm("idiv");

	if (b->digits < 1)
		iinvparm("idiv");

	if (isgn(b) <= 0)
		iinvparm("idiv");

	utrim(a);
	utrim(b);

	if (b->digits > 1)
	{
		imov(res, a);

		if (res->digits >= b->digits)
		{
			if (res->capacity <= res->digits)
				uextend(res, res->digits + 1);

			xdiv(res->value, b->value, res->digits, b->digits, a->value);
			a->digits = res->digits - b->digits + 1;
			res->digits = b->digits;
		}
		else // d(b) > d(a)
			imovk(a, 0);
	}
	else // b->digits = 1
	{
		imovk(res, 0);
		res->value[0] = xdivk(a->value, b->value[0], a->digits);
		res->flags = a->flags;
	}
}
