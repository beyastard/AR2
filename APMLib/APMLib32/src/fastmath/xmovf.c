#include "fastmath.h"

// a = b, same size
//
// xmovf (&a, &b, d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(b)
// Assumptions: d(a) = d(b) > 0
void xmovf(long* a, long* b, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		
		mov    Zcx,ECX
LmovfB:
		mov    ECX,[EDX]         ; ECX = b_i
        lea    EDX,[EDX+4]
        mov    [EAX],ECX         ; a_i = b_i
        lea    EAX,[EAX+4]
        dec    EBX
        jg     LmovfB
        mov    ECX,Zcx
		
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
	}
}
