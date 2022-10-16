#include "wmod.h"

// u = 1/x (mod p), y = gcd(x, p)
//
// Registers:   eax = x, edx = p.
// Assumptions: 0 <= x < p < 2^31, p > 0.
// Return:      eax = 1/x (mod p) if gcd(p, x) = 1,
//              eax = 0 otherwise.
long winvmod(long a, long b)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		eax,a
		mov		edx,b
		
		push	edi
        push	esi
        push	ebp
        push	ecx
        push	ebx
        mov 	ebp,edx         ; ebp = p

; See Henri Cohen, "A Course in Computational Algebraic 
; Number Theory", 1.3.2, for details of this algorithm. 
; See AR2I.C for C version.
;
; Numbers can be negative, but never outside range [-p, p], 
; temp2 and temp4 are always >= 0,
; result is always in [-(p-1)/2, (p-1)/2].

        mov		sv_u,1          ; u = 1
        mov		sv_y,eax        ; y = a
        xor		ebx,ebx         ; ebx = temp1 = 0
        mov		ecx,ebp         ; ecx = temp2 = p
wimod2:
		mov 	eax,sv_y 
        xor 	edx,edx         ; edx:eax = temp5 = y
        idiv	ecx             ; eax = [temp5/temp2]
        mov 	esi,edx         ; esi = temp4 = temp5%temp2
        imul	eax,ebx         ; eax = temp5 * temp1
        mov 	edi,sv_u        ; edi = temp3 = u
        sub 	edi,eax         ; edi = temp3 - temp5
        mov 	sv_u,ebx        ; u = temp1
        mov 	sv_y,ecx        ; y = temp2
        mov 	ebx,edi         ; temp1 = temp3
        mov 	ecx,esi         ; temp2 = temp4
        and 	esi,esi
        jnz 	wimod2

        cmp		sv_y,1
        jne		wimod8          ; gcd(x, p) != 1

        mov		eax,sv_u        ; eax = 1/x (mod p)
        cmp		eax,0
        jnl		wimod6
        add		eax,ebp         ; Add p if < 0
wimod6:
		pop		ebx
        pop		ecx
        pop		ebp
        pop		esi
        pop		edi
        
		mov		edx,Wdx
		jmp		wimod9
wimod8:
		xor		eax,eax
        jmp		wimod6
wimod9:
		mov		result,eax
	}

	return result;
}
