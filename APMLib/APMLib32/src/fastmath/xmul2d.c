#include "fastmath.h"

// a = a * 2^(32*d)  (d = 32-bit integer)
//
// xmul2d (&a, d, d(a));
//
// Registers:   EAX = &a, EDX = d, EBX = d(a)
// Assumptions: d(a) > 0, c(a) >= d(a) + d
void xmul2d(long* a, long d, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,d
		mov    ebx,da
		
		cmp    EDX,0
        jle    Lmul2d8            ; d = 0: nothing to do
        mov    Zcx,ECX
        lea    EAX,[EAX+EBX*4]    ; EAX = &a_d
Lmul2d4:
		mov    ECX,[EAX-4]        ; EAX = &a{i+1}, ECX = a_i
        mov    [EAX+4*EDX-4],ECX  ; Multiply by 2^(32*d)
        dec    EBX
        lea    EAX,[EAX-4]
        jg     Lmul2d4

        mov    ECX,Zcx

		cmp    EDX,0
        jle    Lmovz8a
Lmovz2a:
		mov    dword ptr [EAX],0 
        dec    EDX
        lea    EAX,[EAX+4]
        jg     Lmovz2a
Lmovz8a:
Lmul2d8:
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}
