#include "primes.h"

unsigned char  ar2p_pr_tab[pr_tab_dim];
unsigned char* ar2p_pr;
unsigned char* ar2p_pr_lo = ar2p_pr_tab;
unsigned char* ar2p_pr_hi = ar2p_pr_tab + pr_tab_dim;

unsigned long  ar2p_es_tab[es_tab_dim];
unsigned long* ar2p_es;
unsigned long* ar2p_es_lo = ar2p_es_tab;
unsigned long* ar2p_es_hi = ar2p_es_tab + es_tab_dim;

ireg  ar2p_rp_lo;
ireg  ar2p_rp_hi;

ireg *ar2p_ep_lo = &ar2p_rp_lo;
ireg *ar2p_ep_hi = &ar2p_rp_hi;

long  ar2p_f = 0;
