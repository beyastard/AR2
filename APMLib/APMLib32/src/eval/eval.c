#include "eval.h"

ireg  eval_rstack[eval_dim];
ireg* eval_astack[eval_dim];
long  eval_ostack[eval_dim];

long  eval_ptr;
long  eval_lev;

long  eval_exp;
char* eval_err;
