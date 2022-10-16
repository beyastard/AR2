#include "intmath.h"

/* Remainder register */
extern ireg rres = { NULL, 0, 0, 0 };
extern ireg *res = &rres;
extern long kres = 0;

/* Temporary registers */
extern ireg rtemp1 = { NULL, 0, 0, 0 };
extern ireg rtemp2 = { NULL, 0, 0, 0 };
extern ireg rtemp3 = { NULL, 0, 0, 0 };
extern ireg rtemp4 = { NULL, 0, 0, 0 };
extern ireg rtemp5 = { NULL, 0, 0, 0 };
extern ireg *temp1 = &rtemp1;
extern ireg *temp2 = &rtemp2;
extern ireg *temp3 = &rtemp3;
extern ireg *temp4 = &rtemp4;
extern ireg *temp5 = &rtemp5;

/* Useful constants */
double ilog2 = 0.69314718055994530941723212145817656807;
double ilog10 = 2.30258509299404568401799145468436420757;
