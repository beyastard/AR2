#include "regmem.h"

// Free integer array memory
void ufree(ireg* a)
{
	free(a->value);
	a->value = NULL;
}
