#include "fastmath.h"

// Number of significant digits of a
//
// n = xdig (d(a), &a);
//
// Registers:   EAX = d(a), EDX = &a
// Assumptions: d(a) > 0
// Returns:     EAX = number of significant digits of a 
long xdig(long da, long* a)
{
	long result;
	
	__asm
	{		
		mov    Zdx,edx
		mov    eax,da
		mov    edx,a
xdig_:
		cmp    EAX,1
        jbe    Ldig8
        cmp    dword ptr [EDX+EAX*4-4],0
        jne    Ldig8
        dec    EAX
        jmp    xdig_
Ldig8:
		mov    edx,Zdx
		mov    result,eax
	}
	
	return result;
}
