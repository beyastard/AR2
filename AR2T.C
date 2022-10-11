/* AR2T.C  2.2.1F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include <conio.h>;
#include <stdarg.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2f.h";
#include "ar2t.h";

long quit = 0;

FILE *jnl_file = NULL;

long test_quit()
  /* Test keyboard "Q" pressed */
  {
  if (!kbhit()) return(0);
  if (getch() != 'Q') return(0);
  quit = 1;
  return(1);
  }

void halt()
  /* Temporarily or permanently halt the program */
  {
  while (kbhit()) getch();
  fclose(jnl_file); jnl_file = NULL;
  printf("AR2TEST stopped: Press X to terminate, anything else to continue. \n"); 
  if (getch() == 'X') 
    {
    printf ("AR2TEST terminated \n");    
    exit(0);
    }
  printf("AR2TEST restarted \n"); 
  quit = 0;
  }

void file_fail(char *f, char *t)
  /* General file failure */
  {
  printf("Unable to %s file AR2TEST.%s.\n", t, f);
  printf("Stopped: Press ENTER to try again.\n"); halt();
  }

void tprint(char *s, ...)
  {
  /* Print and log things. */
  static va_list arglist;
  if (jnl_file == NULL)
    while ((jnl_file = fopen("AR2TEST.JNL", "a")) == NULL) file_fail("JNL", "open"); 
  va_start(arglist, s); vprintf(s, arglist); va_end(arglist); 
  va_start(arglist, s); vfprintf(jnl_file, s, arglist); va_end(arglist); 
  }

void tidisp(ireg *a)
  {
  /* Print and log an integer register. */
  if (jnl_file == NULL)
    while ((jnl_file = fopen("AR2TEST.JNL", "a")) == NULL) file_fail("JNL", "open"); 
  idisp(a);
  idispf(jnl_file, a);
  }


void tudump (ireg *a)
  /*
     Unformatted print of an integer register
  */
  {
  static long i;
  tprint("c=%d d=%d f=%d &v=%d v=",
          a->capacity, a->digits, a->flags, a->value);
  for (i = 0; i < a->digits; i++)
    {  
    tprint("%d ", (unsigned)a->value[i]);
    }
  tprint("\n");
  }

void tsqusch(ireg *a)
  {
  static long f;
  f = FFTsqu;
  FFTsqu = 999999999;
  isqu(a);
  FFTsqu = f;
  }

void tsqufft(ireg *a)
  {
  static long f;
  f = FFTsqu;
  FFTsqu = 0;
  isqu(a);
  FFTsqu = f;
  }

void tmulsch(ireg *a, ireg *b)
  {
  static long f;
  f = FFTmul;
  FFTmul = 999999999;
  imul(a, b);
  FFTmul = f;
  }

void tmulfft(ireg *a, ireg *b)
  {
  static long f;
  f = FFTmul;
  FFTmul = 0;
  imul(a, b);
  FFTmul = f;
  }

void tsqumulsch(ireg *a)
  {
  static long f;
  f = FFTmul;
  FFTmul = 999999999;
  imov(temp1, a);
  imul(a, temp1);
  FFTmul = f;
  }

void tsqumulfft(ireg *a)
  {
  static long f;
  f = FFTmul;
  FFTmul = 0;
  imov(temp1, a);
  imul(a, temp1);
  FFTmul = f;
  }

void tcheckr (ireg *a, ireg *b, char *id, long ik)
  /* Print "Checksum failed" messgae if a != b */
  /* Print "Fail" / "Pass" messgae if &a = &b  */
  /* Add (b - a)^2 to chk                      */
  {
  static ireg rchk;
  static ireg *chk = &rchk;
  static ireg rchx;
  static ireg *chx = &rchx;
  static checknum = 0;
  if (test_quit()) halt();
  if (chk->value == NULL) { imovk(chk, 0); }
  if (a == b)
    {
    if (isgn(chk)) { tprint("Fail\n"); } else { tprint("Pass!\n"); }
    }
  else
    {
    checknum++;
    imov(chx, a);
    isub(chx, b);
    if (checkFFTerror && maxFFTerror > 0.25)
      {
      tprint("Checksum id %s [%d] number %d FFT error:", id, ik, checknum);  
      tprint("maxFFTerror = %g \n", maxFFTerror);
      halt();
      maxFFTerror = 0;
      }
    if (CheckStop * isgn(chx))
      {
      tprint("Checksum id %s [%d] number %d failed:", id, ik, checknum);  
      tprint("\n  Actual value:  "); tidisp(a); 
      tprint("\n  Correct value: "); tidisp(b); 
      tprint("\n  Difference:    "); tidisp(chx); 
      tprint("\n"); halt();
      }
    else 
      {
      if (CheckLog)
        {
        tprint("Checksum id %s [%d] number %d OK\n", id, ik, checknum);
        if (CheckStop > 1) halt();
        } 
      }
    isqu(chx);
    iadd(chk, chx); 
    }
  }

void tcheckd (ireg *a, char *s, char *id, long ik)
  {
  static ireg rchs;
  static ireg *chs = &rchs;
  imovd(chs, s); 
  tcheckr(a, chs, id, ik);
  }

void tdiag (ireg *a, long i, long j, long k)
  {
  if (CheckDiag)
    {
    tidisp(a);
    tprint(" >> %d %d %d <<\n", i, j, k);
    tudump(a);
    halt();
    }
  }

void tmodm2ke(ireg *a, ireg *q, long m, long k, ireg *e)
  /* 
    a = (a mod q), where q = m*2^k + e

    Assumes 0 <= a < q^2, q = m*2^k + e > 0,
            e < 2^(k/2)

    a =  a_0 + a_1*2^k,  a_1 = a_10 + a_11*m
                     
      =  a_0 + (a_10 + a_11*m)*2^k 
     
      == a_0 - a_11*e + a_10*2^k (mod q),

         where 0 <= a_0 < 2^k, 0 <= a_10 < m
  */
  {
  static long i, s;
  s = isgn(e);
  for (i = 1; i <= 2; i++)
    {
    usep2k(a, res, k);      /* a = a_0, res = a_1 */
    if (isgn(res))
      {
      idivk (res, m);       /* res = a_11, kres = a_10 */
      imul  (res, e);       /* res = e*a_11 */
      isub  (a, res);
      imovk (res, kres);
      imul2k(res, k);       /* res = a_10*2^k */
      iadd  (a, res);
      }
    }
  while (icmp(a, q) >= 0) { isub(a, q); }
  while (isgn(a) < 0)     { iadd(a, q); }
  }

void texpmodm2ke(ireg *a, ireg *b, ireg *c, ireg *q, long m, long k, ireg *e)
  /* 
    a = (b^c mod q), where q = m*2^k + e

    Assumes b, c >= 0, q = m*2^k + e > 0,
            e < 2^(k/2)
  */
  {
  static long i, f;
  static unsigned long j;

  f = 0;
  imovk(a, 1);
  for (i = c->digits - 1; i >= 0; i--)
    {
    for (j = 0x80000000; j >= 1; j >>= 1)
      {
      if (f)
        {
        isqu(a);
        tmodm2ke(a, q, m, k, e);
        }
      if (c->value[i] & j)
        {
        imul(a, b);
        tmodm2ke(a, q, m, k, e);
        f++;
        }
      }
    }
  }

