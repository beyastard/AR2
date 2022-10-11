/* AR2TESTC.C  2.2.1F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <conio.h>;
#include <stdlib.h>;
#include <time.h>
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2t.h";
#include "ar2f.h";
#include "ar2fC0.h";

static ireg ra, rb, rc;
static ireg *a = &ra; static ireg *b = &rb; static ireg *c = &rc; 

static clock_t sclock, tclock;
static double secs;
static long i, j, ca, da, cb, db, f;

void ielement(ireg *y, ireg *x, long n, long d)
  /*
     y = element n of x, 

     where each element consists of d digits
  */
  {
  static long i, j;
  if (y->capacity < d) uextend(y, d);
  for (i = n*d, j = 0; i < x->digits && j < d; i++, j++)
    y->value[j] = x->value[i];
  if (j) 
    { y->digits = j; y->flags = 0; }
  else 
    imovk(y, 0);
  }

void test_C0_debug()
  {
  ca = 258048; da = 129024;
  cb = 528513; db = 528513;

  if (a->value == NULL) ualloc (a, ca, 1);
  if (a->capacity < ca) uextend(a, ca);
  a->value[0] = 3;
  for (i = 1; i < da; i++)
    a->value[i] = 0;
  if (b->value == NULL) ualloc (b, cb, db);
  if (b->capacity < cb) uextend(b, cb);

  for(i = 1; ; i++)
    {
    a->digits = da;
    tidisp (a); tprint(" = a before i=%d d=%d \n", i, a->digits);
    xsqufftC0(a->value, da, b->value);
    tidisp (a); tprint(" = a after i=%d d=%d \n", i, a->digits);
    for (j = 0; j < 4096; j++)
      {
      ielement(c, b, j, 129);
      if (isgn(c) == 0) continue;
      tidisp(c);
      tprint(" = X(%d) i=%d \n", j, i);
      if (test_quit()) halt();
      }
    if (test_quit()) halt();
    }
  }

void test_C0_time()
  {
  imovk(a,77);
  imovk(b,77);

  for(i = 1; ; i++)
    {
    utrim (a);
    if (a->digits > 104000) a->digits = 104000; da = a->digits;
    imov (b, a);
    imov (c, a); if (c->digits > 320) c->digits = 320; 
    sclock = clock();
    isqufftC0(a);
    tclock = clock(); secs = (tclock - sclock); secs /= CLOCKS_PER_SEC; 
    tprint("isqufftC0: i=%d d=%d time=%9.7f sec\n", i, da, secs);
    sclock = clock();
    isqu(b);
    tclock = clock(); secs = (tclock - sclock); secs /= CLOCKS_PER_SEC; 
    tprint("isqu:      i=%d d=%d time=%9.7f sec\n", i, da, secs);
    f = 0;
    if (icmp(a, b) != 0) { tprint("Error (ab)\n"); f = 1; }
    isqu(c); if (c->digits > 320) c->digits = 320;
    da = a->digits; if (a->digits > 320) a->digits = 320;
    db = b->digits; if (b->digits > 320) b->digits = 320;
    if (icmp(a, c) != 0) { tprint("Error (ac)\n"); f = 1; }
    if (icmp(b, c) != 0) { tprint("Error (bc)\n"); f = 1; }
    if (f) halt();
    a->digits = da; b->digits = db;
    if (test_quit()) halt();
    }
  }

void main()
  {
  tprint("ARITH2 TEST AR2FC0A.ASM  Version 2.1.6m\n");
 
  tprint("AR2TESTC \n"); 

  test_C0_time();

  }
