#include "fastmath.h"

// a = a * b
//
// xmul (&a, &b, d(a), d(b));
// 
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: d(a), d(b) > 0, c(a) >= d(a) + d(b)
//
//                      EDI ->
//                       |
//   |xxx|xxx|xxx|xxx|a_4|a_3|a_2|a_1|a_0|
//
//   *                   |b_3|b_2|b_1|b_0|
//                                       |
//                                   <- ESI
//                                         
//   |ppp|ppp|ppp|ppp|ppp|                  = a_4 * b
//   |ccc|ppp|ppp|ppp|ppp|ppp|              + a_3 * b
//   |ccc|ccc|ppp|ppp|ppp|ppp|ppp|          + a_2 * b
//   |ccc|ccc|ccc|ppp|ppp|ppp|ppp|ppp|      + a_1 * b
//   |ccc|ccc|ccc|ccc|ppp|ppp|ppp|ppp|ppp|  + a_0 * b
void xmul(long* a, long* b, long da, long db)
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
		
		mov    Zbp,EBP
        mov    Zsi,ESI
        mov    Zdi,EDI

        mov    Zmulda,EBX         ; d(a)
        mov    Zmulb0,EDX         ; &b
        mov    Zmuldb,ECX         ; d(b)

; Compute a_{d(a)-1} * b and move into a
        lea    EDI,[EAX+EBX*4-4]  ; EDI = &a{d(a)-1}
        mov    ESI,EDX            ; ESI = &b
        mov    EBX,[EDI]          ; EBX = a_{d(a)-1}
        xor    EBP,EBP            ; EBP = carry
Lmuld:
		mov    EAX,[ESI]          ; ESI = b_j
        lea    ESI,[ESI+4]
        mul    EBX                ; EDX:EAX = a_{d(a)-1}*b_j
        add    EAX,EBP            ; Add carry
        mov    EBP,EDX            ; Save new carry
        mov    [EDI],EAX          ; Move product into a
        adc    EBP,0
        lea    EDI,[EDI+4]
        dec    ECX
        jg     Lmuld
        mov    [EDI],EBP          ; Final carry
        dec    Zmulda
        jz     Lmulx

; Compute a_i * b and accumulate into a
Lmulh:
		mov    ECX,Zmuldb         ; ECX = d(b)
        xor    EBP,EBP            ; EBP = carry
        lea    EAX,[ECX*4+4]       ; EAX = 4*d(b) + 4
        sub    EDI,EAX            ; EDI = &a_i
        mov    ESI,Zmulb0         ; ESI = &b
        mov    EBX,[EDI]          ; EBX = a_i
        mov    dword ptr [EDI],0  ; New a_i = 0

; Compute a_i * b_j and add to a_{i+j}
Lmulk:
		mov    EAX,[ESI]          ; EAX = b_j
        lea    ESI,[ESI+4]
        mul    EBX                ; EDX:EAX = a_i * b_j
        add    EAX,EBP            ; Add carry
        mov    EBP,EDX            ; Save new carry
        mov    EDX,[EDI]          ; EDX = a_{i+j}
        adc    EBP,0
        add    EAX,EDX            ; EAX = a_i*b_j + a_{i+j}
        adc    EBP,0
        mov    [EDI],EAX          ; New a_{i+j}
        lea    EDI,[EDI+4]
        dec    ECX
        jg     Lmulk
        add    [EDI],EBP          ; Add final carry
        adc    dword ptr [EDI+4],0
        jc     Lmulp

; Next a_i
Lmulq:
		dec    Zmulda
        jg     Lmulh
Lmulx:
		mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
		jmp    Lmulr
Lmulp:
		lea    EAX,[EDI+8]
        call   xcar
        jmp    Lmulq
Lmulr:
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
	}
}
