#include "fastmath.h"

// a = b
//
// xmov  (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a) >= d(b) > 0
void xmov(long* a, long* b, long da, long db)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,b
		mov    ebx,da
		mov    ecx,db

		mov    Zsg,0
		mov    Zdi,EDI

        sub    EBX,ECX           ; EBX = d(a) - d(b)
LmovB:
		mov    EDI,[EDX]         ; EDI = b_i
        lea    EDX,[EDX+4]
        mov    [EAX],EDI         ; Move to a
        dec    ECX
        lea    EAX,[EAX+4]
        jg     LmovB             ; More digits

        cmp    EBX,0 
        jbe    LmovX             ; d(a) = d(b)
        sar    EDI,31 
        and    EDI,Zsg           ; EDI = b sign bits
LmovD:
		mov    dword ptr[EAX],0  ; Extend with zero
        dec    EBX
        lea    EAX,[EAX+4]
        jg     LmovD             ; More digits
LmovX:
		mov    EDI,Zdi
		
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
	}
}
