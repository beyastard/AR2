/* AR2I.C  2.2.2E  TONY FORBES  October 1999 */
/*
  Integer registers may have leading zeroes.
  NegativeReg bit in ->flags indicates non-positive integer.
  idiv, idivk, idiv2k: sgn(remainder) = sgn(dividend).
  imod, imodk, imod2k: sgn(remainder) >= 0 always.
*/  
#include <stdio.h>;
#include <stdlib.h>;
#include <math.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";

/* Remainder register */
extern ireg rres = {NULL, 0, 0, 0};
extern ireg *res = &rres; 
extern long kres = 0;

/* Temporary registers */
extern ireg rtemp1 = {NULL, 0, 0, 0};
extern ireg rtemp2 = {NULL, 0, 0, 0};
extern ireg rtemp3 = {NULL, 0, 0, 0};
extern ireg rtemp4 = {NULL, 0, 0, 0};
extern ireg rtemp5 = {NULL, 0, 0, 0};
extern ireg *temp1 = &rtemp1; 
extern ireg *temp2 = &rtemp2; 
extern ireg *temp3 = &rtemp3; 
extern ireg *temp4 = &rtemp4; 
extern ireg *temp5 = &rtemp5; 

/* Useful constants */
double ilog2 = 0.69314718055994530941723212145817656807;
double ilog10 = 2.30258509299404568401799145468436420757;

void imov (ireg *a, ireg *b)
  /*
     a = b
  */
  {
  if (b->value == NULL) { iinvparm("imov"); }
  if (b->digits < 1) { iinvparm("imov"); }

  if (a->value == NULL) { ualloc(a, b->digits, b->digits); }
  if (a->capacity < b->digits) { uextend(a, b->digits); }
  xmovf(a->value, b->value, b->digits);
  a->digits = b->digits;
  a->flags = b->flags;
  }

void imovk (ireg *a, long k)
  /*
     a = k, -2^31 < k < 2^31
  */
  {
  if (a->value == NULL) { ualloc(a, 8, 1); }
  if (a->digits < 1) { iinvparm("imovk"); }

  if (k >= 0) { a->value[0] =  k; a->flags = 0; }
  else        { a->value[0] = -k; a->flags = NegativeReg; }
  a->digits = 1;
  }

void imovd (ireg *a, char *c)
  /*
     a = string of decimal digits
  */
  {
  static long i, k, s;
  if (a->value == NULL) { ualloc(a, 8, 1); }
  if (a->digits < 1) { iinvparm("imovd"); }
  
  i = 0;
  s = 0;
  /* see if continuation (_) or minus sign (-) */
  if (c[0] == '_')
    { s = (a->flags & NegativeReg); i++; }
  else
    {
    imovk(a, 0);
    if (c[0] == '-') { s = NegativeReg; i++; }
    }

  while (1)  
    /* Stop at first non-decimal */
    {
    k = c[i] - '0';
    if (k < 0 || k > 9) { break; }
    if (a->capacity <= a->digits) { uextend(a, a->digits + 1); }
    xmulk(a->value, 10, a->digits);
    a->digits++;
    xaddk(a->value, k,  a->digits);
    utrim(a);
    i++;
    }
  a->flags = s;  
  }
  
void iadd (ireg *a, ireg *b)
  /* 
     a = a + b
  */
  {
  static long f;
  if (a->value == NULL) { iinvparm("iadd"); }
  if (b->value == NULL) { iinvparm("iadd"); }
  if (a->digits < 1) { iinvparm("iadd"); }
  if (b->digits < 1) { iinvparm("iadd"); }

  /* Extend a if shorter than b */
  if (a->digits < b->digits)
    {
    if (a->capacity < b->digits) { uextend(a, b->digits); }
    xmovz(&a->value[a->digits], b->digits - a->digits);
    a->digits = b->digits;
    }   

  if ((a->flags & NegativeReg) == (b->flags & NegativeReg))
    {
    /* Same signs */
    f = xadd(a->value, b->value, a->digits, b->digits);
    if (f & CarryFlag) 
      {
      if (a->capacity <= a->digits) { uextend(a, a->digits + 1); }
      a->value[a->digits] = 1;
      a->digits++;
      }
    }
  else
    {
    /* Different signs */
    f = xsub(a->value, b->value, a->digits, b->digits);
    if (f & CarryFlag) 
      {
      xneg(a->value, a->digits);
      a->flags ^= NegativeReg;
      }
    }
  }
  
