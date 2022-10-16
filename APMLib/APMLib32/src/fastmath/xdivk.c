#include "fastmath.h"

// a = [a / k]  (k = 32-bit integer)
//
// r = xdivk (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: k > 0, d(a) > 0
// Returns:     (a mod k) in EAX
long xdivk(long* a, long k, long da)
{
	long result;
	
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zdi,EDI

        lea    EDI,[EAX+EBX*4-4]  ; EDI = &a_{d-1}
        mov    ECX,EDX            ; ECX = k
        xor    EDX,EDX            ; EDX = 0
Ldivk2:
		mov    EAX,[EDI]          ; EAX = a_i
        div    ECX                ; EAX = [(2^32*EDX + a_i)/k]
        mov    [EDI],EAX          ; EDX = (2^32*EDX + a_i) mod k]
        sub    EDI,4
        dec    EBX
        jg     Ldivk2
        mov    EAX,EDX            ; Remainder

        mov    ECX,Zcx
        mov    EDI,Zdi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    result,eax
	}
	
	return result;
}
