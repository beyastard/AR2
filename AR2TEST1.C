/* AR2TEST1.C  2.2.1F  TONY FORBES  February 2000 */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2e.h";
#include "ar2t.h";

void ttestdivmod(char *n, char *m, long to, long mult);
void ttest2k(long i, long k);
void ttbit(char *dig);

static ireg ra, rb, rc, rd, re, rf, rg, rh, rw, rx, ry, rz;
static ireg rs, rt, ru, rv;
static ireg *a = &ra; static ireg *b = &rb;
static ireg *c = &rc; static ireg *d = &rd;
static ireg *e = &re; static ireg *f = &rf;
static ireg *g = &rg; static ireg *h = &rh;
static ireg *s = &rs; static ireg *t = &rt;
static ireg *u = &ru; static ireg *v = &rv;
static ireg *w = &rw; static ireg *x = &rx;
static ireg *y = &ry; static ireg *z = &rz;

extern long FFTsqu;
extern long FFTmul;

void main()
  {

  static long i;
  tprint("ARITH2 TEST 1  Version 2.1.4m\n");

  tprint("Check imovd and idisp\n");
  imovd(x, "0000011111222223333344444555556666677777888889999900000"); 
  tprint("11111222223333344444555556666677777888889999900000 = \n"); 
  tidisp(x); tprint("\n"); 
  imovd(y, "-0000011111222223333344444555556666677777888889999900000");
  tprint("-11111222223333344444555556666677777888889999900000 = \n"); 
  tidisp(y); tprint("\n"); 
  imov(z, x); iadd(z, y); tprint("0 = "); tidisp(z); tprint("\n");
  imovd(v, "1"); 
  tprint( "1 = "); tidisp(v); tprint("\n"); 
  imovd(w, "-1"); 
  tprint( "-1 = "); tidisp(w); tprint("\n"); 
  imov(z, v); iadd(z, w); tprint("0 = "); tidisp(z); tprint("\n");
  kres = -1;
  imovk(res, kres);
  imovd(res, "_234");
  imovd(res, "_56789");
  imov (z, res);
  tprint("-123456789 = "); tidisp(z); tprint("\n"); 
  halt();
  imulk(v, 10);
  for (i = 1; i <= 12; i++)
    {
    isqu(v);
    tidisp(v); tprint(" v=10^2^%d, d(v)=%d\n", i, v->digits);
    }
 
  tprint("Fundamental operations: move, add, subtract, compare\n");

  tprint("Check iadd\n");
  /* Both operands same size and small */
  imovk(a, 5); imovk(b, 3); imovk(c, -4); imovk(d, -6);
  imov(e, a); iadd(e, b); tcheckd(e,   "8", "iadd 5 + 3", 11);
  imov(e, a); iadd(e, c); tcheckd(e,   "1", "iadd 5 - 4", 11);
  imov(e, a); iadd(e, d); tcheckd(e,  "-1", "iadd 5 - 6", 11);
  imov(e, c); iadd(e, d); tcheckd(e, "-10", "iadd -4 - 6", 11);
  imov(e, c); iadd(e, a); tcheckd(e,   "1", "iadd -4 + 5", 11);
  imov(e, c); iadd(e, b); tcheckd(e,  "-1", "iadd -4 + 3", 11);
  imov(e, a); iadd(e, b); iadd(e, c); iadd(e, c); tcheckd(e, "0", "iadd 0", 11);
  /* Operand 1 small, operand 2 large */
  imulk(b, 1000000000); imulk(b, 1000000000); imulk(b, 1000000000); imulk(b, 1000000000);
  imulk(d, 1000000000); imulk(d, 1000000000); imulk(d, 1000000000); imulk(d, 1000000000);
  imov(e, a); iadd(e, b);
  tcheckd(e,  "3000000000000000000000000000000000005", "iadd 5 + 3*10^36", 15);
  imov(e, a); iadd(e, d);
  tcheckd(e, "-5999999999999999999999999999999999995", "iadd 5 - 6*10^36", 15);
  imov(e, c); iadd(e, b);
  tcheckd(e,  "2999999999999999999999999999999999996", "iadd -4 + 3*10^36", 15);
  imov(e, c); iadd(e, d);
  tcheckd(e, "-6000000000000000000000000000000000004", "iadd -4 - 6*10^36", 15);
  imov(e, a); iadd(e, a); iadd(e, a); iadd(e, a);
  iadd(e, b); iadd(e, b); iadd(e, b); iadd(e, b);
  iadd(e, c); iadd(e, c); iadd(e, c); iadd(e, c); iadd(e, c);
  iadd(e, d); iadd(e, d); tcheckd(e, "0", "iadd 0", 15);
  /* Operand 1 large, operand 2 small */
  imov(e, b); iadd(e, a);
  tcheckd(e,  "3000000000000000000000000000000000005", "iadd 3*10^36 + 5", 51);
  imov(e, d); iadd(e, a);
  tcheckd(e, "-5999999999999999999999999999999999995", "iadd -6*10^36 + 5", 51);
  imov(e, b); iadd(e, c);
  tcheckd(e,  "2999999999999999999999999999999999996", "iadd 3*10^36 - 4", 51);
  imov(e, d); iadd(e, c);
  tcheckd(e, "-6000000000000000000000000000000000004", "iadd -6*10^36 - 4", 51);
  imov(e, c); iadd(e, c); iadd(e, c); iadd(e, c); iadd(e, c);
  iadd(e, a); iadd(e, a); iadd(e, a); iadd(e, a);
  iadd(e, b); iadd(e, b); iadd(e, b); iadd(e, b);
  iadd(e, d); iadd(e, d); tcheckd(e,   "0", "iadd 0", 51);
  /* Both operands same size and large */
  imulk(a, 1000000000); imulk(a, 1000000000); imulk(a, 1000000000); imulk(a, 1000000000);
  imulk(c, 1000000000); imulk(c, 1000000000); imulk(c, 1000000000); imulk(c, 1000000000);
  imov(e, a); iadd(e, b); tcheckd(e,   "8000000000000000000000000000000000000", "iadd(5 + 3)*10^36", 55);
  imov(e, a); iadd(e, c); tcheckd(e,   "1000000000000000000000000000000000000", "iadd(5 - 4)*10^36", 55);
  imov(e, a); iadd(e, d); tcheckd(e,  "-1000000000000000000000000000000000000", "iadd(5 - 6)*10^36", 55);
  imov(e, c); iadd(e, d); tcheckd(e, "-10000000000000000000000000000000000000", "iadd(-4 - 6)*10^36", 55);
  imov(e, c); iadd(e, a); tcheckd(e,   "1000000000000000000000000000000000000", "iadd(-4 + 5)*10^36", 55);
  imov(e, c); iadd(e, b); tcheckd(e,  "-1000000000000000000000000000000000000", "iadd(-4 + 3)*10^36", 55);
  imov(e, a); iadd(e, b); iadd(e, c); iadd(e, c); tcheckd(e, "0", "iadd 0", 55);

  tprint("Check iaddk\n");
  /* Both operands same size and small */
  imovk(a, 5); imovk(b, 3); imovk(c, -4); imovk(d, -6);
  imov(e, a); iaddk(e, 2+1); tcheckd(e,   "8", "iaddk 5 + 3", 11);
  imov(e, a); iaddk(e,-2-2); tcheckd(e,   "1", "iaddk 5 - 4", 11);
  imov(e, a); iaddk(e,-3-3); tcheckd(e,  "-1", "iaddk 5 - 6", 11);
  imov(e, c); iaddk(e,-2-2-2); tcheckd(e, "-10", "iaddk -4 - 6", 11);
  imov(e, c); iaddk(e, 1+2+2); tcheckd(e,   "1", "iaddk -4 + 5", 11);
  imov(e, c); iaddk(e, 3); tcheckd(e,  "-1", "iaddk -4 + 3", 11);
  imov(e, a); iaddk(e, 3); iaddk(e,-4); iaddk(e,-4); tcheckd(e, "0", "iaddk 0", 11);
  /* Operand 1 large, operand 2 small */
  imulk(b, 1000000000); imulk(b, 1000000000); imulk(b, 1000000000); imulk(b, 1000000000);
  imulk(d, 1000000000); imulk(d, 1000000000); imulk(d, 1000000000); imulk(d, 1000000000);
  imov(e, b); iaddk(e, 5);
  tcheckd(e,  "3000000000000000000000000000000000005", "iaddk 3*10^36 + 5", 51);
  imov(e, d); iaddk(e, 5);
  tcheckd(e, "-5999999999999999999999999999999999995", "iaddk -6*10^36 + 5", 51);
  imov(e, b); iaddk(e,-4);
  tcheckd(e,  "2999999999999999999999999999999999996", "iaddk 3*10^36 - 4", 51);
  imov(e, d); iaddk(e,-4);
  tcheckd(e, "-6000000000000000000000000000000000004", "iaddk -6*10^36 - 4", 51);
  imov(e, c); iaddk(e,-4); iaddk(e,-4); iaddk(e,-4); iaddk(e,-4);
  iaddk(e, 5); iaddk(e, 5); iaddk(e, 5); iaddk(e, 5);
  iaddk(e, 3); iaddk(e, 3); iaddk(e, 3); iaddk(e, 3);
  iaddk(e,-6); iaddk(e,-6); tcheckd(e, "0", "iaddk 0", 51);

  tprint("Check isub\n");
  /* Both operands same size and small */
  imovk(a, 5); imovk(b, 3); imovk(c, -4); imovk(d, -6);
  imov(e, a); isub(e, b); tcheckd(e,  "2", "isub 5 - 3", 11);
  imov(e, a); isub(e, c); tcheckd(e,  "9", "isub 5 + 4", 11);
  imov(e, a); isub(e, d); tcheckd(e, "11", "isub 5 + 6", 11);
  imov(e, c); isub(e, d); tcheckd(e,  "2", "isub -4 + 6", 11);
  imov(e, c); isub(e, a); tcheckd(e, "-9", "isub -4 - 5", 11);
  imov(e, c); isub(e, b); tcheckd(e, "-7", "isub -4 - 3", 11);
  imov(e, a); ineg(e); isub(e, b); isub(e, c); isub(e, c); tcheckd(e, "0", "isub 0", 11);
  /* Operand 1 small, operand 2 large */
  imulk(b, 1000000000); imulk(b, 1000000000); imulk(b, 1000000000); imulk(b, 1000000000);
  imulk(d, 1000000000); imulk(d, 1000000000); imulk(d, 1000000000); imulk(d, 1000000000);
  imov(e, a); isub(e, b);
  tcheckd(e, "-2999999999999999999999999999999999995", "isub 5 - 3*10^36", 15);
  imov(e, a); isub(e, d);
  tcheckd(e,  "6000000000000000000000000000000000005", "isub 5 + 6*10^36", 15);
  imov(e, c); isub(e, b);
  tcheckd(e, "-3000000000000000000000000000000000004", "isub -4 - 3*10^36", 15);
  imov(e, c); isub(e, d);
  tcheckd(e,  "5999999999999999999999999999999999996", "isub -4 + 6*10^36", 15);
  imov(e, a); ineg(e); isub(e, a); isub(e, a); isub(e, a);
  isub(e, b); isub(e, b); isub(e, b); isub(e, b);
  isub(e, c); isub(e, c); isub(e, c); isub(e, c); isub(e, c);
  isub(e, d); isub(e, d); tcheckd(e, "0", "isub 0", 15);
  /* Operand 1 large, operand 2 small */
  imov(e, b); isub(e, a);
  tcheckd(e,  "2999999999999999999999999999999999995", "isub 3*10^36 - 5", 51);
  imov(e, d); isub(e, a);
  tcheckd(e, "-6000000000000000000000000000000000005", "isub -6*10^36 - 5", 51);
  imov(e, b); isub(e, c);
  tcheckd(e,  "3000000000000000000000000000000000004", "isub 3*10^36 + 4", 51);
  imov(e, d); isub(e, c);
  tcheckd(e, "-5999999999999999999999999999999999996", "isub -6*10^36 + 4", 51);
  imov(e, c); ineg(e); isub(e, c); isub(e, c); isub(e, c); isub(e, c);
  isub(e, a); isub(e, a); isub(e, a); isub(e, a);
  isub(e, b); isub(e, b); isub(e, b); isub(e, b);
  isub(e, d); isub(e, d); tcheckd(e, "0", "isub 0", 51);
  /* Both operands same size and large */
  imulk(a, 1000000000); imulk(a, 1000000000); imulk(a, 1000000000); imulk(a, 1000000000);
  imulk(c, 1000000000); imulk(c, 1000000000); imulk(c, 1000000000); imulk(c, 1000000000);
  imov(e, a); isub(e, b); tcheckd(e,  "2000000000000000000000000000000000000", "isub(5 - 3)*10^36", 55);
  imov(e, a); isub(e, c); tcheckd(e,  "9000000000000000000000000000000000000", "isub(5 + 4)*10^36", 55);
  imov(e, a); isub(e, d); tcheckd(e, "11000000000000000000000000000000000000", "isub(5 + 6)*10^36", 55);
  imov(e, c); isub(e, d); tcheckd(e,  "2000000000000000000000000000000000000", "isub(-4 + 6)*10^36", 55);
  imov(e, c); isub(e, a); tcheckd(e, "-9000000000000000000000000000000000000", "isub(-4 - 5)*10^36", 55);
  imov(e, c); isub(e, b); tcheckd(e, "-7000000000000000000000000000000000000", "isub(-4 - 3)*10^36", 55);
  imov(e, a); ineg(e); isub(e, b); isub(e, c); isub(e, c); tcheckd(e, "0", "isub 0", 55);

  tprint("Check isubk\n");
  /* Both operands same size and small */
  imovk(a, 5); imovk(b, 3); imovk(c, -4); imovk(d, -6);
  imov(e, a); isubk(e, 3); tcheckd(e,  "2", "isubk 5 - 3", 11);
  imov(e, a); isubk(e,-4); tcheckd(e,  "9", "isubk 5 + 4", 11);
  imov(e, a); isubk(e,-6); tcheckd(e, "11", "isubk 5 + 6", 11);
  imov(e, c); isubk(e,-6); tcheckd(e,  "2", "isubk -4 + 6", 11);
  imov(e, c); isubk(e, 5); tcheckd(e, "-9", "isubk -4 - 5", 11);
  imov(e, c); isubk(e, 3); tcheckd(e, "-7", "isubk -4 - 3", 11);
  imov(e, a); ineg(e); isubk(e, 3); isubk(e,-4); isubk(e,-4); tcheckd(e, "0", "isubk 0", 11);
  /* Operand 1 large, operand 2 small */
  imulk(b, 1000000000); imulk(b, 1000000000); imulk(b, 1000000000); imulk(b, 1000000000);
  imulk(d, 1000000000); imulk(d, 1000000000); imulk(d, 1000000000); imulk(d, 1000000000);
  imov(e, b); isubk(e, 5);
  tcheckd(e,  "2999999999999999999999999999999999995", "isubk 3*10^36 - 5", 51);
  imov(e, d); isubk(e, 5);
  tcheckd(e, "-6000000000000000000000000000000000005", "isubk -6*10^36 - 5", 51);
  imov(e, b); isubk(e,-4);
  tcheckd(e,  "3000000000000000000000000000000000004", "isubk 3*10^36 + 4", 51);
  imov(e, d); isubk(e,-4);
  tcheckd(e, "-5999999999999999999999999999999999996", "isubk -6*10^36 + 4", 51);
  imov(e, c); ineg(e); isubk(e,-4); isubk(e,-4); isubk(e,-4); isubk(e,-4);
  isubk(e, 5); isubk(e, 1+2+2); isubk(e, 2+3); isubk(e, 10-5);
  isubk(e, 3); isubk(e, 6-3); isubk(e, 0+3); isubk(e, 9/3);
  isubk(e,-12+6); isubk(e,-6); tcheckd(e, "0", "isubk 0", 51);
  
  tprint("Check imovk, iaddk, isubk, imov, iadd, isub again\n");
  imovk(a, 999999999); tcheckd(a, "999999999", "imovk", 1);
  imovk(b, 222222222); tcheckd(b, "222222222", "imovk", 2);
  imov(c, a); imov(d, a); imov(e, a); imov(f, a);
  for (i = 1; i <= 10001; i++)
    {  
    iaddk(c, 222222222);
    isubk(d, 222222222);
    iaddk(e, -222222222);
    isubk(f, -222222222);
    }
  tcheckd(c,  "2223444442221", "iaddk", 3);
  tcheckd(d, "-2221444442223", "isubk", 4);
  tcheckd(e, "-2221444442223", "iaddk", 5);
  tcheckd(f,  "2223444442221", "isubk", 6);
  for (i = 1; i <= 101; i++)
    {  
    imov(h, b); iaddk(h,17); ineg(h); 
    iadd(c, b); isub(d, b);
    iadd(e, h); isub(f, h);
    imulk(b, 3);
    }
  tcheckd(c,  "171792506738877936768149932909720047645495418656297490443", "iaddk", 13);
  tcheckd(d, "-171792506738877936768149932909720047645495418654297490445", "isubk", 14);
  tcheckd(e, "-171792506738877936768149932909720047645495418654297492162", "iaddk", 15);
  tcheckd(f,  "171792506738877936768149932909720047645495418656297492160", "isubk", 16);
  for (i = 0; i <= 1000000000; i += 1234567)
    {
    uaddk(c, i); usubk(f, i);
    }
  tcheckd(c, "171792506738877936768149932909720047645495419061797194428", "uadd", 23);
  tcheckd(f, "171792506738877936768149932909720047645495418250797788175", "usub", 24);

  tprint("Check umovf, uaddf, usubf\n");
  /* s, t and u are 5 digits, so we are working modulo 2^160 */
  ualloc(s, 5, 5); ualloc(t, 5, 5); ualloc(u, 5, 5); 
  imovd(x, "1461501637330902918203684832716283019655932542976"); /* 2^160 */
  imovd(a, "3000000000300000000030000000003000000000");
  umovf(s, a); s->flags = 0; tcheckr(s, a, "umovf", 1);
  imovd(b, "5000000000500000000050000000005000000000");
  umovf(t, b); t->flags = 0; tcheckr(t, b, "umovf", 2);
  umovf(c, a); c->flags = 0;
  umovf(u, s); u->flags = 0;
  isub(c, b); iadd(c, x);     /* c = a - b + 2^160      */
  usubf(u, t);                /* u = s - t  (mod 2^160) */
  tcheckr(u, c, "usubf", 3);
  iadd(c, b); isub(c, x);     /* c = a                  */
  uaddf(u, t);                /* u = s                  */
  tcheckr(u, c, "uaddf", 4);
  iadd(c, b);                 /* c = a + b              */
  uaddf(u, t);                /* u = s + t              */
  tcheckr(u, c, "uaddf", 5);
  isub(c, a); isub(c, b);     /* c = 0                  */
  tcheckd(c, "0", "usubf", 6);
  usubf(u, s); usubf(u, t);   /* u = 0                  */
  tcheckd(u, "0", "usubf", 7);

  tprint("Check iadd, isub with one operand\n");
  imovk(a, -5); 
  iadd(a, a);
  tcheckd(a, "-10", "iadd", 1);
  isub(a, a);
  tcheckd(a, "0", "isub", 1);
  imovk(a, 2000000000); 
  iadd(a, a); iadd(a, a);
  tcheckd(a, "8000000000", "iadd", 2);
  isub(a, a);
  tcheckd(a, "0", "isub", 2);
  imovd(a, "500000000000000000000000000000000000000000000000000000000000"); 
  iadd(a, a);
  tcheckd(a, "1000000000000000000000000000000000000000000000000000000000000", "iadd", 3);
  isub(a, a);
  tcheckd(a, "0", "isub", 3);

  tprint("Check ineg, isgn\n");
  imovk(a, 0);
  imovk(b, isgn(a)); tcheckd(b, "0", "isgn 0", 1);
  ineg(a);           tcheckd(a, "0", "ineg 0", 2);
  imovk(b, isgn(a)); tcheckd(b, "0", "isgn 0", 3);
  imovk(a, 230000);
  imovk(b, isgn(a)); tcheckd(b, "1", "isgn 1", 4);
  ineg(a); tcheckd(a, "-230000", "ineg -230000", 5);
  imovk(b, isgn(a)); tcheckd(b, "-1", "isgn -1", 6);
  ineg(a); tcheckd(a, "230000", "ineg 230000", 7);
  isqu(a); isqu(a); isqu(a); isqu(a); isqu(a); isqu(a); isqu(a); isqu(a);
  imovk(b, isgn(a)); tcheckd(b, "1", "isgn 1", 8);
  ineg(a);
  imovk(b, isgn(a)); tcheckd(b,"-1", "isgn -1", 9);

  tprint("Check iwd0, iodd, ieven\n");
  imovk(a, 0);
  imovk(b, iwd0(a)); tcheckd(b, "0", "iwd0 0", 1);
  imovk(b, iodd(a)); tcheckd(b, "0", "iodd 0", 1);
  imovk(b, ieven(a)); tcheckd(b, "1", "ieven 0", 1);
  imovk(a, 230000);
  imovk(b, iwd0(a)); tcheckd(b, "230000", "iwd0 230000", 2);
  imovk(b, iodd(a)); tcheckd(b, "0", "iodd 230000", 2);
  imovk(b, ieven(a)); tcheckd(b, "1", "ieven 230000", 2);
  imovk(a, -230000);
  imovk(b, iwd0(a)); tcheckd(b, "230000", "iwd0 -230000", 3);
  imovk(b, iodd(a)); tcheckd(b, "0", "iodd -230000", 3);
  imovk(b, ieven(a)); tcheckd(b, "1", "ieven -230000", 3);
  imovd(a, "3333333333333333333333333333333333333");
  imovk(b, iwd0(a)); tcheckd(b, "1431655765", "iwd0 333...333", 4);
  imovk(b, iodd(a)); tcheckd(b, "1", "iodd 111...111", 4);
  imovk(b, ieven(a)); tcheckd(b, "0", "ieven 111...111", 4);
  imovd(a, "-3333333333333333333333333333333333333");
  imovk(b, iwd0(a)); tcheckd(b, "1431655765", "iwd0 -333...333", 5);
  imovk(b, iodd(a)); tcheckd(b, "1", "iodd -111...111", 5);
  imovk(b, ieven(a)); tcheckd(b, "0", "ieven -111...111", 5);

  tprint("Check ilen, ibit\n");
  imovk(a, 0);
  imovk(b, ilen(a)); tcheckd(b, "0", "ilen 0", 1);
  imovk(b, ibit(a,  0)); tcheckd(b, "0", "ibit 0, 0", 1);
  imovk(b, ibit(a,  1)); tcheckd(b, "0", "ibit 0, 1", 1);
  imovk(b, ibit(a, 31)); tcheckd(b, "0", "ibit 0, 31", 1);
  imovk(b, ibit(a, 32)); tcheckd(b, "0", "ibit 0, 32", 1);
  ttbit("0");
  imovk(a, 2300001);
  imovk(b, ilen(a)); tcheckd(b, "22", "ilen 2300001", 1);
  imovk(b, ibit(a,   0)); tcheckd(b, "1", "ibit 2300001, 0", 1);
  imovk(b, ibit(a,   1)); tcheckd(b, "0", "ibit 2300001, 1", 1);
  imovk(b, ibit(a,  21)); tcheckd(b, "1", "ibit 2300001, 21", 1);
  imovk(b, ibit(a,  22)); tcheckd(b, "0", "ibit 2300001, 22", 1);
  imovk(b, ibit(a, 500)); tcheckd(b, "0", "ibit 2300001, 500", 1);
  ttbit("2300001");
  imovd(a, "3333333333333333333333333333333333333");
  imovk(b, ilen(a)); tcheckd(b, "122", "ilen 333...333", 1);
  imovk(b, ibit(a,   0)); tcheckd(b, "1", "ibit 333...333, 0", 1);
  imovk(b, ibit(a,   1)); tcheckd(b, "0", "ibit 333...333, 1", 1);
  imovk(b, ibit(a, 121)); tcheckd(b, "1", "ibit 333...333, 21", 1);
  imovk(b, ibit(a, 122)); tcheckd(b, "0", "ibit 333...333, 22", 1);
  imovk(b, ibit(a, 500)); tcheckd(b, "0", "ibit 333...333, 500", 1);
  ttbit("3333333333333333333333333333333333333");

  tprint("Check icmp\n");
  /* All small */
  imovk(a, 5); imovk(b, 3); imovk(c, -4); imovk(d, -6);
  imovk(e, 0); e->flags = 0; imovk(f, 0); f->flags = NegativeReg;
  imovk(g, 5); imovk(h, -4);
  imovk(x, icmp(a, g)); tcheckd(x, "0", "icmp 5 = 5", 11);
  imovk(x, icmp(c, h)); tcheckd(x, "0", "icmp -4 = -4", 11);
  imovk(x, icmp(a, b)); tcheckd(x, "1", "icmp 5 > 3", 11);
  imovk(x, icmp(a, b)); tcheckd(x, "1", "icmp 5 > 3", 11);
  imovk(x, icmp(a, c)); tcheckd(x, "1", "icmp 5 > -4", 11);
  imovk(x, icmp(a, d)); tcheckd(x, "1", "icmp 5 > -6", 11);
  imovk(x, icmp(c, d)); tcheckd(x, "1", "icmp -4 > -6", 11);
  imovk(x, icmp(c, a)); tcheckd(x,"-1", "icmp -4 < 5", 11);
  imovk(x, icmp(c, b)); tcheckd(x,"-1", "icmp -4 < 3", 11);
  imovk(x, icmp(a, e)); tcheckd(x, "1", "icmp 5 > 0", 11);
  imovk(x, icmp(a, f)); tcheckd(x, "1", "icmp 5 > -0", 11);
  imovk(x, icmp(c, e)); tcheckd(x,"-1", "icmp -4 < 0", 11);
  imovk(x, icmp(c, f)); tcheckd(x,"-1", "icmp -4 < -0", 11);
  imovk(x, icmp(e, a)); tcheckd(x,"-1", "icmp 0 < 5", 11);
  imovk(x, icmp(f, a)); tcheckd(x,"-1", "icmp -0 < 5", 11);
  imovk(x, icmp(e, c)); tcheckd(x, "1", "icmp 0 > -4", 11);
  imovk(x, icmp(f, c)); tcheckd(x, "1", "icmp -0 > -4", 11);
  imovk(x, icmp(e, e)); tcheckd(x, "0", "icmp 0 = 0", 11);
  imovk(x, icmp(e, f)); tcheckd(x, "0", "icmp 0 = -0", 11);
  imovk(x, icmp(f, e)); tcheckd(x, "0", "icmp -0 = 0", 11);
  imovk(x, icmp(f, f)); tcheckd(x, "0", "icmp -0 = -0", 11);
  /* a, c, e small, b, d, f, g, h large */
  uextend(b, 21); xmovk(&b->value[1], 0, 20);
  uextend(d, 21); xmovk(&d->value[1], 0, 20);
  uextend(f, 21); xmovk(&f->value[1], 0, 20);
  uextend(g, 21); xmovk(&g->value[1], 0, 20);
  uextend(h, 21); xmovk(&h->value[1], 0, 20);
  imovk(x, icmp(a, g)); tcheckd(x, "0", "icmp 5 = 5", 15);
  imovk(x, icmp(c, h)); tcheckd(x, "0", "icmp -4 = -4", 15);
  imovk(x, icmp(a, b)); tcheckd(x, "1", "icmp 5 > 3", 15);
  imovk(x, icmp(a, d)); tcheckd(x, "1", "icmp 5 > -6", 15);
  imovk(x, icmp(c, b)); tcheckd(x,"-1", "icmp -4 < 3", 15);
  imovk(x, icmp(c, d)); tcheckd(x, "1", "icmp -4 > -6", 15);
  imovk(x, icmp(b, a)); tcheckd(x,"-1", "icmp 3 < 5", 51);
  imovk(x, icmp(d, b)); tcheckd(x,"-1", "icmp -6 < 5", 51);
  imovk(x, icmp(b, c)); tcheckd(x, "1", "icmp 3 > -4", 51);
  imovk(x, icmp(d, c)); tcheckd(x,"-1", "icmp -6 < 4", 51);
  imovk(x, icmp(a, e)); tcheckd(x, "1", "icmp 5 > 0", 11);
  imovk(x, icmp(a, f)); tcheckd(x, "1", "icmp 5 > -0", 15);
  imovk(x, icmp(c, e)); tcheckd(x,"-1", "icmp -4 < 0", 11);
  imovk(x, icmp(c, f)); tcheckd(x,"-1", "icmp -4 < -0", 15);
  imovk(x, icmp(e, a)); tcheckd(x,"-1", "icmp 0 < 5", 11);
  imovk(x, icmp(f, a)); tcheckd(x,"-1", "icmp -0 < 5", 51);
  imovk(x, icmp(e, c)); tcheckd(x, "1", "icmp 0 > -4", 11);
  imovk(x, icmp(f, c)); tcheckd(x, "1", "icmp -0 > -4", 51);
  imovk(x, icmp(e, e)); tcheckd(x, "0", "icmp 0 = 0", 11);
  imovk(x, icmp(e, f)); tcheckd(x, "0", "icmp 0 = -0", 15);
  imovk(x, icmp(f, e)); tcheckd(x, "0", "icmp -0 = 0", 51);
  imovk(x, icmp(f, f)); tcheckd(x, "0", "icmp -0 = -0", 55);
  /* All large */
  uextend(a, 21); xmovk(&a->value[1], 0, 20);
  uextend(c, 21); xmovk(&c->value[1], 0, 20);
  uextend(e, 21); xmovk(&e->value[1], 0, 20);
  imovk(x, icmp(a, g)); tcheckd(x, "0", "icmp 5 = 5", 55);
  imovk(x, icmp(c, h)); tcheckd(x, "0", "icmp -4 = -4", 55);
  imovk(x, icmp(a, b)); tcheckd(x, "1", "icmp 5 > 3", 55);
  imovk(x, icmp(a, c)); tcheckd(x, "1", "icmp 5 > -4", 55);
  imovk(x, icmp(a, d)); tcheckd(x, "1", "icmp 5 > -6", 55);
  imovk(x, icmp(c, d)); tcheckd(x, "1", "icmp -4 > -6", 55);
  imovk(x, icmp(c, a)); tcheckd(x,"-1", "icmp -4 < 5", 55);
  imovk(x, icmp(c, b)); tcheckd(x,"-1", "icmp -4 < 3", 55);
  imovk(x, icmp(a, e)); tcheckd(x, "1", "icmp 5 > 0", 55);
  imovk(x, icmp(a, f)); tcheckd(x, "1", "icmp 5 > -0", 55);
  imovk(x, icmp(c, e)); tcheckd(x,"-1", "icmp -4 < 0", 55);
  imovk(x, icmp(c, f)); tcheckd(x,"-1", "icmp -4 < -0", 55);
  imovk(x, icmp(e, a)); tcheckd(x,"-1", "icmp 0 < 5", 55);
  imovk(x, icmp(f, a)); tcheckd(x,"-1", "icmp -0 < 5", 55);
  imovk(x, icmp(e, c)); tcheckd(x, "1", "icmp 0 > -4", 55);
  imovk(x, icmp(f, c)); tcheckd(x, "1", "icmp -0 > -4", 55);
  imovk(x, icmp(e, e)); tcheckd(x, "0", "icmp 0 = 0", 55);
  imovk(x, icmp(e, f)); tcheckd(x, "0", "icmp 0 = -0", 55);
  imovk(x, icmp(f, e)); tcheckd(x, "0", "icmp -0 = 0", 55);
  imovk(x, icmp(f, f)); tcheckd(x, "0", "icmp -0 = -0", 55);

  tprint("Check icmpk\n");
  /* All small */
  imovk(a, 5); imovk(b, 3); imovk(c, -4); imovk(d, -6);
  imovk(e, 0); e->flags = 0; imovk(f, 0); f->flags = NegativeReg;
  imovk(g, 5); imovk(h, -4);
  imovk(x, icmpk(a,  5)); tcheckd(x, "0", "icmpk 5 = 5", 11);
  imovk(x, icmpk(c, -4)); tcheckd(x, "0", "icmpk -4 = -4", 11);
  imovk(x, icmpk(a,  3)); tcheckd(x, "1", "icmpk 5 > 3", 11);
  imovk(x, icmpk(a,  3)); tcheckd(x, "1", "icmpk 5 > 3", 11);
  imovk(x, icmpk(a, -4)); tcheckd(x, "1", "icmpk 5 > -4", 11);
  imovk(x, icmpk(a, -6)); tcheckd(x, "1", "icmpk 5 > -6", 11);
  imovk(x, icmpk(c, -6)); tcheckd(x, "1", "icmpk -4 > -6", 11);
  imovk(x, icmpk(c,  5)); tcheckd(x,"-1", "icmpk -4 < 5", 11);
  imovk(x, icmpk(c,  3)); tcheckd(x,"-1", "icmpk -4 < 3", 11);
  imovk(x, icmpk(a,  0)); tcheckd(x, "1", "icmpk 5 > 0", 11);
  imovk(x, icmpk(a, -0)); tcheckd(x, "1", "icmpk 5 > -0", 11);
  imovk(x, icmpk(c,  0)); tcheckd(x,"-1", "icmpk -4 < 0", 11);
  imovk(x, icmpk(c, -0)); tcheckd(x,"-1", "icmpk -4 < -0", 11);
  imovk(x, icmpk(e,  5)); tcheckd(x,"-1", "icmpk 0 < 5", 11);
  imovk(x, icmpk(f,  5)); tcheckd(x,"-1", "icmpk -0 < 5", 11);
  imovk(x, icmpk(e, -4)); tcheckd(x, "1", "icmpk 0 > -4", 11);
  imovk(x, icmpk(f, -4)); tcheckd(x, "1", "icmpk -0 > -4", 11);
  imovk(x, icmpk(e,  0)); tcheckd(x, "0", "icmpk 0 = 0", 11);
  imovk(x, icmpk(e, -0)); tcheckd(x, "0", "icmpk 0 = -0", 11);
  imovk(x, icmpk(f,  0)); tcheckd(x, "0", "icmpk -0 = 0", 11);
  imovk(x, icmpk(f, -0)); tcheckd(x, "0", "icmpk -0 = -0", 11);
  /* a, c, e small, b, d, f, g, h large */
  uextend(b, 21); xmovk(&b->value[1], 0, 20);
  uextend(d, 21); xmovk(&d->value[1], 0, 20);
  uextend(f, 21); xmovk(&f->value[1], 0, 20);
  uextend(g, 21); xmovk(&g->value[1], 0, 20);
  uextend(h, 21); xmovk(&h->value[1], 0, 20);
  imovk(x, icmpk(a,  5)); tcheckd(x, "0", "icmpk 5 = 5", 11);
  imovk(x, icmpk(c, -4)); tcheckd(x, "0", "icmpk -4 = -4", 11);
  imovk(x, icmpk(a,  3)); tcheckd(x, "1", "icmpk 5 > 3", 11);
  imovk(x, icmpk(a,  3)); tcheckd(x, "1", "icmpk 5 > 3", 11);
  imovk(x, icmpk(a, -4)); tcheckd(x, "1", "icmpk 5 > -4", 11);
  imovk(x, icmpk(a, -6)); tcheckd(x, "1", "icmpk 5 > -6", 11);
  imovk(x, icmpk(c, -6)); tcheckd(x, "1", "icmpk -4 > -6", 11);
  imovk(x, icmpk(c,  5)); tcheckd(x,"-1", "icmpk -4 < 5", 11);
  imovk(x, icmpk(c,  3)); tcheckd(x,"-1", "icmpk -4 < 3", 11);
  imovk(x, icmpk(a,  0)); tcheckd(x, "1", "icmpk 5 > 0", 11);
  imovk(x, icmpk(a, -0)); tcheckd(x, "1", "icmpk 5 > -0", 11);
  imovk(x, icmpk(c,  0)); tcheckd(x,"-1", "icmpk -4 < 0", 11);
  imovk(x, icmpk(c, -0)); tcheckd(x,"-1", "icmpk -4 < -0", 11);
  imovk(x, icmpk(e,  5)); tcheckd(x,"-1", "icmpk 0 < 5", 11);
  imovk(x, icmpk(f,  5)); tcheckd(x,"-1", "icmpk -0 < 5", 11);
  imovk(x, icmpk(e, -4)); tcheckd(x, "1", "icmpk 0 > -4", 11);
  imovk(x, icmpk(f, -4)); tcheckd(x, "1", "icmpk -0 > -4", 11);
  imovk(x, icmpk(e,  0)); tcheckd(x, "0", "icmpk 0 = 0", 11);
  imovk(x, icmpk(e, -0)); tcheckd(x, "0", "icmpk 0 = -0", 11);
  imovk(x, icmpk(f,  0)); tcheckd(x, "0", "icmpk -0 = 0", 11);
  imovk(x, icmpk(f, -0)); tcheckd(x, "0", "icmpk -0 = -0", 11);
  /* All large */
  uextend(a, 21); xmovk(&a->value[1], 0, 20);
  uextend(c, 21); xmovk(&c->value[1], 0, 20);
  uextend(e, 21); xmovk(&e->value[1], 0, 20);
  imovk(x, icmpk(a,  5)); tcheckd(x, "0", "icmpk 5 = 5", 11);
  imovk(x, icmpk(c, -4)); tcheckd(x, "0", "icmpk -4 = -4", 11);
  imovk(x, icmpk(a,  3)); tcheckd(x, "1", "icmpk 5 > 3", 11);
  imovk(x, icmpk(a,  3)); tcheckd(x, "1", "icmpk 5 > 3", 11);
  imovk(x, icmpk(a, -4)); tcheckd(x, "1", "icmpk 5 > -4", 11);
  imovk(x, icmpk(a, -6)); tcheckd(x, "1", "icmpk 5 > -6", 11);
  imovk(x, icmpk(c, -6)); tcheckd(x, "1", "icmpk -4 > -6", 11);
  imovk(x, icmpk(c,  5)); tcheckd(x,"-1", "icmpk -4 < 5", 11);
  imovk(x, icmpk(c,  3)); tcheckd(x,"-1", "icmpk -4 < 3", 11);
  imovk(x, icmpk(a,  0)); tcheckd(x, "1", "icmpk 5 > 0", 11);
  imovk(x, icmpk(a, -0)); tcheckd(x, "1", "icmpk 5 > -0", 11);
  imovk(x, icmpk(c,  0)); tcheckd(x,"-1", "icmpk -4 < 0", 11);
  imovk(x, icmpk(c, -0)); tcheckd(x,"-1", "icmpk -4 < -0", 11);
  imovk(x, icmpk(e,  5)); tcheckd(x,"-1", "icmpk 0 < 5", 11);
  imovk(x, icmpk(f,  5)); tcheckd(x,"-1", "icmpk -0 < 5", 11);
  imovk(x, icmpk(e, -4)); tcheckd(x, "1", "icmpk 0 > -4", 11);
  imovk(x, icmpk(f, -4)); tcheckd(x, "1", "icmpk -0 > -4", 11);
  imovk(x, icmpk(e,  0)); tcheckd(x, "0", "icmpk 0 = 0", 11);
  imovk(x, icmpk(e, -0)); tcheckd(x, "0", "icmpk 0 = -0", 11);
  imovk(x, icmpk(f,  0)); tcheckd(x, "0", "icmpk -0 = 0", 11);
  imovk(x, icmpk(f, -0)); tcheckd(x, "0", "icmpk -0 = -0", 11);
  /* UBASIC ======================================
   10   A=999999999
   20   B=222222222
   30   C=A:D=A:E=A:F=A
   70   for I=1 to 10001
   80   C+=222222222
   90   D-=222222222
  100   E+=-222222222
  110   F-=-222222222
  120   next
  130   print C:print D:print E:print F
  170   for I=1 to 101
  200     H=-(B+17)
  210     C+=B:D-=B
  230     E+=H:F-=H
  250     B*=3
  260   next
  270   print C:print D:print E:print F
  280   I=0
  290     C+=I:F-=I
  300   I+=1234567: if I<=1000000000 then 290
  310   print C:print D
  End of UBASIC =============================== */

  tprint("Fundamental operations: multiply, divide and related functions\n");

  tprint("Various large-number checks for isqu and imul\n");
  imov(a, ieval("10000! * 20000! * 30000!"));
  imov(h, ieval("0"));
  for (i = 2; i <= 10000; i++) 
    { idivk(a, i); imovk(b, kres); isqu(b); iadd(h, b); }
  tcheckd(h, "0", "imul! kres", i);
  imov(h, ieval("0"));
  for (i = 2; i <= 20000; i++) 
    { idivk(a, i); imovk(b, kres); isqu(b); iadd(h, b); }
  imov(h, ieval("0"));
  tcheckd(h, "0", "imul! kres", i);
  for (i = 2; i <= 30000; i++) 
    { idivk(a, i); imovk(b, kres); isqu(b); iadd(h, b); }
  tcheckd(h, "0", "imul! kres", i);
  tcheckd(a, "1", "imul!", i); 
  imov(a, ieval("10000!^2"));
  imov(h, ieval("0"));
  for (i = 2; i <= 10000; i++) 
    { idivk(a, i*i); imovk(b, kres); isqu(b); iadd(h, b); }
  tcheckd(h, "0", "isqu! kres", i);
  tcheckd(a, "1", "isqu!", i); 
  imov(a, ieval("10^100000 * 10^200000"));
  imov(b, ieval("10^150000 * 10^150000"));
  imov(c, ieval("(10^150000)^2"));
  imov(h, ieval("10^300000"));
  isub(a, h);
  tcheckd(a, "0", "imul10^x", 100000);
  isub(b, h);
  tcheckd(b, "0", "imul10^x", 150000);
  isub(c, h);
  tcheckd(c, "0", "isqu10^x", 150000);
 
  tprint("Check isqu, imul, imulk, idivk\n");
  imovk(a, 999999999);
  imovk(b, 222222222);
  imov(c, a);
  for (i = 2; i <= 64; i <<= 1)
    {
    isqu(c);
    tprint("c = a^%d ", i); tidisp(c); tprint("\n");
    if (i == 2) { tcheckd(c, "999999998000000001", "isqu", i); }
    if (i == 4) { tcheckd(c, "999999996000000005999999996000000001", "isqu", i); }
    }
  imov(d, a); imov(e, a); 
  for (i = 2; i <= 64; i++)
    {
    imul(d, a); imulk(e, 999999999);
    if (i == 2) { tcheckd(d, "999999998000000001", "imul", i); }
    if (i == 2) { tcheckd(e, "999999998000000001", "imulk", i); }
    if (i == 3) { tcheckd(d, "999999997000000002999999999", "imul", i); }
    if (i == 3) { tcheckd(e, "999999997000000002999999999", "imulk", i); }
    if (i == 4) { tcheckd(d, "999999996000000005999999996000000001", "imul", i); }
    if (i == 4) { tcheckd(e, "999999996000000005999999996000000001", "imulk", i); }
    }
  tcheckr(d, c, "imul/imulk dc", 0); tcheckr(e, c, "imul/imulk ec", 0); 
  for (i = 64; i > 0; i--)
    {
    if (i == 4) { tcheckd(c, "999999996000000005999999996000000001", "idivk1", i); }
    idivk(c, 999999999); imovk(h, kres); tcheckd(h, "0", "idivk1 kres", i);
    }
  tcheckd(c, "1", "idivk1", i);
  for (i = 64*4; i > 32*4; i--)
    { idivk(d, 3); imovk(h, kres); tcheckd(h, "0", "idivk2a kres", i); }
  for (i = 64; i > 32; i--)
    { idivk(d, 37); imovk(h, kres); tcheckd(h, "0", "idivk2b kres", i); }
  for (i = 64; i > 32; i--)
    { idivk(d, 333667); imovk(h, kres); tcheckd(h, "0", "idivk2c kres", i); }
  for (i = 32; i > 0; i--)
    {
    if (i == 4) { tcheckd(d, "999999996000000005999999996000000001", "idivk3", i); }
    idivk(d, 3); imovk(h, kres); tcheckd(h, "0", "idivk3a kres", i);
    idivk(d, 3); imovk(h, kres); tcheckd(h, "0", "idivk3b kres", i);
    idivk(d, 3); imovk(h, kres); tcheckd(h, "0", "idivk3c kres", i);
    idivk(d, 3); imovk(h, kres); tcheckd(h, "0", "idivk3d kres", i);
    idivk(d, 37); imovk(h, kres); tcheckd(h, "0", "idivk3e kres", i);
    idivk(d, 333667); imovk(h, kres); tcheckd(h, "0", "idivk3f kres", i);
    }
  tcheckd(d, "1", "idivk3", i);
  for (i = 63*4; i > 0; i--)
    { idivk(e, 3); imovk(h, kres); tcheckd(h, "0", "idivk4a kres", i); }
  for (i = 63; i > 0; i--)
    { idivk(e, 37); imovk(h, kres); tcheckd(h, "0", "idivk4b kres", i); }
  for (i = 63; i > 0; i--)
    { idivk(e, 333667); imovk(h, kres); tcheckd(h, "0", "idivk4c kres", i); }
  tcheckd(e, "999999999", "idivk4", i);
  
  tprint("Check isqu imul imulk, divk, imodk signs\n");
  imovk(a, 100000); isqu(a); tcheckd(a, "10000000000", "isqu+", 1);
  imovk(a,-100000); isqu(a); tcheckd(a, "10000000000", "isqu-", 2);
  imovk(a, 100000); imovk(b, 23); imul(a, b); tcheckd(a, "2300000", "imul++", 1);
  imovk(a, 100000); imovk(b,-23); imul(a, b); tcheckd(a,"-2300000", "imul+-", 2);
  imovk(a,-100000); imovk(b, 23); imul(a, b); tcheckd(a,"-2300000", "imul-+", 3);
  imovk(a,-100000); imovk(b,-23); imul(a, b); tcheckd(a, "2300000", "imul--", 4);
  imovk(a, 100000); imulk(a, 23); tcheckd(a, "2300000", "imulk++", 1);
  imovk(a, 100000); imulk(a,-23); tcheckd(a,"-2300000", "imulk+-", 2);
  imovk(a,-100000); imulk(a, 23); tcheckd(a,"-2300000", "imulk-+", 3);
  imovk(a,-100000); imulk(a,-23); tcheckd(a, "2300000", "imulk--", 4);
  imovk(a, 100000);
  idivk(a, 100); imovk(b, kres);
  tcheckd(a, "1000", "idivk 0+", 1); tcheckd(b, "0", "kres 0+", 1);
  imovk(a,-100000);
  idivk(a, 100); imovk(b, kres);
  tcheckd(a,"-1000", "idivk 0-", 2); tcheckd(b, "0", "kres 0-", 2);
  imovk(a, 100000);
  idivk(a, 77); imovk(b, kres);
  tcheckd(a, "1298", "idivk +", 3); tcheckd(b, "54", "kres +", 3);
  imovk(a,-100000);
  idivk(a, 77); imovk(b, kres);
  tcheckd(a,"-1298", "idivk +", 4); tcheckd(b, "-54", "kres +", 4);
  imovk(a, 100000);
  imodk(a, 100); 
  tcheckd(a, "0", "imodk 0+", 1);
  imovk(a,-100000);
  imodk(a, 100);
  tcheckd(a, "0", "imodk 0-", 2);
  imovk(a, 100000);
  imodk(a, 77);
  tcheckd(a, "54", "imodk +", 3);
  imovk(a,-100000);
  imodk(a, 77); 
  tcheckd(a, "23", "imodk +", 4);

  tprint("Check usep2k, imul2k, idiv2k, imod2k\n");
  imovd(h, "11111");
  for (i = 0; i <= 20; i++)
    {
    ttest2k(1, i);
    }
  imovd(h, "1111111111");
  for (i = 0; i <= 35; i++)
    {
    ttest2k(2, i);
    }
  imovd(h, "11111111112222222222333333333344444444445555555555");
  for (i = 0; i <= 200; i++)
    {
    ttest2k(3, i);
    }

  tprint("Check idiv and imod\n");
  ttestdivmod("22", "7", 160, 100);
  ttestdivmod("0", "1", 80, 100);
  ttestdivmod("1001", "13", 80, 100);
  ttestdivmod("1", "987654321", 80, 100);
  ttestdivmod("7", "13", 80, 100);
  ttestdivmod("999999999", "99999991", 80, 100);
  ttestdivmod("888888888999999999", "99999991", 80, 100);
  ttestdivmod("777777777888888888999999999", "99999991", 80, 100);
  ttestdivmod("666666666777777777888888888999999999", "99999991", 80, 100);
  ttestdivmod("555555555666666666777777777888888888999999999", "99999991", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "2", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "22", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "222", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "2222", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "22222", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "222222", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "2222222", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "22222222", 80, 100);
  ttestdivmod("11111111111111111111111111111111111111111111111111111111", "222222222", 80, 100);
  ttestdivmod("18446744073709551615", "65535", 80, 128);
  ttestdivmod("340282366920938463426481119284349108225", "65535", 80, 128);
  ttestdivmod("18446744073709551615", "65536", 80, 128);
  ttestdivmod("340282366920938463426481119284349108225", "65536", 80, 128);
  ttestdivmod("18446744073709551615", "65537", 80, 128);
  ttestdivmod("340282366920938463426481119284349108225", "65537", 80, 128);
  ttestdivmod("18446744073709551616", "65535", 80, 128);
  ttestdivmod("340282366920938463426481119284349108226", "65535", 80, 128);
  ttestdivmod("18446744073709551616", "65536", 80, 128);
  ttestdivmod("340282366920938463426481119284349108226", "65536", 80, 128);
  ttestdivmod("18446744073709551616", "65537", 80, 128);
  ttestdivmod("340282366920938463426481119284349108226", "65537", 80, 128);
  ttestdivmod("18446744073709551617", "65535", 80, 128);
  ttestdivmod("340282366920938463426481119284349108227", "65535", 80, 128);
  ttestdivmod("18446744073709551617", "65536", 80, 128);
  ttestdivmod("340282366920938463426481119284349108227", "65536", 80, 128);
  ttestdivmod("18446744073709551617", "65537", 80, 128);
  ttestdivmod("340282366920938463426481119284349108227", "65537", 80, 128);
  ttestdivmod("18446744073709551615", "1073741823", 80, 128);
  ttestdivmod("340282366920938463426481119284349108225", "1073741823", 80, 128);
  ttestdivmod("18446744073709551615", "1073741824", 80, 128);
  ttestdivmod("340282366920938463426481119284349108225", "1073741824", 80, 128);
  ttestdivmod("18446744073709551615", "1073741825", 80, 128);
  ttestdivmod("340282366920938463426481119284349108225", "1073741825", 80, 128);
  ttestdivmod("18446744073709551616", "1073741823", 80, 128);
  ttestdivmod("340282366920938463426481119284349108226", "1073741823", 80, 128);
  ttestdivmod("18446744073709551616", "1073741824", 80, 128);
  ttestdivmod("340282366920938463426481119284349108226", "1073741824", 80, 128);
  ttestdivmod("18446744073709551616", "1073741825", 80, 128);
  ttestdivmod("340282366920938463426481119284349108226", "1073741825", 80, 128);
  ttestdivmod("18446744073709551617", "1073741823", 80, 128);
  ttestdivmod("340282366920938463426481119284349108227", "1073741823", 80, 128);
  ttestdivmod("18446744073709551617", "1073741824", 80, 128);
  ttestdivmod("340282366920938463426481119284349108227", "1073741824", 80, 128);
  ttestdivmod("18446744073709551617", "1073741825", 80, 128);
  ttestdivmod("340282366920938463426481119284349108227", "1073741825", 80, 128);
  ttestdivmod("18446744073709551615", "2147483647", 80, 128);
  ttestdivmod("340282366920938463426481119284349108225", "2147483647", 80, 128);
  ttestdivmod("18446744073709551616", "2147483647", 80, 128);
  ttestdivmod("340282366920938463426481119284349108226", "2147483647", 80, 128);
  ttestdivmod("18446744073709551617", "2147483647", 80, 128);
  ttestdivmod("340282366920938463426481119284349108227", "2147483647", 80, 128);
  ttestdivmod("17458334880", "3764973290", 80, 3);

  tcheckr(a, a, "END", 0);
  tprint("All done\n"); halt();
  }  