void iaddk (ireg *a, long k)
  /*
     a = a + k, -2^31 < k < 2^31
  */
  {
  static long f;
  if (a->value == NULL) { iinvparm("iaddk"); }
  if (a->digits < 1) { iinvparm("iaddk"); }
  
  if (k >= 0)
    {
    if (!(a->flags & NegativeReg))
      {
      f = xaddk(a->value, k, a->digits);
      if (f & CarryFlag) 
        {
        if (a->capacity <= a->digits) { uextend(a, a->digits + 1); }
        a->value[a->digits] = 1;
        a->digits++;
        }
      }
    else
      {
      f = xsubk(a->value, k, a->digits);
      if (f & CarryFlag) 
        {
        xneg(a->value, a->digits);
        a->flags ^= NegativeReg;
        }
      }
    }
  else /* k < 0 */
    {
    if (!(a->flags & NegativeReg))
      {
      f = xsubk(a->value, -k, a->digits);
      if (f & CarryFlag) 
        {
        xneg(a->value, a->digits);
        a->flags ^= NegativeReg;
        }
      }
    else
      {
      f = xaddk(a->value, -k, a->digits);
      if (f & CarryFlag) 
        {
        if (a->capacity <= a->digits) { uextend(a, a->digits + 1); }
        a->value[a->digits] = 1;
        a->digits++;
        }
      }
    }
  }
  
void isub (ireg *a, ireg *b)
  /* 
     a = a - b
  */
  {
  static long f;
  if (a->value == NULL) { iinvparm("isub"); }
  if (b->value == NULL) { iinvparm("isub"); }
  if (a->digits < 1)    { iinvparm("isub"); }
  if (b->digits < 1)    { iinvparm("isub"); }

  /* Extend a if shorter than b */
  if (a->digits < b->digits)
    {
    if (a->capacity < b->digits) { uextend(a, b->digits); }
    xmovz(&a->value[a->digits], b->digits - a->digits);
    a->digits = b->digits;
    }   

  if ((a->flags & NegativeReg) != (b->flags & NegativeReg))
    {
    /* Different signs */
    f = xadd(a->value, b->value, a->digits, b->digits);
    if (f & CarryFlag) 
      {
      if (a->capacity <= a->digits) { uextend(a, a->digits + 1); }
      a->value[a->digits] = 1;
      a->digits++;
      }
    }
  else
    {
    /* Same signs */
    f = xsub(a->value, b->value, a->digits, b->digits);
    if (f & CarryFlag) 
      {
      xneg(a->value, a->digits);
      a->flags ^= NegativeReg;
      }
    }
  }
    
long isgn (ireg *a)
  /*
     sgn(a) = -1 if a < 0, 0 if a = 0, 1 if a > 0
  */
  {
  if (a->value == NULL) { iinvparm("isgn"); }
  if (a->digits < 1) { iinvparm("isgn"); }

  if (!xsig(a->value, a->digits)) { return(0); }
  if (a->flags & NegativeReg)     { return(-1); }
  return(1);
  }
  
