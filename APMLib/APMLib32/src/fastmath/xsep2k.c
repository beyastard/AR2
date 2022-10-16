#include "fastmath.h"

// a = a mod 2^k,  q = [a/2^k]  (k = 32-bit integer)
//
// xsep2k (&a, &q, k, d(a));
//
// Registers:   EAX = &a, EDX = &q, EBX = k, ECX = d(a)
// Assumptions: 0 <= k < 32*d, d(a) > 0, c(q) >= d(a) - [k/32]
//
// Example: d=6, k=104
//                       (k mod 32) = 8
//                        |
// |h h h h|h h h h|h h h l|l l l l|l l l l|l l l l|
// |       |       |       |       |       |       a
// |<----- d - [k/32] ---->|<------ [k/32] ------->|
//                        
//                 |0 0 0 l|l l l l|l l l l|l l l l|
//                 |       |       |       |       a
//
//                         |0 h h h|h h h h|h h h h|
//                         |       |       |       q
void xsep2k(long* a, long* q, long k, long d)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    Zcx,ecx
		mov    eax,a
		mov    edx,q
		mov    ebx,k
		mov    ecx,d
		
		mov    Zbp,EBP
        mov    Zsi,ESI
        mov    Zdi,EDI

        mov    EDI,EDX            ; EDI = &q
        mov    EBP,ECX            ; EBP = d
        mov    ECX,EBX
        and    ECX,31             ; ECX = k mod 32
        shr    EBX,5              ; EBX = [k/32]
        lea    ESI,[EAX+EBX*4]    ; ESI = &a + 4*[k/32]
        sub    EBP,EBX            ; EBP = d - [k/32]

        mov    EDX,1
        mov    EAX,[ESI]          ; EAX = a_[32/k]
        shl    EDX,CL 
        dec    EDX                ; EDX = 2^(k mod 32) - 1
        and    EDX,EAX            ; EDX = a_[32/k] & 2^(k mod 32) - 1
        mov    [ESI],EDX          ; Put back
        dec    EBP
        jz     Lsep8
        mov    EDX,[ESI+4]        ; EDX = a_[32/k + 1]
        shrd   EAX,EDX,CL         ; Divide by 2^(k mod 32)
        mov    [EDI],EAX          ; Store quotient in q
        dec    EBP
        lea    ESI,[ESI+4]
        lea    EDI,[EDI+4]
        jz     Lsep6
Lsep4:
		mov    EAX,[ESI+0]        ; EAX = next a_i
        mov    EDX,[ESI+4]        ; EDX = a_{i+1}
        shrd   EAX,EDX,CL         ; Divide by 2^(k mod 32)
        mov    dword ptr [ESI],0  ; Zap a_i
        mov    [EDI],EAX          ; Store quotient in q
        dec    EBP
        lea    ESI,[ESI+4]
        lea    EDI,[EDI+4]
        jg     Lsep4
Lsep6:
		mov    EAX,[ESI]          ; EAX = last a_i
        mov    dword ptr [ESI],0  ; Zap a_i
Lsep8:
		shr    EAX,CL             ; Divide by 2^(k mod 32)
        mov    [EDI],EAX          ; Store last quotient in q

        mov    EBP,Zbp
        mov    EDI,Zdi
        mov    ESI,Zsi
        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
	}
}
