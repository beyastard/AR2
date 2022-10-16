#include "fastmath.h"

// a = a + b (mod 2^(32*d(a)))
//
// xaddf (&a, &b, d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(b)
// Assumptions: d(a) = d(b) > 0
void xaddf(long* a, long* b, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zbp,EBP

        clc                      ; clear carry
LaddfB:
		mov    EBP,[EDX]         ; EBP = b_i
        mov    ECX,[EAX]         ; ECX = a_i
        lea    EDX,[EDX+4]
        adc    ECX,EBP           ; ECX = a_i + b_i + carry 
        dec    EBX
        mov    [EAX],ECX         ; Move to a
        lea    EAX,[EAX+4]
        jg     LaddfB            ; More digits

        mov    EBP,Zbp
        mov    ECX,Zcx
        
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}