long icmp (ireg *a, ireg *b)
  /*
     sgn(a - b) = -1 if a < b, 0 if a = b, 1 if a > b
  */
  {
  static long s;
  if (a->value == NULL) iinvparm("icmp"); 
  if (a->digits < 1)    iinvparm("icmp"); 
  if (b->value == NULL) iinvparm("icmp"); 
  if (b->digits < 1)    iinvparm("icmp"); 

  if (!(a->flags & NegativeReg))
    {
    if (!(b->flags & NegativeReg))
      {
      s = xcmp(a->value, b->value, a->digits, b->digits); /* a >= 0, b >= 0 */
      if (s & ZeroFlag)  return(0); 
      if (s & CarryFlag) return(-1);
      else               return(1); 
      }
    else
      {
      if (isgn(a)) return(1);                             /* a >= 0, b <= 0 */
      if (isgn(b)) return(1);
      return(0);
      }
    }
  else 
    {
    if (b->flags & NegativeReg)
      {
      s = xcmp(b->value, a->value, b->digits, a->digits); /* a <= 0, b <= 0 */
      if (s & ZeroFlag)  return(0); 
      if (s & CarryFlag) return(-1);
      else               return(1); 
      }
    else
      {
      if (isgn(a)) return(-1);                            /* a <= 0, b >= 0 */
      if (isgn(b)) return(-1); 
      return(0);
      }
    }
  }

long icmpk (ireg *a, long k)
  /*
     sgn(a - k) = -1 if a < k, 0 if a = k, 1 if a > k, -2^31 < k < 2^31
  */
  {
  static long s, b;
  if (a->value == NULL) iinvparm("icmp");
  if (a->digits < 1)    iinvparm("icmp");
  if (k > 0)
    {
    if (!(a->flags & NegativeReg))
      {     
      b = k;                               /* a >= 0, k > 0 */
      s = xcmp(a->value, &b, a->digits, 1);
      if (s & ZeroFlag)  return(0); 
      if (s & CarryFlag) return(-1);
      else               return(1); 
      }
    else return(-1);                       /* a <= 0, k > 0 */
    }
  else if (k < 0)
    {
    if (a->flags & NegativeReg)
      {
      b = -k;                              /* a <= 0, k < 0 */
      s = xcmp(&b, a->value, 1, a->digits);
      if (s & ZeroFlag)  return(0); 
      if (s & CarryFlag) return(-1);
      else               return(1); 
      }
    else return(1);                        /* a >= 0, k < 0 */
    }
  else return(isgn(a));                    /* k = 0 */
  }
  
void isqu (ireg *a)
  /*
     a = a^2
  */
  {
  static long da2, da4;
  register long L;
  if (a->value == NULL) { iinvparm("isqu"); }
  if (a->digits < 1)    { iinvparm("isqu"); }
  utrim(a);
  da2 = (a->digits + a->digits);
  
  if (a->digits < FFTsqu)
    /* School method */
    {
    if (a->capacity < da2) { uextend(a, da2); }
    xsqu(a->value, a->digits);
    }
  else
    /* Traditional FFT method */
    {
    /* Allocate FFT vector */
    /* Set L = least power of two greater than 4 * d(a)   */
    /* FFT elements are 16 bits */
    /* BUG! This routine does not work well with numbers >= 2^1000000 */
    da4 = da2 + da2;
    L = 1;
    while (L <= da4) { L += L; }
    if (a->capacity <= (L>>1)) { uextend(a, 1 + (L>>1)); }
    xfft_init(L, 1);
    xfft_pack(FFTv1, (unsigned short *)a->value, L, da2);
    xfft_real_to_hermitian(FFTv1, L);
    xfft_square_hermitian(FFTv1, L);
    xfft_inv_hermitian_to_real(FFTv1, L);
    xfft_unpack(FFTv1, (unsigned short *)a->value, L);
    }
 
  a->digits = da2;
  a->flags = 0;
  }
  
