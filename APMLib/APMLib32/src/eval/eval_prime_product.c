#include "eval.h"

void eval_prime_product(void)
{
	static unsigned long f;
	static unsigned long i;

	if (isgn(eval_astack[eval_ptr]) <= 0)
	{
		imovk(eval_astack[eval_ptr], 1);
		return;
	}

	utrim(eval_astack[eval_ptr]);

	if (eval_astack[eval_ptr]->digits > 1)
	{
		eval_err = "Too large";
		return;
	}

	f = iwd0(eval_astack[eval_ptr]);
	imovk(eval_astack[eval_ptr], 1);

	for (i = 2; i <= f; i = xnxtprmk(i))
	{
		if (_kbhit())
			eval_break();

		imulk(eval_astack[eval_ptr], i);
	}
}
