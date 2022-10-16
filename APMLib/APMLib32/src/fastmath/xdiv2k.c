#include "fastmath.h"

// a = [a/2^k]  (k = 32-bit integer)
//
// xdiv2k (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: 0 <= k < 32*d(a), d(a) > 0
//
// Example: d=6, k=104  
//                       ECX = (k mod 32) = 8
//                        |
// |a a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
// |       |       |       |       |       |       |
// |<---EBX = d - [k/32]-->|<-----EDX = [k/32]---->|
//                                                EAX
void xdiv2k(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    Zsi,ESI
        mov    ECX,EDX
        mov    Zdi,EDI
        and    ECX,31             ; ECX = k mod 32
        shr    EDX,5              ; EDX = [k/32]
        sub    EBX,EDX            ; EBX = d - [k/32]
        dec    EBX
        jz     Ldiv2k6            ; Only 1 digit
Ldiv2k4:
		mov    ESI,[EAX+EDX*4+0]  ; ESI = a_i
        mov    EDI,[EAX+EDX*4+4]  ; EDI = a_{i+1}
        shrd   ESI,EDI,CL         ; Divide by 2^(k mod 32)
        mov    [EAX],ESI          ; Store quotient in a
        dec    EBX
        lea    EAX,[EAX+4]
        jg     Ldiv2k4
Ldiv2k6:
		mov    ESI,[EAX+EDX*4+0]  ; ESI = last a_i
        mov    EDI,Zdi
        shr    ESI,CL             ; Divide by 2^(k mod 32)
        mov    ECX,Zcx
        mov    [EAX],ESI          ; Store last quotient in a
        mov    ESI,Zsi
        
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}
