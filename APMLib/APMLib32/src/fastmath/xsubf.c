#include "fastmath.h"

// a = a - b (mod 2^(32*d(a)))
//
// xsubf (&a, &b, d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(b)
// Assumptions: d(a) = d(b) > 0
void xsubf(long* a, long* b, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zdi,EDI

        clc                      ; clear carry
LsubfB:
		mov    EDI,[EDX]         ; EDI = b_i
        mov    ECX,[EAX]         ; ECX = a_i
        lea    EDX,[EDX+4]
        sbb    ECX,EDI           ; ECX = a_i - b_i - borrow
        dec    EBX
        mov    [EAX],ECX         ; Move to a
        lea    EAX,[EAX+4]
        jg     LsubfB            ; More digits

        mov    EDI,Zdi
        mov    ECX,Zcx
        
		mov    edx,Zdx
		mov    ebx,Zbx
	}	
}
