/* AR2TEST3.C  2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2f.h";
#include "ar2t.h";
#include "ar2f77.h";
#include "ar2f80.h";
#include "ar2f88.h";
#include "ar2f89.h";
#include "ar2f8A.h";
#include "ar2fA0.h";

void ttestmersenne(void (fsqu)(ireg *a), char *id);
long tmersenne (long k, void (fsqu)(ireg *a));
void ttestfermatm2ke(void (fsqu)(ireg *a), char *id);
long tfermatm2ke(long m, long k, ireg *e, long g, void (fsqu)(ireg *a));
void ttestfsquA(void (fsqu)(ireg *a), long k0, long km, long kd, long il, char *id);
void ttestfsquB(void (fsqu)(ireg *a), long k0, long km, long kd, long il, char *id);
void ttestfsquC(void (fsqu)(ireg *a), long k0, long km, long kd, long il, char *id);

static ireg ra, rb, rc, rd, re, rv, rw, rx, ry, rz;
static ireg *a = &ra; static ireg *b = &rb;
static ireg *c = &rc; static ireg *d = &rd;
static ireg *e = &re; 
static ireg *v = &rv; static ireg *w = &rw;
static ireg *x = &rx; static ireg *y = &ry;
static ireg *z = &rz;

void main()
  {

  static long i;
  tprint( "ARITH2 TEST 3  Version 2.1.6m\n");
 
  tprint("Check imovd and tidisp\n");
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
    isqufftA0(v);
    tidisp(v); tprint(" v=10^2^%d, d(v)=%d\n", i, v->digits);
    }
  checkFFTerror = 1;
  ttestfsquA(isqufft77,     3480, 1, 2, 100, "A isqufft77");
  ttestfsquA(isqufft88,    15360, 1, 2, 100, "A isqufft88");
  ttestfsquA(isqufft89,    30720, 1, 2,  50, "A isqufft89");
  ttestfsquA(isqufft8A,    61440, 1, 2,  25, "A isqufft8A");
  ttestfsquA(isqufft80,    61440, 1, 2,  25, "A isqufft80");
  ttestfsquA(isqufftA0,   245760, 1, 2,  20, "A isqufftA0");
  ttestfsquA(tsqufft,     245760, 1, 2,  20, "A tsqufft");
  ttestfsquA(tsqumulsch,   31440, 1, 2,  50, "A tsqumulsch");
  ttestfsquA(tsqumulfft,   31440, 1, 2,  50, "A tsqumulfft");
  ttestfsquB(isqufft77,     3483, 7, 8,  40, "B isqufft77");
  ttestfsquB(isqufft88,    15363, 7, 8,  40, "B isqufft88");
  ttestfsquB(isqufft89,    30723, 7, 8,  20, "B isqufft89");
  ttestfsquB(isqufft8A,    61443, 7, 8,  20, "B isqufft8A");
  ttestfsquB(isqufft80,    61443, 7, 8,  20, "B isqufft80");
  ttestfsquB(isqufftA0,   245763, 7, 8,  15, "B isqufftA0");
  ttestfsquB(tsqufft,     245763, 7, 8,  15, "B tsqufft");
  ttestfsquB(tsqumulfft,  245763, 7, 8,  15, "B tsqumulfft");
  ttestfsquC(isqufftA0,  1032192, 3, 4,  50, "C isqufftA0");
  ttestfsquC(tsqufft,    1000003, 3, 4,  30, "C tsqufft");
  ttestfsquC(tsqumulfft, 1000003, 3, 4,  30, "C tsqumulfft");
  ttestmersenne(isqufft77, "M isqufft77");
  ttestfermatm2ke(isqufft77, "F isqufft77");
  ttestmersenne(isqufft80, "M isqufft80");
  ttestfermatm2ke(isqufft80, "F isqufft80");
  ttestmersenne(isqufftA0, "M isqufftA0");
  ttestfermatm2ke(isqufftA0, "F isqufftA0");
  ttestmersenne(tsqufft, "M tsqufft");
  ttestfermatm2ke(tsqufft, "F tsqufft");

  tcheckr(a, a, "END", 0);
  tprint("All done\n"); halt();
  }  
  
void ttestfsquA(void (fsqu)(ireg *a), long k0, long km, long kd, long il, char *id)
  {
  static long i, k;
  tprint("Check fsqu: FFT %s\n", id);
  for (k = k0; k >= 1; k *= km, k /= kd)
    {
    imovd(a, "123456789"); imov(b, a);
    imovd(c, "987654321"); imov(d, c);
    for (i = 1; i <= il; i++)
      {
      b->flags = NegativeReg;
      fsqu(a);   tsqusch(b);
      tcheckr(a, b, id, 1000*k + i);
      imod2k(a, k);
      imod2k(b, k);
      tcheckr(a, b, id, 1000*k + i);
      c->flags = NegativeReg;
      fsqu(c);   imov(e, d);   imul(d, e);
      tcheckr(c, d, id, - 1000*k - i);
      imod2k(c, k);
      imod2k(d, k);
      tcheckr(c, d, id, - 1000*k - i);
      }
    }
  }

void ttestfsquB(void (fsqu)(ireg *a), long k0, long km, long kd, long il, char *id)
  {
  static long i, k;
  tprint("Check fsqu: FFT %s\n", id);
  for (k = k0; k >= 256; k *= km, k /= kd)
    {
    imovd(a, "123456789"); imov(b, a);
    imovd(c, "987654321"); imov(d, c);
    for (i = 1; i <= il; i++)
      {
      b->flags = NegativeReg;
      fsqu(a); tsqusch(b);
      tcheckr(a, b, id, 1000*k + i);
      imod2k(a, k);
      imod2k(b, k);
      tcheckr(a, b, id, 1000*k + i);
      c->flags = NegativeReg;
      fsqu(c); imov(e, d); imul(d, e);
      tcheckr(c, d, id, - 1000*k - i);
      imod2k(c, k);
      imod2k(d, k);
      tcheckr(c, d, id, - 1000*k - i);
      }
    }
  }

void ttestfsquC(void (fsqu)(ireg *a), long k0, long km, long kd, long il, char *id)
  {
  static long i, k;
  tprint("Check fsqu: FFT %s\n", id);
  for (k = k0; k >= 1; k *= km, k /= kd)
    {
    imovd(a, "111222333"); imov(b, a);
    for (i = 1; i <= il; i++)
      {
      fsqu(a);
      imod2k(a, k-1);
      fsqu(b);
      imod2k(b, k);
      imov(c, b);
      imod2k(c, k-1);
      tcheckr(a, c, id, 1000*k + i);
      }
    }
  }

void ttestmersenne(void (fsqu)(ireg *a), char *id)
  {
  tprint("Mersenne primes\n");
  
  imovk(a, tmersenne(83, fsqu)); tcheckd(a, "0", id, 0);
  imovk(a, tmersenne(89, fsqu)); tcheckd(a, "1", id, 0);
  imovk(a, tmersenne(107, fsqu)); tcheckd(a, "1", id, 0);
  imovk(a, tmersenne(127, fsqu)); tcheckd(a, "1", id, 0);
  imovk(a, tmersenne(521, fsqu)); tcheckd(a, "1", id, 0);
  imovk(a, tmersenne(607, fsqu)); tcheckd(a, "1", id, 0);
  imovk(a, tmersenne(1279, fsqu)); tcheckd(a, "1", id, 0);
  imovk(a, tmersenne(2203, fsqu)); tcheckd(a, "1", id, 0);
  imovk(a, tmersenne(2281, fsqu)); tcheckd(a, "1", id, 0);
  imovk(a, tmersenne(3217, fsqu)); tcheckd(a, "1", id, 0);
  }

long tmersenne (long k, void (fsqu)(ireg *a))
  /* 
     Lucas-Lehmer test for Mersenne primes, using squaring function fsqu

     Returns  1 if 2^k - 1 is prime, k > 64
              0 if 2^k - 1 is composite, k > 64
             -1 if k <= 64
  */
  {
  static ireg ta, tb;
  static ireg *a = &ta; static ireg *b = &tb;
  static da, db, i, d;
  if (k <= 64) { return(-1); }

  d  = (k+31)/32;
  db = d + 8;
  da = 2 * db;
  if (b->value == NULL) { ualloc(b, db, 1); }
  if (a->value == NULL) { ualloc(a, da, 1); }
  imovk(a, 4);  
  for (i = 2; i <= k - 1; i++)
    {
    fsqu   (a);
    usep2k (a, b, k);
    iadd   (a, b);
    isubk  (a, 2);
    utrim  (a);
    if (a->digits > d) 
      {
      usep2k (a, b, k);
      iadd   (a, b);
      utrim  (a);
      }
    }
    /* Reduce mod 2^k - 1 again */
    iaddk  (a, 1);
    usep2k (a, b, k);
    iadd   (a, b);
    /* And again */
    usep2k (a, b, k);
    iadd   (a, b);
    isubk  (a, 1);
    if (isgn(a) == 0)
      { tprint("2^%d - 1 is prime\n", k); return(1); }
    else
      { tprint("2^%d - 1 is composite\n", k); return(0); }
  }
  