void imul (ireg *a, ireg *b)
  /*
     a = a * b
  */
  {
  static long da2, db2, dab, dmax2, dmax4;
  register long L;
  if (a->value == NULL) { iinvparm("imul"); }
  if (b->value == NULL) { iinvparm("imul"); }
  if (a->digits < 1)    { iinvparm("imul"); }
  if (b->digits < 1)    { iinvparm("imul"); }
  utrim(a);
  utrim(b);
  da2 = (a->digits + a->digits);
  db2 = (b->digits + b->digits);
  dab = a->digits + b->digits;

  if (a->digits < FFTmul || b->digits < FFTmul)
    /* School method */
    {
    if (a->capacity < dab) { uextend(a, dab); }
    xmul(a->value, b->value, a->digits, b->digits);
    }
  else
    /* Traditional FFT method */
    {
    /* Allocate FFT vectors */
    /* Set L = least power of two greater than 4 * max(d(a), d(b)) */
    /* FFT elements are 16 bits */
    /* BUG! This routine does not work well with numbers >= 2^1000000 */
    dmax2 = a->digits;
    if (dmax2 < b->digits) dmax2 = b->digits;
    dmax2 += dmax2;
    dmax4 = dmax2 + dmax2;
    L = 1;
    while (L <= dmax4) { L += L; }
    if (a->capacity <= (L>>1)) { uextend(a, 1 + (L>>1)); }
    xfft_init(L, 3);
    xfft_pack(FFTv1, (unsigned short *)a->value, L, da2);
    xfft_pack(FFTv2, (unsigned short *)b->value, L, db2);
    xfft_real_to_hermitian(FFTv1, L);
    xfft_real_to_hermitian(FFTv2, L);
    xfft_mul_hermitian(FFTv1, FFTv2, L);
    xfft_inv_hermitian_to_real(FFTv1, L);
    xfft_unpack(FFTv1, (unsigned short *)a->value, L);
    }
  a->digits = dab;
  a->flags = a->flags ^ (b->flags & NegativeReg);
  }
  
void imulk (ireg *a, long k)
  /*
     a = a * k
  */
  {
  if (a->value == NULL) { iinvparm("imulk"); }
  if (a->digits < 1)    { iinvparm("imulk"); }
  utrim(a);
  if (a->capacity <= a->digits) { uextend(a, a->digits + 1); }
  if (k < 0)
    {
    xmulk(a->value, -k, a->digits);
    a->flags ^= NegativeReg;
    }
  else
    { xmulk(a->value, k, a->digits); }
  a->digits++;
  }
  
void imul2k (ireg *a, long k)
  /*
     a = [a * 2^k],  (k = 32-bit positive integer)
  */
  {
  static long da;
  if (a->value == NULL) { iinvparm("imul2k"); }
  if (a->digits < 1)    { iinvparm("imul2k"); }
  
  da = (k + 31) >> 5;
  if (a->capacity < a->digits + da) { uextend(a, a->digits + da); }
  xmul2k(a->value, k, a->digits);
  a->digits += da;
  }
  
void idiv (ireg *a, ireg *b)
  /*
     a =  [ a / b], res =  ( a mod k), a >= 0, 
     a = -[ a / b], res = -(-a mod k), a <  0;  b > 0

     (b is trimmed by this routine)
  */
  {
  if (a->value == NULL) { iinvparm("idiv"); }
  if (a->digits < 1)    { iinvparm("idiv"); }
  if (b->value == NULL) { iinvparm("idiv"); }
  if (b->digits < 1)    { iinvparm("idiv"); }
  if (isgn(b) <= 0)     { iinvparm("idiv"); }
  utrim(a);
  utrim(b);
  if (b->digits > 1)
    {
    imov(res, a);
    if (res->digits >= b->digits)
      {
      if (res->capacity <= res->digits) { uextend(res, res->digits + 1); }
      xdiv(res->value, b->value, res->digits, b->digits, a->value);
      a->digits   = res->digits - b->digits + 1;
      res->digits = b->digits;
      }
    else
      /* d(b) > d(a) */
      {
      imovk(a, 0);
      }
    }
  else
    /* b->digits = 1 */
    {
    imovk(res, 0);
    res->value[0] = xdivk(a->value, b->value[0], a->digits);
    res->flags = a->flags;
    }
  }
  
