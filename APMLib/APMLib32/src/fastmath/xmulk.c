#include "fastmath.h"

// a = a * k,  (k = 32-bit integers)
//
// xmulk (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: d(a) > 0, c(a) > d(a)
void xmulk(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zbp,EBP
        mov    Zsi,ESI

        mov    ESI,EAX            ; ESI = &a
        mov    ECX,EDX            ; ECX = k
        xor    EBP,EBP
Lmulk2:
		mov    EAX,[ESI]          ; EAX = a_i
        mul    ECX                ; Multiply by k
        add    EAX,EBP            ; Add carry
        adc    EDX,0  
        mov    EBP,EDX            ; Save carry
        mov    [ESI],EAX
        lea    ESI,[ESI+4]
        dec    EBX
        jg     Lmulk2
        mov    [ESI],EDX          ; Final high order digit

        mov    ESI,Zsi
		
        mov    EBP,Zbp
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}