void ttestdivmod(char *n, char *m, long to, long mult)
  /* check idiv and imod */
  {
  static long i, k;

  imovd(a, n);                        /* a =  n         */
  imovd(b, m);                        /* b =  m         */
  k = b->value[0];  
  imov (c, a); umodk(c, k);           /* c =  (n mod m) */
  imov (d, a); udivk(d, k);           /* d =  [n / m]   */
  imov (e, c); ineg(e);               /* e = -(n mod m) */
  imov (f, d); ineg(f);               /* f = -[n / m]   */
  imov (g, c); ineg(g); umodk(g, k);  /* g = -(n mod m) */
  if (isgn(g) < 0) { iadd(g, b); }    /* g = (-n mod m) */
  
  for (i = 0; i <= to; i++)
    {
    tprint("n=%s m=%s \n", n, m); 
    imov(x, a); imov(y, x); imov(z, b); 
    imod(x, z); tcheckr(x, c, "imod A+", i);
    idiv(y, z);
    tcheckr(y, d, "idivq A+", i); tcheckr(res, c, "idivr A+", i);
    imov(x, a); ineg(x); imov(y, x);
    imod(x, z); tcheckr(x, g, "imod A-", i);
    idiv(y, z);
    tcheckr(y, f, "idivq A-", i); tcheckr(res, e, "idivr A-", i);
    imulk(a, mult); imulk(b, mult);
    imulk(c, mult); imulk(e, mult); imulk(g, mult);
    }
  }

