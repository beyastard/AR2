#include "wmod.h"

// a (mod b)
//
// Registers:   eax = a, edx = b.
// Assumptions: -2^31 <= a < 2^31, 1 < b < 2^31.
// Return:      eax = a (mod b).
long wmods(long a, long b)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		eax,a
		mov		edx,b

		push	ebx
		mov 	ebx,edx
		mov 	edx,eax      ; Extend sgn(a) to edx 
		sar 	edx,31 
		idiv	ebx          ; Signed integer division 
		cmp 	edx,0
		jnl 	wmods5
		add 	edx,ebx      ; Add b if negative
wmods5:
		mov		eax,edx
		pop		ebx
		
		mov		edx,Wdx
		mov		result,eax
	}

	return result;
}
