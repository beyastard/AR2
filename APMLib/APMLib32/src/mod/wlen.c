#include "wmod.h"

// (log a)/(log 2) + 1
//
// Registers:   eax = a.
// Assumptions: 0 <= a < 2^32.
// Return:      eax = length of a.
long wlen(long a)
{
	long result;

	__asm
	{
		mov		eax,a
		
		push	edx
        mov 	edx,eax
        xor 	eax,eax
        and 	edx,edx
        jz  	wlen4
wlen2:
		shr 	edx,1
        inc 	eax
        jnz 	wlen2
wlen4:
		pop 	edx
        
		mov		result,eax
	}

	return result;
}
