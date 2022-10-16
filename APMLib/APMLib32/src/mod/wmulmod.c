#include "wmod.h"

// a*d (mod b)
//
// Registers:   eax = a, edx = d, ebx = c.
// Assumptions: 0 <= a < 2^32, 0 < b < 2^32, a*d < b*2^32.
// Return:      eax = a*d (mod b).
long wmulmod(long a, long d, long c)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		Wbx,ebx
		mov		eax,a
		mov		edx,d
		mov		ebx,c
		
		mul		edx
        div		ebx
        mov		eax,edx
        
		mov		edx,Wdx
		mov		ebx,Wbx
		mov		result,eax
	}

	return result;
}
