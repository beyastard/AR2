#include "eval.h"

void eval_push(long key)
{
	if (key < 24)
		eval_compute(key);
	else
		eval_compute(eval_stp);

	eval_ostack[eval_ptr] = eval_lev + key;

	if (eval_ptr + 1 >= eval_dim)
	{
		eval_err = "Overflow";
		return;
	}

	eval_ptr++;
	eval_ostack[eval_ptr] = 0;
	eval_astack[eval_ptr] = &(eval_rstack[eval_ptr]);
	imovk(eval_astack[eval_ptr], 0);
}
