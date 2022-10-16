#include "fastmath.h"

// a = a * 2^(32*d)  (d = 32-bit integer)
//
// xdiv2d (&a, d, d(a));
//
// Registers:   EAX = &a, EDX = d, EBX = d(a)
// Assumptions: 0 <= d < d(a), d(a) > 0
void xdiv2d(long* a, long d, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,d
		mov    ebx,da
		
		cmp    EDX,0
        jle    Ldiv2d8            ; d = 0: nothing to do
        mov    Zcx,ECX
        sub    EBX,EDX            ; EBX = d(a) - d
Ldiv2d4:
		mov    ECX,[EAX+EDX*4]    ; EAX = &a, ECX = a_i
        mov    [EAX],ECX          ; Divide by 2^(32*d)
        dec    EBX
        lea    EAX,[EAX+4]
        jg     Ldiv2d4

        mov    ECX,Zcx
Ldiv2d8:
		
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}
