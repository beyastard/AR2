#ifndef __WMOD_H__
#define __WMOD_H__

unsigned long sv_u;
unsigned long sv_y;

unsigned long Wbx;
unsigned long Wdx;

extern long wmod(long a, long b);               // a (mod b) unsigned a
extern long wmods(long a, long b);              // a (mod b) signed a

extern long wmulmod(long a, long b, long c);
extern long wexpmod(long a, long b, long c);
extern long winvmod(long a, long b);

extern long wexp(long a, long b);
extern long wlen(long a);

#endif