void idivk (ireg *a, long k)
  /*
     a =  [ a / k], kres =  ( a mod k), a >= 0,
     a = -[-a / k], kres = -(-a mod k), a <  0;  k = 32-bit positive integer
  */
  {
  if (a->value == NULL) { iinvparm("idivk"); }
  if (a->digits < 1)    { iinvparm("idivk"); }
  if (k == 0)           { iinvparm("idivk"); }

  utrim(a);
  kres = xdivk(a->value, k, a->digits);
  if (a->flags & NegativeReg) { kres = -kres; }
  }
  
void idiv2k (ireg *a, long k)
  /*
     a = [a / 2^k],  (k = 32-bit positive integer)
  */
  {
  static long d;
  if (a->value == NULL) { iinvparm("idiv2k"); }
  if (a->digits < 1)    { iinvparm("idiv2k"); }

  d = k >> 5;
  if (d < a->digits)
    {
    xdiv2k(a->value, k, a->digits);
    a->digits -= d;
    }
  else 
    { imovk(a, 0); }
  }
  
void imod (ireg *a, ireg *b)
  /*
     a = (a mod b) >= 0, b > 0

     (b is trimmed by this routine)
  */
  {
  if (a->value == NULL) { iinvparm("imod"); }
  if (a->digits < 1)    { iinvparm("imod"); }
  if (b->value == NULL) { iinvparm("imod"); }
  if (b->digits < 1)    { iinvparm("imod"); }
  if (isgn(b) <= 0)     { iinvparm("imod"); }
  utrim(a);
  utrim(b);
  if (b->digits > 1)
    {
    if (a->digits >= b->digits)
      {
      if (a->capacity <= a->digits) { uextend(a, a->digits + 1); }
      xmod(a->value, b->value, a->digits, b->digits);
      a->digits = b->digits;
      }
    /* Remainder must be non-negative */
    if (isgn(a) < 0) { iadd(a, b); }
    }
  else
    /* b->digits = 1 */
    {
    imodk(a, b->value[0]);
    }
  }
 
void imodk (ireg *a, long k)
  /*
     a = (a mod k) >= 0,  k = 32-bit positive integer
  */
  {
  if (a->value == NULL) { iinvparm("imodk"); }
  if (a->digits < 1)    { iinvparm("imodk"); }
  if (k == 0)           { iinvparm("imodk"); }

  utrim(a);
  xmodk(a->value, k, a->digits);
  a->digits = 1;
  /* Result must be non-negative */
  if ((a->flags & NegativeReg) && a->value[0])
    {
    a->value[0] = k - a->value[0];
    a->flags ^= NegativeReg;
    }
  }
  
void imod2k (ireg *a, long k)
  /*
     a = (a mod 2^k) > 0,  k = 32-bit positive integer

     d(a)=6, k=72           kr=8 km=255
                            |
     |a a a a|a a a a|a a a a|a a a a|a a a a|
                             |<-----kq------>|
                     |<---------da---------->|
  */
  {
  static long da, kq, kr; 
  static unsigned long km;
  if (a->value == NULL) { iinvparm("imod2k"); }
  if (a->digits < 1)    { iinvparm("imod2k"); }

  if (k)
    {  
    kq = k >> 5;
    kr = k & 31;
    km = 1 << kr; km--;
    da = (k + 31) >> 5;
    if (a->flags & NegativeReg)
      {
      a->flags ^= NegativeReg;
      if (!usig(a)) return;
      /* a < 0: compute 2^k - |a| */
      if (a->capacity < da) { uextend(a, da); }
      if (a->digits < da) 
        { xmovz(&a->value[a->digits], da - a->digits); }
      xneg(a->value, da);
      if (kr) { a->value[kq] &= km; }
      }
    else    
      {
      if (kq >= a->digits) { return; }
      if (kr)              { a->value[kq] &= km; }
      }
      a->digits = da;
    }
  else { imovk(a, 0); }
  }