void ttestfermatm2ke(void (fsqu)(ireg *a), char *id)
  {
  tprint("Fermat test for probable primes\n");

  imovk(e, 1); imovk(a, tfermatm2ke(3, 66, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(3, 189, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(3, 201, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(3, 209, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(3, 276, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(3, 353, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(3, 408, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(3, 438, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(3, 534, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(5, 75, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(5, 85, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(5, 1947, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 92, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 120, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 174, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 180, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 290, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 320, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 390, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 432, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 616, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 830, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(7, 1804, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 65, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 67, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 81, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 134, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 162, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 206, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 211, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 366, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 663, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 782, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 1305, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 1411, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(9, 1494, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(11, 81, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(11, 125, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(11, 127, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(11, 209, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(11, 211, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(13, 82, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(13, 188, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(13, 308, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(13, 316, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(13, 1000, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 78, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 112, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 168, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 229, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 297, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 339, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 517, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 522, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 654, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 900, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(15, 1518, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(17, 147, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(17, 243, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(17, 267, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(17, 347, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(17, 471, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(17, 747, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(19, 366, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(19, 1246, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 124, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 128, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 129, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 187, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 209, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 276, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 313, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 397, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 899, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 1532, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 1613, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(21, 1969, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(23, 69, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(23, 73, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(23, 341, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(23, 381, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(23, 389, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(23, 649, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(23, 1961, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 78, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 184, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 232, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 268, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 340, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 448, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 554, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 664, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 740, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 748, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 1280, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 1328, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(25, 1640, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(27, 92, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(27, 175, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(27, 215, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(27, 275, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(27, 407, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(27, 455, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(27, 1076, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(27, 1090, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 75, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 77, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 93, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 103, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 143, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 185, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 231, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 245, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 391, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 1053, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(29, 1175, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(31, 68, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(31, 140, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(31, 216, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(31, 416, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(31, 1808, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 66, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 93, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 118, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 289, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 412, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 453, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 525, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 726, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 828, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 1420, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(33, 1630, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(35, 147, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(35, 245, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(35, 327, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(35, 355, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(35, 663, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(35, 1423, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(35, 1443, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 68, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 82, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 84, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 106, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 110, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 166, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 236, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 254, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 286, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 290, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 712, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 1240, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 1706, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 1804, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(37, 1904, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 67, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 70, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 71, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 73, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 251, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 370, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 375, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 389, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 407, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 518, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 818, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 865, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 1057, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(39, 1602, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(41, 215, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(41, 289, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(41, 379, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(41, 1991, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(43, 94, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(43, 98, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(43, 104, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(43, 144, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(43, 158, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(43, 252, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(43, 778, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(43, 1076, e, 101, fsqu)); tcheckd(a, "1", id, 0);

  imovk(e, 3); imovk(a, tfermatm2ke(1, 67, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 84, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 228, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 390, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 784, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 1110, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 1704, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 2008, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 2139, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 2191, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 2367, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 3); imovk(a, tfermatm2ke(1, 2370, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(400047, 416, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-1); imovk(a, tfermatm2ke(400047, 416, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-5); imovk(a, tfermatm2ke(400047, 416, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(656523, 544, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-1); imovk(a, tfermatm2ke(656523, 544, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-5); imovk(a, tfermatm2ke(656523, 544, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(437637, 896, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-1); imovk(a, tfermatm2ke(437637, 896, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-5); imovk(a, tfermatm2ke(437637, 896, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 17); imovk(a, tfermatm2ke(1, 3381, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 17); imovk(a, tfermatm2ke(1, 4441, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-17); imovk(a, tfermatm2ke(1, 3632, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-17); imovk(a, tfermatm2ke(1, 4062, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-1); imovk(a, tfermatm2ke(1961020893, 3664, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(1961020893, 3664, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e,-1); imovk(a, tfermatm2ke(1666962633, 3672, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovk(e, 1); imovk(a, tfermatm2ke(1666962633, 3672, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "1301655396715"); imovk(a, tfermatm2ke(1, 1056, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "1301655396717"); imovk(a, tfermatm2ke(1, 1056, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "1301655396721"); imovk(a, tfermatm2ke(1, 1056, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "1301655396723"); imovk(a, tfermatm2ke(1, 1056, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "3405302217"); imovk(a, tfermatm2ke(1, 1664, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "3405302221"); imovk(a, tfermatm2ke(1, 1664, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "3405302223"); imovk(a, tfermatm2ke(1, 1664, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "2294808807"); imovk(a, tfermatm2ke(1, 2016, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "2294808811"); imovk(a, tfermatm2ke(1, 2016, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "2294808813"); imovk(a, tfermatm2ke(1, 2016, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "14906370057"); imovk(a, tfermatm2ke(1, 2400, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "14906370061"); imovk(a, tfermatm2ke(1, 2400, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  imovd(e, "14906370063"); imovk(a, tfermatm2ke(1, 2400, e, 101, fsqu)); tcheckd(a, "1", id, 0);
  }


long tfermatm2ke(long m, long k, ireg *e, long g, void (fsqu)(ireg *a))
  /* 
    Fermat test for q = m*2^k + e, using squaring function fsqu

    Assumes m*2^k + e > 0, k > 64
    Returns 1 if g^q == g (mod q), k > 64
            0 if g^q != g (mod q), k > 64
           -1 if k <= 64
  */
  {
  if (k <= 64) { return(-1); }
  if (isgn(e) < 0)
    { tprint("%d*2^%d - ", m, k); ineg(e); tidisp(e); ineg(e); }
  else
    { tprint("%d*2^%d + ", m, k); tidisp(e); }
  if (ffermatm2ke(m, k, e, g, fsqu) == 1)
    { tprint(" is probably prime\n"); return(1); }
  else
    { tprint(" is composite\n"); return(0); }
  }

