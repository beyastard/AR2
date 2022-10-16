#include "fastmath.h"

// a = a + k (mod 2^(32*d)), k = 32-bit integer
//
// xaddk (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d = d(a)
// Assumptions: k >= 0, d(a) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xaddk(long* a, long k, long da)
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
        add    EAX,EDX  
        mov    [EDI],EAX         ; a_0 = a_0 + k
        jnc    LaddkX
        dec    EBX
        jz     LaddkX            ; Only one digit
LaddkK:
		mov    EAX,[EDI+4]       ; a_i
        add    EAX,1             
        mov    [EDI+4],EAX       ; Move to a
        jnc    LaddkX
        dec    EBX
        lea    EDI,[EDI+4]
        jg     LaddkK            ; More
LaddkX:
		lahf    
        mov    EDI,Zdi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
	}
	
	return result;
}
