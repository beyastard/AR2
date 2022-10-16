#include "fastmath.h"

// a = a - k (mod 2^(32*d)), k = 32-bit integer
//
// xsubk (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: k >= 0, d(a) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xsubk(long* a, long k, long da)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zdi,EDI

        mov    EDI,EAX           ; EDI = &a
        mov    EAX,[EDI]
        sub    EAX,EDX  
        mov    [EDI],EAX         ; a_0 = a_0 - k
        jnc    LsubkX
        dec    EBX
        jz     LsubkX            ; Only one digit
LsubkK:
		mov    EAX,[EDI+4]       ; a_i
        sub    EAX,1             
        mov    [EDI+4],EAX       ; Move to a
        jnc    LsubkX
        dec    EBX
        lea    EDI,[EDI+4]
        jg     LsubkK            ; More
LsubkX:
		lahf    
        mov    EDI,Zdi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    result,eax
	}
	
	return result;
}
