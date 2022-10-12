#ifndef __AR_2_E_H__
#define __AR_2_E_H__

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
