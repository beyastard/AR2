#include "fastmath.h"

// nxtprm = xnxtprmk(k);
//
// Registers:   EAX = k
// Assumptions: k >= 0
// Returns:     EAX = next prime after k if k < 2^32
//              EAX = 1 otherwise
long xnxtprmk(long k)
{
	long result;
	
	__asm
	{
		mov    eax,k
		
		cmp    EAX,2
        jb     Lnxtp2
        je     Lnxtp3
        mov    Zdx,EDX
        mov    Zbx,EBX
        mov    Zcx,ECX
        mov    EBX,EAX
        dec    EBX
        or     EBX,1
Lnxtpa:
		add    EBX,2      ; next k
        jc     Lnxtpx     ; Return 1 if overlflow
        mov    ECX,1      ; ECX = c
Lnxtpb:
		add    ECX,2      ; Next c
        mov    EAX,EBX
        xor    EDX,EDX
        div    ECX        ; EAX = [k/c], EDX = k mod c
        or     EDX,EDX
        jz     Lnxtpa     ; No remainder
        cmp    ECX,EAX
        jb     Lnxtpb     ; c < quotient
Lnxtpx:
		mov    EAX,EBX    ; Return k
        mov    ECX,Zcx
        mov    EBX,Zbx
        mov    EDX,Zdx
		jmp    Lnxtp4
Lnxtp2:
		mov    EAX,2
		jmp    Lnxtp4
Lnxtp3:
		mov    EAX,3
Lnxtp4:
		mov    result,eax
	}
	
	return result;
}
