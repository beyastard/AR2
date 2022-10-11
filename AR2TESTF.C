/* AR2TESTF.C  2.2.1F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <conio.h>;
#include <stdlib.h>;
#include <time.h>
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2t.h";
#include "ar2f.h";
#include "ar2f76.h";

static ireg ra, rb, rc, rf12;
static ireg *a = &ra; static ireg *b = &rb; static ireg *c = &rc; 
static ireg *f12 = &rf12; 

static clock_t sclock, tclock;
static double secs;
static long i, j, ca, da, f, tblock;

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

void test_76_debug()
  {
  for(i = 1; ; i++)
    {
    a->digits = da;
    tidisp (a); tprint(" = a before i=%d d=%d \n", i, a->digits);
    xsqumodF12(a->value, a->value);
    tidisp (a); tprint(" = a after i=%d d=%d \n", i, a->digits);
    for (j = 64; j < 64; j++)
      {
      ielement(c, b, j, 5);
      if (isgn(c) == 0) continue;
      tidisp(c);
      tprint(" = X(%d) i=%d \n", j, i);
      if (test_quit()) halt(0);
      }
    halt(0);
    }
  }

void test_76_time()
  {

  for(i = 1; ; i++)
    {
    tblock=10000;
    a->digits = da; utrim(a);
    iaddk(a, i);
    imov (c, a);
    sclock = clock();
    for (j = 1; j < tblock; j++) { xsqumodF12(a->value, a->value); }
    tclock = clock(); secs = (tclock - sclock); secs /= CLOCKS_PER_SEC; 
    a->digits = da; utrim (a);
    tprint("xsqumodF12: i=%d d=%d time for %d: %9.7f sec\n",
                        i, a->digits, tblock, secs);
    sclock = clock();
    for (j = 1; j < tblock; j++) { isqu(c); imod(c, f12); }
    utrim(c);
    tclock = clock(); secs = (tclock - sclock); secs /= CLOCKS_PER_SEC; 
    tprint("isqu/imod:  i=%d d=%d time for %d: %9.7f sec\n",
                        i, c->digits, tblock, secs);
    if (icmp(a, c) != 0)
      {
      tprint("Error (a != c)\n"); f = 1; 
      halt(1);
      }
    if (test_quit()) halt(0);
    }
  }

void main()
  {
  tprint("ARITH2 TEST AR2F76A.ASM  Version 2.1.6m\n");
 
  tprint("AR2TESTF \n"); 

  ca = 129; da = 129;
  if (a->value == NULL) ualloc (a, ca, 1);
  if (a->capacity < ca) uextend(a, ca);
  for (i = 0; i < da; i++) a->value[i] = 0;
  for (i = 0; i < 128; i++) a->value[i] = i >> 1;
  imovk(f12, 1); imul2k(f12, 4096); iaddk(f12, 1);

  test_76_time();

  }
