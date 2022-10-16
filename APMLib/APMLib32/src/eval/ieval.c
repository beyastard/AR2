#include "eval.h"

// Return the result of evaluating the expression exp.
// 
// Syntax     1  2  3  4  5  6
// 
// 1 (        Y  Y  Y  N  N  N
// 2 prefix   Y  Y  Y  N  N  N
// 3 digit    N  N  Y  Y  Y  Y
// 4 suffix   N  N  N  Y  Y  Y
// 5 binary   Y  Y  Y  N  N  N
// 6 )        N  N  N  Y  Y  Y
ireg *ieval(char *exp)
{
	static char* i;
	static long n;
	static long t;
	static long u;

	eval_astack[0] = &(eval_rstack[0]);
	imovk(eval_astack[eval_ptr], 0);

	eval_lev = eval_stp;
	eval_ptr = 0;
	eval_exp = 0;
	eval_err = "";

	u = 1;

	for (i = exp; *i != 0; i++)
	{
		t = u; // t = last type

		if (*eval_err != 0)
			break;

		if (*i <= ' ')
			continue; // White space

		if (*i == ';')
			break; // Stop at a semicolon

		if (*i < '0' || *i > '9')
			eval_exp = 1; // It's an expression

		u = 0; // u = this type

		if (t == 1 || t == 2 || t == 5)
		{
			u = 1;

			if (*i == '(') // (
			{
				eval_lev += eval_stp;
				continue;
			}

			u = 2;

			if (*i == '+') // Prefix +
			{
				continue;
			}

			if (*i == '-') // Prefix -
			{
				eval_push(25);
				continue;
			}

			if (*i == 'q') // Integer square root
			{
				eval_push(26);
				continue;
			}
		}

		if (t == 1 || t == 2 || t == 3 || t == 5)
		{
			u = 3;

			if ('0' <= *i && *i <= '9') // Digit
			{
				n = *i - 48; // Convert from ASCII
				imulk(eval_astack[eval_ptr], 10);
				iaddk(eval_astack[eval_ptr], n);
				continue;
			}
		}

		if (t == 3 || t == 4 || t == 6)
		{
			u = 4;

			if (*i == '!') // N!
			{
				eval_factorial();
				continue;
			}

			if (*i == '#') // N#
			{
				eval_prime_product();
				continue;
			}

			u = 5;

			if (*i == '+') // Binary +
			{
				eval_push(12);
				continue;
			}

			if (*i == '-') // Binary -
			{
				eval_push(13);
				continue;
			}

			if (*i == '*')
			{
				eval_push(16);
				continue;
			}

			if (*i == '/' || *i == '\\')
			{
				eval_push(17);
				continue;
			}

			if (*i == '@')
			{
				eval_push(18);
				continue;
			}

			if (*i == '^')
			{
				eval_push(20);
				continue;
			}

			u = 6;

			if (*i == ')') /* ) */
			{
				if (eval_lev <= eval_stp)				
					eval_err = "Syntax error";
				
				eval_compute(4);
				eval_lev -= eval_stp;
				continue;
			}
		}

		u = 0;

		eval_err = "Syntax error";
	}

	if (eval_lev > eval_stp)
		eval_err = "Syntax error";

	eval_lev = eval_stp;

	if (*eval_err == 0)
		eval_compute(0);

	if (*eval_err != 0)
		imovk(eval_astack[0], 0);

	return eval_astack[0];
}
