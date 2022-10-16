#ifndef __PRIMES_H__
#define __PRIMES_H__

#include <stdio.h>
#include <stdlib.h>

#include "fastmath.h"
#include "regmem.h"
#include "intmath.h"

#define pr_tab_dim 80000
#define es_tab_dim 40000

extern unsigned char  ar2p_pr_tab[pr_tab_dim];
extern unsigned char* ar2p_pr;
extern unsigned char* ar2p_pr_lo;
extern unsigned char* ar2p_pr_hi;

extern unsigned long  ar2p_es_tab[es_tab_dim];
extern unsigned long* ar2p_es;
extern unsigned long* ar2p_es_lo;
extern unsigned long* ar2p_es_hi;

extern ireg  ar2p_rp_lo;
extern ireg  ar2p_rp_hi;

extern ireg *ar2p_ep_lo;
extern ireg *ar2p_ep_hi;

extern long  ar2p_f;

void ar2p_get_primes(void);
void ar2p_check_pr_tab(void);
void ar2p_load_pr_tab(void);
void inxtprm(ireg* p);

#endif
