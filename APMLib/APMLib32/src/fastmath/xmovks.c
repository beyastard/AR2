#include "fastmath.h"

// a = k,  k sign extended  (k = 32-bit integer)
//
// xmovks (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: d(a) > 0
void xmovks(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zsg,0FFFFFFFFh
		mov    [EAX],EDX         ; Move k to a
        dec    EBX
        jz     LmovkX
        SAR    EDX,31   
        and    EDX,Zsg           ; EDX = s sign bits
LmovkD:
		mov    [EAX+4],EDX       ; Extend with sign
        lea    EAX,[EAX+4]
        dec    EBX
        jg     LmovkD
LmovkX:
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}
