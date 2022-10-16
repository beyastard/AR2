#include "eval.h"

void eval_compute(long key)
{
	static long k;

	while (eval_ptr > 0 && *eval_err == 0)
	{
		if (_kbhit())
			eval_break();

		if ((eval_lev + key) / 4 > eval_ostack[eval_ptr - 1] / 4)
			break;

		k = eval_ostack[eval_ptr - 1] % eval_stp;

		if (k == 12)
			iadd(eval_astack[eval_ptr - 1], eval_astack[eval_ptr]);
		else if (k == 13)
			isub(eval_astack[eval_ptr - 1], eval_astack[eval_ptr]);
		else if (k == 16)
			imul(eval_astack[eval_ptr - 1], eval_astack[eval_ptr]);
		else if (k == 17)
		{
			if (isgn(eval_astack[eval_ptr]) == 0)
				eval_err = "Divide by zero";
			else
			{
				if (isgn(eval_astack[eval_ptr]) < 0)
				{
					ineg(eval_astack[eval_ptr - 1]);
					ineg(eval_astack[eval_ptr]);
				}

				idiv(eval_astack[eval_ptr - 1], eval_astack[eval_ptr]);
			}
		}
		else if (k == 18)
		{
			if (isgn(eval_astack[eval_ptr]) == 0)
				eval_err = "Divide by zero";
			else
			{
				if (isgn(eval_astack[eval_ptr]) < 0)
				{
					ineg(eval_astack[eval_ptr - 1]);
					ineg(eval_astack[eval_ptr]);
				}

				imod(eval_astack[eval_ptr - 1], eval_astack[eval_ptr]);
			}
		}
		else if (k == 20)
		{
			if (isgn(eval_astack[eval_ptr]) <= 0)
				imovk(eval_astack[eval_ptr - 1], 1);
			else
			{
				imov(temp1, eval_astack[eval_ptr - 1]);
				iexp(eval_astack[eval_ptr - 1], temp1, eval_astack[eval_ptr]);
			}
		}
		else if (k == 25)
		{
			imov(eval_astack[eval_ptr - 1], eval_astack[eval_ptr]);
			ineg(eval_astack[eval_ptr - 1]);
		}
		else if (k == 26)
		{
			if (isgn(eval_astack[eval_ptr]) < 0)
				eval_err = "Invalid operand";
			else if (isgn(eval_astack[eval_ptr]) == 0)
				imovk(eval_astack[eval_ptr - 1], 0);
			else
				isqrt(eval_astack[eval_ptr - 1], eval_astack[eval_ptr]);
		}

		if (eval_ptr > 0)
			eval_ptr--;
		else
			eval_err = "Syntax error";
	}
}
