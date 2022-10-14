/* AR2U.C  2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>
#include <stdlib.h>

#include "ar2x.h"
#include "ar2u.h"

/*
Allocate array memory for an integer register

a = integer register
c = capacity of integer register in digits
d = initial number of digits
*/
void ualloc(ireg* a, long c, long d)
{
	a->capacity = max(c, 1);
	a->digits = min(max(d, 1), c);
	a->flags = 0;
	a->value = (long*)malloc(sizeof(long) * c);

	if (a->value != NULL)
		xmovz(a->value, a->digits);
	else
	{
		printf("Unable to allocate %ld digits in ualloc\n", a->capacity);
		give_up();
	}
}

/* Free integer array memory */
void ufree(ireg* a)
{
	free(a->value);
	a->value = NULL;
}

/*
Extend an integer register

a = integer register
d = number of digits to extend integer register to
*/
void uextend(ireg* a, long d)
{
	static long* v;

	if (a->value == NULL)
	{
		ualloc(a, d, d);
		return;
	}

	if (d <= a->capacity)
		return;

	/* Allocate new array, copy old into new, free old */
	v = (long*)realloc(a->value, sizeof(long) * d);

	if (v != NULL)
	{
		a->value = v;
		a->capacity = d;
	}
	else
	{
		printf("Unable to allocate %ld extra digits in uextend\n", d - a->capacity);
		give_up();
	}
}

/*
Split a into two parts, a and b, at 2^k

|a| =  (|a| mod 2^k), |b| = [|a| / 2^k]
*/
void usep2k(ireg* a, ireg* b, long k)
{
	static long db;

	db = a->digits - (k >> 5); /* quotient length */

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

		b->value[0] = 0;
		b->digits = 1;
	}

	b->flags = a->flags;
}

/* Unformatted print of an integer register */
void udump(ireg* a)
{
	static long i;

	printf("c=%ld d=%ld f=%ld &v=%ld v=", a->capacity, a->digits, a->flags, (long)a->value);

	for (i = 0; i < a->digits; i++)
		printf("%d ", (unsigned)a->value[i]);

	printf("\n");
}
