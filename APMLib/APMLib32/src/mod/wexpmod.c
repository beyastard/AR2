#include "wmod.h"

// a^d (mod b)
//
// Registers:   eax = a, edx = d, ebx = b.
// Assumptions: 0 <= a, d < 2^32, 0 < b < 2^32.
// Return:      eax = a^d (mod b).
long wexpmod(long a, long d, long b)
{
	long result;

	__asm	
	{
		mov		Wdx,edx
		mov		Wbx,ebx
		mov		eax,a
		mov		edx,d
		mov		ebx,b
		
		push	ecx
        push	esi
        push	edi
        mov 	ecx,eax                ; ecx = a
        mov 	esi,edx                ; esi = d
        mov 	edi,32
        mov 	eax,1
wemod2:
		cmp 	eax,1
        jna 	wemod4
        mul 	eax
        div 	ebx
        mov 	eax,edx
wemod4:
		shl 	esi,1
        jnc 	wemod6
        mul 	ecx
        div 	ebx
        mov 	eax,edx
wemod6:
		dec 	edi
        jnz 	wemod2
        pop 	edi
        pop 	esi
        pop 	ecx
        
		mov		edx,Wdx
		mov		ebx,Wbx
		mov		result,eax
	}

	return result;
}
