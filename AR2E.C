/* AR2E.C  2.2.0F  TONY FORBES  October 1999 */

#include <stdio.h>;
#include <stdlib.h>;
#include <string.h>;
#include <conio.h>;
#include "ar2x.h";
#include "ar2u.h";
#include "ar2i.h";
#include "ar2e.h";

#define eval_dim 20
#define eval_stp 32
static ireg eval_rstack[eval_dim];
static ireg *eval_astack[eval_dim];
static long eval_ostack[eval_dim];
static long eval_ptr, eval_lev;
long eval_exp;
char *eval_err;

void eval_break()
  /* 
     Keyboard was hit. Pause if it was "Q".
  */
  {
  if (getch() != 'Q') return;
  while (kbhit()) getch();
  printf("Interrupted: Press X to terminate, or ENTER to continue.\n");
  if (getch() == 'X') 
    {
    printf ("Program terminated \n");    
    exit(0);
    }
  }

void eval_factorial()
  {
  static unsigned long f, i;
  if (isgn(eval_astack[eval_ptr]) <= 0) 
    { imovk(eval_astack[eval_ptr], 1); return; }
  utrim(eval_astack[eval_ptr]);
  if (eval_astack[eval_ptr]->digits > 1)
    { eval_err = "Too large"; return; }
  f = iwd0(eval_astack[eval_ptr]);
  imovk(eval_astack[eval_ptr], 1);
  for (i = 2; i <= f; i++) 
    {
    if (kbhit()) eval_break();
    imulk(eval_astack[eval_ptr], i);
    }
  }

void eval_prime_product()
  {
  static unsigned long f, i;
  if (isgn(eval_astack[eval_ptr]) <= 0) 
    { imovk(eval_astack[eval_ptr], 1); return; }
  utrim(eval_astack[eval_ptr]);
  if (eval_astack[eval_ptr]->digits > 1)
    { eval_err = "Too large"; return; }
  f = iwd0(eval_astack[eval_ptr]);
  imovk(eval_astack[eval_ptr], 1);
  for (i = 2; i <= f; i = xnxtprmk(i))
    {
    if (kbhit()) eval_break();
    imulk(eval_astack[eval_ptr], i);
    }
  }

void eval_compute(long key)
  {
  static long k;
  while (eval_ptr > 0 && *eval_err == NULL)
    {
    if (kbhit()) eval_break();
    if ((eval_lev + key) / 4 > eval_ostack[eval_ptr-1] / 4) break;
    k = eval_ostack[eval_ptr-1] % eval_stp;
    if      (k == 12) iadd(eval_astack[eval_ptr-1], eval_astack[eval_ptr]);
    else if (k == 13) isub(eval_astack[eval_ptr-1], eval_astack[eval_ptr]);
    else if (k == 16) imul(eval_astack[eval_ptr-1], eval_astack[eval_ptr]);
    else if (k == 17) 
      {
      if (isgn(eval_astack[eval_ptr]) == 0) eval_err = "Divide by zero";
      else 
        {
        if (isgn(eval_astack[eval_ptr]) < 0)
          {
          ineg(eval_astack[eval_ptr-1]);
          ineg(eval_astack[eval_ptr]);
          }
        idiv(eval_astack[eval_ptr-1], eval_astack[eval_ptr]);
        }
      }
    else if (k == 18) 
      {
      if (isgn(eval_astack[eval_ptr]) == 0) eval_err = "Divide by zero";
      else 
        {
        if (isgn(eval_astack[eval_ptr]) < 0)
          {
          ineg(eval_astack[eval_ptr-1]);
          ineg(eval_astack[eval_ptr]);
          }
        imod(eval_astack[eval_ptr-1], eval_astack[eval_ptr]);
        }
      }
    else if (k == 20)
      {
      if (isgn(eval_astack[eval_ptr]) <= 0) imovk(eval_astack[eval_ptr-1], 1);
      else
        {
        imov(temp1, eval_astack[eval_ptr-1]); 
        iexp(eval_astack[eval_ptr-1], temp1, eval_astack[eval_ptr]);
        }
      }
    else if (k == 25)
      {
      imov(eval_astack[eval_ptr-1], eval_astack[eval_ptr]);
      ineg(eval_astack[eval_ptr-1]);
      }
    else if (k == 26)
      {
      if      (isgn(eval_astack[eval_ptr]) < 0) eval_err = "Invalid operand";
      else if (isgn(eval_astack[eval_ptr]) == 0) imovk(eval_astack[eval_ptr-1], 0);
      else    isqrt(eval_astack[eval_ptr-1], eval_astack[eval_ptr]);
      }
    if (eval_ptr > 0) eval_ptr--;
    else eval_err = "Syntax error";
    }
  }

