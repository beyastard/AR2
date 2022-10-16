#include "fastmath.h"

// a = a + b (mod 2^(32*d(a)))
//
// f = xadd  (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a) >= d(b) > 0
// Returns:     Result sign bit EAX bit 31
//              Carry flag      EAX bit  8
long xadd(long* a, long* b, long da, long db)
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
		mov    Zbp,EBP
        mov    Zdi,EDI

        mov    EDI,EAX           ; EDI = &a
        sub    EBX,ECX           ; EBX = d(a) - d(b)
        clc                      ; clear carry
LaddB:
		mov    EBP,[EDX]         ; EBP = b_i
        mov    EAX,[EDI]         ; EAX = a_i
        lea    EDX,[EDX+4]
        adc    EAX,EBP           ; EAX = a_i + b_i + carry 
        dec    ECX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LaddB             ; More digits
        lahf                     ; Carry flag to AH

        cmp    EBX,0 
        jbe    LaddX             ; d(a) = d(b)
        sar    EBP,31 
        and    EBP,Zsg           ; EBP = b sign bits
        sahf                     ; Restore carry
LaddD:
		mov    EAX,[EDI]         ; EAX = a_i
        adc    EAX,EBP           ; EAX = a_i + b sign bits + carry 
        dec    EBX
        mov    [EDI],EAX         ; Move to a
        lea    EDI,[EDI+4]
        jg     LaddD             ; More sign bits
        lahf                     ; Carry flag to AH
LaddX:
		mov    EDI,Zdi
        mov    EBP,Zbp
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
		mov    result,eax
	}
	
	return result;
}