void iexp(ireg *a, ireg *b, ireg *c)
  /* 
    a = b^c
  */
  {
  static long i, f;
  static unsigned long j;
  if (b->value == NULL) { iinvparm("iexp"); }
  if (b->digits < 1)    { iinvparm("iexp"); }
  if (c->value == NULL) { iinvparm("iexp"); }
  if (c->digits < 1)    { iinvparm("iexp"); }
  imovk(a, 1);
  if (isgn(c) <= 0) { return; }
  f = 0;
  for (i = c->digits - 1; i >= 0; i--)
    {
    for (j = 0x80000000; j >= 1; j >>= 1)
      {
      if (f)               { isqu(a); }
      if (c->value[i] & j) { imul(a, b); f++; }
      }
    }
  }

void iexpmod(ireg *a, ireg *b, ireg *c, ireg *q)
  /* 
    a = (b^c mod q)

    Assumes b, c >= 0, q > 0
  */
  {
  static long i, f;
  static unsigned long j;
  if (b->value == NULL) { iinvparm("iexpmod"); }
  if (b->digits < 1)    { iinvparm("iexpmod"); }
  if (c->value == NULL) { iinvparm("iexpmod"); }
  if (c->digits < 1)    { iinvparm("iexpmod"); }
  imovk(a, 1);
  if (isgn(c) <= 0) {return; }
  f = 0;
  for (i = c->digits - 1; i >= 0; i--)
    {
    for (j = 0x80000000; j >= 1; j >>= 1)
      {
      if (f)               { isqu(a);    imod(a, q);      }
      if (c->value[i] & j) { imul(a, b); imod(a, q); f++; }
      }
    }
  }

void iexpmodm2ke(ireg *a, ireg *b, ireg *c, ireg *q, long m, long k, ireg *e)
  /* 
    a = (b^c mod q), where q = m*2^k + e

    Assumes b, c >= 0, q = m*2^k + e > 0
  */
  {
  static long i, f;
  static unsigned long j;
  if (b->value == NULL) { iinvparm("iexpmodm2ke"); }
  if (b->digits < 1)    { iinvparm("iexpmodm2ke"); }
  if (c->value == NULL) { iinvparm("iexpmodm2ke"); }
  if (c->digits < 1)    { iinvparm("iexpmodm2ke"); }
  if (q->value == NULL) { iinvparm("iexpmodm2ke"); }
  if (q->digits < 1)    { iinvparm("iexpmodm2ke"); }
  if (isgn(q) <= 0)     { iinvparm("iexpmodm2ke"); }
  imovk(a, 1);
  if (isgn(c) <= 0) {return; }
  f = 0;
  for (i = c->digits - 1; i >= 0; i--)
    {
    for (j = 0x80000000; j >= 1; j >>= 1)
      {
      if (f)
        {
        isqu  (a);
        usep2k(a, res, k);      /* a = a_0, res = a_1 */
        idivk (res, m);         /* res = a_11, kres = a_10 */
        imul  (res, e);         /* res = e*a_11 */
        isub  (a, res);
        imovk (res, kres);
        imul2k(res, k);         /* res = a_10*2^k */
        iadd  (a, res);
        }
      if (c->value[i] & j)
        {
        imul  (a, b);
        usep2k(a, res, k);      /* a = a_0, res = a_1 */
        idivk (res, m);         /* res = a_11, kres = a_10 */
        imul  (res, e);         /* res = e*a_11 */
        isub  (a, res);
        imovk (res, kres);
        imul2k(res, k);         /* res = a_10*2^k */
        iadd  (a, res);
        f++;
        }
      utrim(a);
      if (a->digits > q->digits + 8) { imod(a, q); }
      }
    }
  imod(a, q);
  }

