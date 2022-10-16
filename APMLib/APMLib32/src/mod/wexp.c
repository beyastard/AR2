#include "wmod.h"

// a^d
//
// Registers:   eax = a, edx = d.
// Assumptions: 0 <= a, d < 2^32.
// Return:      eax = a^d.
long wexp(long a, long b)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		eax,a
		mov		edx,b
		
		push	ecx
        push	esi
        push	edi
        mov 	ecx,eax                ; ecx = a
        mov 	esi,edx                ; esi = d
        mov 	edi,32
        mov 	eax,1
wexp2:
		cmp 	eax,1
        jna 	wexp4
        mul 	eax
wexp4:
		shl 	esi,1
        jnc 	wexp6
        mul 	ecx
wexp6:
		dec 	edi
        jnz 	wexp2
        pop 	edi
        pop 	esi
        pop 	ecx
		
		mov		edx,Wdx
		mov		result,eax
	}

	return result;
}
