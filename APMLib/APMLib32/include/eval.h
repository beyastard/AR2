#ifndef  __EVAL_H__
#define __EVAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "fastmath.h"
#include "regmem.h"
#include "intmath.h"

#define eval_dim 20
#define eval_stp 32

extern ireg  eval_rstack[eval_dim];
extern ireg* eval_astack[eval_dim];
extern long  eval_ostack[eval_dim];

extern long  eval_ptr;
extern long  eval_lev;
extern long  eval_exp;
extern char* eval_err;

void eval_break(void);
void eval_factorial(void);
void eval_prime_product(void);
void eval_compute(long key);
void eval_push(long key);
ireg* ieval(char* exp);

#endif
