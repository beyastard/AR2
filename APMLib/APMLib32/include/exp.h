#ifndef __EXP_H__
#define __EXP_H__

#include <stdio.h>
#include <stdlib.h>

#include "fastmath.h"
#include "regmem.h"
#include "intmath.h"

void fexpmod(ireg* a, ireg* b, ireg* c, ireg* q, void (fsqu)(ireg* a));
void fexpmodm2ke(ireg* a, ireg* b, ireg* c, ireg* q, long m, long k, ireg* e, void (fsqu)(ireg* a));
long ffermatm2ke(long m, long k, ireg* e, long g, void (fsqu)(ireg* a));

#endif
