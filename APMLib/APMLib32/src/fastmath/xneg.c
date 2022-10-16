#include "fastmath.h"

// a = 2^(32*d(a)) - a
//
// xneg (&a, d(a));
//
// Registers:   EAX = &a, EDX = d(a)
// Assumptions: d(a) > 0
void xneg(long* a, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    eax,a
		mov    edx,da
		
		mov    Zbx,EBX
        mov    EBX,[EAX]         ; EBX = a_0
        not    EBX
        add    EBX,1             ; EBX = -a_0
        dec    EDX
        mov    [EAX],EBX
        jz     LnegX
LnegB:
		mov    EBX,[EAX+4]       ; EBX = a_i
        not    EBX               ; Flip a_i bits
        adc    EBX,0             ; Add carry
        dec    EDX
        mov    [EAX+4],EBX
        lea    EAX,[EAX+4]
        jg     LnegB
LnegX:
		mov    EBX,Zbx
        
		mov    edx,Zdx
	}	
}
