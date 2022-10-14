#ifndef __AR_2_W_H__
#define __AR_2_W_H__

extern long wmod(long a, long b);               /* a (mod b) unsigned a */
extern long wmods(long a, long b);              /* a (mod b) signed a */

extern long wmulmod(long a, long b, long c);
extern long wexpmod(long a, long b, long c);
extern long winvmod(long a, long b);

extern long wexp(long a, long b);
extern long wlen(long a);

#endif