void igcd(ireg *a, ireg *b)
  /* 
    a = gcd(a, b)  (b is destroyed in the process)
  */
  {
  static long sb;
  if (a->value == NULL) { iinvparm("igcd"); }
  if (a->digits < 1)    { iinvparm("igcd"); }
  if (b->value == NULL) { iinvparm("igcd"); }
  if (b->digits < 1)    { iinvparm("igcd"); }
  if (isgn(a) < 0) { ineg (a); }
  sb = isgn(b);
  if (sb < 0) { ineg (b); }
  while (sb)
    {
    imod(a, b);
    if (!usig(a)) { imov(a, b); return; }
    imod(b, a);
    if (!usig(b)) { return; }
    }   
  }

void igcdext(ireg *u, ireg *d, ireg *a, ireg *b)
  /* 
    Return u, d, where a*u = d (mod b), d = gcd(a, b)
  */
  {
  if (a->value == NULL) { iinvparm("igcdext"); }
  if (a->digits < 1)    { iinvparm("igcdext"); }
  if (b->value == NULL) { iinvparm("igcdext"); }
  if (b->digits < 1)    { iinvparm("igcdext"); }
  if (isgn(b) <= 0)     { iinvparm("igcdext"); }
  imov (d, a);
  imod (d, b);
  imovk(u, 1);
  imovk(temp1, 0);
  imov (temp2, b);
  while (usig(temp2))
    {
    imov(temp5, d);
    idiv(temp5, temp2);
    imov(temp4, res);
    imul(temp5, temp1);
    imov(temp3, u);
    isub(temp3, temp5);
    imov(u, temp1);
    imov(d, temp2);
    imov(temp1, temp3);
    imov(temp2, temp4);
    }
  imod(u, b);
  }

long ilen (ireg *a)
  /*
     Return [log(|a|)/log(2)] + 1, the length of a in bits.
  */
  {
  static long i, k;
  static unsigned long v, w;
  if (!isgn(a)) return(0);
  k = a->digits << 5;
  for (i = a->digits - 1; i >= 0; i--)
    {
    v = a->value[i];
    for (w = 0x80000000; w != 0; w >>= 1)
      {
      if (v & w) return(k); 
      k--;
      }
    }
  return(0);
  }

long ibit (ireg *a, long k)
  /*
     Return [|a|/2^k] mod 2, the value of bit k of a. 
     Bit 0 is the units bit.
  */
  {
  static long i;
  i = k >> 5;
  if (i >= a->digits) return(0);
  if (a->value[i] & (1 << (k & 31))) return(1);
  return(0);
  }

double ilog(ireg *a)
  {
  /* 
    Return max{log(|a|), 0}. 
  */
  static double u, v, w;
  if (a->value == NULL) { iinvparm("ilog"); }
  if (a->digits < 1)    { iinvparm("ilog"); }
  utrim(a);
  u = a->value[a->digits-1];
  if (a->digits > 1) v = a->value[a->digits-2]; else v = 0;
  if (u < 0) u += 4294967296.0;
  if (v < 0) v += 4294967296.0;
  w = u + v/4294967296.0;
  if (w <= 0) return(0.0);
  return(log(w) + 32.0*(a->digits-1)*ilog2);
  }

