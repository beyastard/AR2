#ifndef __AR_2_T_H__
#define __AR_2_T_H__

/* CheckLog   0 = log failures only, 1 = log everything */
/* CheckStop  1 = stop if fail, 2 = stop anyway         */
/* CheckDiag  1 = activate tdiag routine                */
#define CheckLog  1
#define CheckStop 1
#define CheckDiag 1

void tcheckr(ireg* a, ireg* b, char* id, long ik);
void tcheckd(ireg* a, char* c, char* id, long ik);
void tdiag(ireg* a, long i, long j, long k);

void tsqusch(ireg* a);
void tsqufft(ireg* a);
void tmulsch(ireg* a, ireg* b);
void tmulfft(ireg* a, ireg* b);
void tsqumulsch(ireg* a);
void tsqumulfft(ireg* a);

void tmodm2ke(ireg* a, ireg* q, long m, long k, ireg* e);
void texpmodm2ke(ireg* a, ireg* b, ireg* c, ireg* q, long m, long k, ireg* e);

void tprint(char* s, ...);
void tidisp(ireg* a);
long test_quit(void);
void halt(void);

#endif