void ttest2k(long k, long i)
  /* check "2k" functions: usep2k, imul2k, idiv2k, imod2k */
  {
  static long j, ki;

  /* d = 2^i             */
  /* e = h mod 2^i       */
  /* f = [h / 2^i]       */
  /* g = [h * 2^i]       */
  ki = k*10000 + i;
  imovk(d, 1); imov(e, h); imov(f, h); imov(g, h); 
  for (j = 1; j <= i; j++) { imulk(d, 2); }
  for (j = 1; j <= i; j++) { idivk(f, 2); }
  imov(x, f); imul(x, d); isub(e, x);            
  imul(g, d);

  imov(a, h);    
  usep2k(a, b, i);
  tcheckr(a, e, "usep2k m", ki); tcheckr(b, f, "usep2k q", ki);
  imov(a, h);    
  imul2k(a, i);
  tcheckr(a, g, "imul2k", ki);
  idiv2k(a, i);
  tcheckr(a, h, "imul2k idiv2k", ki);
  imov(a, h);    
  idiv2k(a, i);
  tcheckr(a, f, "idiv2k", ki);
  imov(a, h);    
  imod2k(a, i);
  tcheckr(a, e, "imod2k", ki);
  imov(a, h);   ineg(a);  
  imod2k(a, i); ineg(a);  
  if (i) { iadd (a, d); }
  tcheckr(a, e, "imod2k neg", ki);
  }

void ttbit(char *dig)
  /* check ibit and ilen */
  {
  static long i;

  imovd(a, dig);
  imovk(b, 0);
  for (i = 0; i < ilen(a); i++)
    {
    imovk(c, ibit(a, i)); imul2k(c, i); 
    iadd (b, c);
    }
  tcheckd(b, dig, "ibit", 0);
  }
