#include "fastmath.h"

// a = 0
//
// xmovz  (&a, d(a));
//
// Registers:   EAX = &a, EDX = d(a)
void xmovz(long* a, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    eax,a
		mov    edx,da

		cmp    EDX,0
        jle    Lmovz8
Lmovz2:
		mov    dword ptr [EAX],0 
        dec    EDX
        lea    EAX,[EAX+4]
        jg     Lmovz2
Lmovz8:
		mov    edx,Zdx
	}
}