void eval_push(long key)
  {
  if (key < 24) eval_compute(key);
  else eval_compute(eval_stp);
  eval_ostack[eval_ptr] = eval_lev + key;
  if (eval_ptr+1 >= eval_dim) 
    { eval_err = "Overflow"; return; }
  eval_ptr++;
  eval_ostack[eval_ptr] = 0;
  eval_astack[eval_ptr] = &(eval_rstack[eval_ptr]);
  imovk(eval_astack[eval_ptr], 0);
  }

ireg *ieval(char *exp)
  {
  /* 
     Return the result of evaluating the expression exp.

     Syntax     1  2  3  4  5  6
                                  
     1 (        Y  Y  Y  N  N  N
     2 prefix   Y  Y  Y  N  N  N
     3 digit    N  N  Y  Y  Y  Y
     4 suffix   N  N  N  Y  Y  Y           
     5 binary   Y  Y  Y  N  N  N
     6 )        N  N  N  Y  Y  Y
  */
  static char *i;
  static long n, t, u;
  eval_astack[0] = &(eval_rstack[0]);
  imovk(eval_astack[eval_ptr], 0);
  eval_lev = eval_stp;
  eval_ptr = 0;
  eval_exp = 0;
  eval_err = "";
  u = 1;
  for (i = exp; *i != NULL; i++)
    {
    t = u;                                  /* t = last type */
    if (*eval_err != NULL) break;
    if (*i <= ' ') continue;                /* White space */
    if (*i == ';') break;                   /* Stop at a semicolon */
    if (*i < '0' || *i > '9') eval_exp = 1; /* It's an expression */
    u = 0;                                  /* u = this type */
    if (t == 1 || t == 2 || t == 5)
      {
      u = 1;
      if (*i == '(')                        /* ( */
        { eval_lev += eval_stp; continue; }
      u = 2;
      if (*i == '+')
        { continue; }                       /* Prefix + */
      if (*i == '-') 
        { eval_push(25); continue; }        /* Prefix - */
      if (*i == 'q') 
        { eval_push(26); continue; }        /* Integer square root */
      }
    if (t == 1 || t == 2 || t == 3 || t == 5)
      {
      u = 3;
      if ('0' <= *i && *i <= '9')           /* Digit */ 
        {
        n = *i - 48;                        /* Convert from ASCII */
        imulk(eval_astack[eval_ptr], 10);
        iaddk(eval_astack[eval_ptr], n);
        continue;
        }
      }
    if (t == 3 || t == 4 || t == 6)
      {
      u = 4;
      if (*i == '!') 
        { eval_factorial(); continue; }     /* N! */
      if (*i == '#') 
        { eval_prime_product(); continue; } /* N# */
      u = 5;
      if (*i == '+')
        { eval_push(12); continue; }        /* Binary + */
      if (*i == '-')
        { eval_push(13); continue; }        /* Binary - */
      if (*i == '*') 
        { eval_push(16); continue; }
      if (*i == '/' || *i == '\\') 
        { eval_push(17); continue; }
      if (*i == '@') 
        { eval_push(18); continue; }
      if (*i == '^') 
        { eval_push(20); continue; }
      u = 6;
      if (*i == ')')                         /* ) */
        {
        if (eval_lev <= eval_stp) 
          { eval_err = "Syntax error"; }
        eval_compute(4); 
        eval_lev -= eval_stp;  
        continue;
        }
      }
    u = 0;
    eval_err = "Syntax error";
    }
  if (eval_lev > eval_stp) eval_err = "Syntax error";
  eval_lev = eval_stp;
  if (*eval_err == NULL) eval_compute(0);
  if (*eval_err != NULL) imovk(eval_astack[0], 0);
  return(eval_astack[0]);
  }
