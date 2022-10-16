#include "wmod.h"

// a (mod b)
//
// Registers:   eax = a, edx = b.
// Assumptions: 0 <= a < 2^32, 0 < b < 2^32.
// Return:      eax = a (mod b).
long wmod(long a, long b)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		eax,a
		mov		edx,b

		push	ebx
		mov		ebx,edx
		xor		edx,edx
		div		ebx
		mov		eax,edx
		pop		ebx
		
		mov		edx,Wdx
		mov		result,eax
	}

	return result;
}
