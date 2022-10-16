#include "fastmath.h"

// high-order digit of a
//
// hdigit = xsig (&a, d(a));
//
// Registers:   EAX = &a, EDX = d(a)
// Assumptions: d(a) > 0
// Returns:     EAX = high order digit of a 
//              (a = 0 iff EAX = 0)
long xsig(long* a, long da)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    eax,a
		mov    edx,da
		
		mov    Zbx,EBX
        lea    EBX,[EAX+EDX*4-4] ; EBX = &a_{d-1}
Lsig2:
		mov    EAX,[EBX]
        lea    EBX,[EBX-4]
        cmp    EAX,0
        jne    Lsig8
        dec    EDX
        jg     Lsig2
Lsig8:
		mov    EBX,Zbx
        
		mov    edx,Zdx
		mov    result,eax
	}
	
	return result;
}
