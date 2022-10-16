#include "fastmath.h"

// flags(a - b)
//
// flags = xcmp  (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a), d(b) > 0
// Returns:     EAX = SignFlag    0x00008000
//                    ZeroFlag    0x00004000
//                    AuxFlag     0x00001000
//                    ParityFlag  0x00000200
//                    CarryFlag   0x00000100
long xcmp(long* a, long* b, long da, long db)
{
	long result;
	
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

		mov    Zsi,ESI
        mov    Zdi,EDI
        lea    ESI,[EAX+EBX*4-4] ; ESI = &a_{d(a)-1}
        lea    EDI,[EDX+ECX*4-4] ; EDI = &b_{d(b)-1}
        sub    ECX,EBX
        je     LcmpK             ; same size
        jb     LcmpF             ; b shorter than a

; a is shorter than b, EBX = d(a), ECX = d(b) - d(a)
        mov    EAX,[ESI]
        sar    EAX,31
        and    EAX,Zsg           ; EAX = a sign
LcmpD:
		mov    EDX,[EDI]         ; Pick up b_i
        cmp    EAX,EDX           ; Compare a sign with b_i
        lea    EDI,[EDI-4]
        jne    LcmpX
        dec    ECX
        jnz    LcmpD
        jmp    LcmpK

; b is shorter than a, EBX = d(a), ECX = -(d(a) - d(b))
LcmpF:
		add    EBX,ECX           ; EBX = d(a)
        mov    EDX,[EDI]
        sar    EDX,31
        and    EDX,Zsg           ; EDX = b sign
LcmpH:
		mov    EAX,[ESI]         ; Pick up a_i
        cmp    EAX,EDX           ; Compare a_i with b sign
        lea    ESI,[ESI-4]
        jne    LcmpX
        inc    ECX
        jnz    LcmpH
LcmpK:
		mov    EAX,[ESI]         ; same size
        mov    EDX,[EDI]
        lea    ESI,[ESI-4]
        cmp    EAX,EDX           ; Compare a_i with b_i
        lea    EDI,[EDI-4]
        jne    LcmpX
        dec    EBX
        jg     LcmpK
        cmp    EAX,EDX           ; Equal
LcmpX:
		lahf   
        mov    EDI,Zdi
        mov    ESI,Zsi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
	}
	
	return result;
}