void isqrt(ireg *a, ireg *b)
  {
  /* 
     Set a = [b^(1/2)], res = b - [b^(1/2)]^2.
  */
  static unsigned long j, w;
  static double u;
  if (b->value == NULL) { iinvparm("isqrt"); }
  if (b->digits < 1)    { iinvparm("isqrt"); }
  if (isgn(b) < 0)      { iinvparm("isqrt"); }
  imovk(temp1, 1);
  if (icmp(b, temp1) <= 0)  { imov(a, b); imovk(res, 0); return; }
  /* Compute a = b^(1/2) by Newton's method */
  /* Compute first approximation to a */
  u = ilog(b)/2;
  j = floor(u/ilog2);
  if (j >= 30) j -= 30; else j = 0;
  w = floor(exp(u - ilog2*j));
  /* Our first guess for a is |w|. */
  imovk(a, w); iabs(a);
  imul2k(a, j);
  /* Iterate a -> (b + a^2) / (2*a) */
  for (;;)
    {
    imov  (temp2, a);         /* save old a */
    isqu  (a);
    iadd  (a, b);             /* a' = b + a^2 */
    idiv  (a, temp2);
    idiv2k(a, 1);             /* a' = (b + a^2)/(2*a) */  
    isub  (temp2, a);
    iabs  (temp2);
    if (icmp(temp2, temp1) <= 0) break;
    }
  /* a = [b^(1/2)] +/- 1 */
  iaddk(a, 1);
  imov(temp2, a); isqu(temp2);
  imov(res, b); 
  isub(res, temp2);           /* res = b - a^2 */
  while (isgn(res) ==  1) { isub(res, a); iaddk(a, 1); isub(res, a); }
  while (isgn(res) == -1) { iadd(res, a); isubk(a, 1); iadd(res, a); }
  }

void idisp (ireg *a)
  /*
     Display an integer register in decimal
  */
  {
  static long i, ddigits, dec, significant;
  if (a->value == NULL) { iinvparm("idisp"); }
  if (a->digits < 1)    { iinvparm("idisp"); }

  /* Decimal occupies at least log(2^32)/log(10^9) times as much space */
  ddigits = ((9*a->digits) >> 3) + 4;
  if (res->value == NULL)      { ualloc(res, ddigits, 1); }
  if (res->capacity < ddigits) { uextend(res, ddigits); }

  /* Repeatedly divide by 10^9, saving remainder starting at ddigits - 1 */
  umov(res, a);
  res->digits = a->digits;
  dec = ddigits;
  while (xsig(res->value, res->digits))
    {
    dec--;
    res->value[dec] = xdivk(res->value, 1000000000, res->digits);
    utrim(res);
    }

  /* Print the remainders */
  if (isgn(a) < 0) { printf("-"); }
  significant = 0;
  for (i = dec; i < ddigits; i++)
    {  
    if (res->value[i] && !significant)
      {
      printf("%d", res->value[i]);
      significant = 1;
      }
    else { printf("%09d", res->value[i]); }
    }
  if (!significant) { printf("0"); }
  }

void idispf (FILE *f, ireg *a)
  /*
     Write an integer register in decimal to a file
  */
  {
  static long i, ddigits, dec, significant;
  if (a->value == NULL) { iinvparm("idisp"); }
  if (a->digits < 1)    { iinvparm("idisp"); }

  /* Decimal occupies at least log(2^32)/log(10^9) times as much space */
  ddigits = ((9*a->digits) >> 3) + 4;
  if (res->value == NULL)      { ualloc(res, ddigits, 1); }
  if (res->capacity < ddigits) { uextend(res, ddigits); }

  /* Repeatedly divide by 10^9, saving remainder starting at ddigits - 1 */
  umov(res, a);
  res->digits = a->digits;
  dec = ddigits;
  while (xsig(res->value, res->digits))
    {
    dec--;
    res->value[dec] = xdivk(res->value, 1000000000, res->digits);
    utrim(res);
    }

  /* Print the remainders */
  if (isgn(a) < 0) { fprintf(f, "-"); }
  significant = 0;
  for (i = dec; i < ddigits; i++)
    {  
    if (res->value[i] && !significant)
      {
      fprintf(f, "%d", res->value[i]);
      significant = 1;
      }
    else { fprintf(f, "%09d", res->value[i]); }
    }
  if (!significant) { fprintf(f, "0"); }
  }

void iinvparm (char *c)
  /*
     Invalid parameter
  */
  {
  printf("Invalid parameter in %s call\n", c); 
  give_up();
  }
