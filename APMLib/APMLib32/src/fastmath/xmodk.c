#include "fastmath.h"

// a = (a mod k)  (k = 32-bit integer)
//
// xmod (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: k > 0, d(a) > 0
void xmodk(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zdi,EDI

        lea    EDI,[EAX+EBX*4-4]  ; EDI = &a_{d-1}
        mov    ECX,EDX            ; ECX = k
        xor    EDX,EDX            ; EDX = 0
Lmodk2:
		mov    EAX,[EDI]          ; EAX = a_i
        div    ECX                ; EAX = [(2^32*EDX + a_i)/k]
        sub    EDI,4
        dec    EBX
        jg     Lmodk2
        mov    [EDI+4],EDX        ; Remainder

        mov    ECX,Zcx
        mov    EDI,Zdi
        
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}
