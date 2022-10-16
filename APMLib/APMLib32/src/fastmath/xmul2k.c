#include "fastmath.h"

// a = a * 2^k       (k = 32-bit integer)
//
// xmul2k (&a, k, d(a));
//
// Registers:   EAX = &a, EDX = k, EBX = d(a)
// Assumptions: d(a) > 0, c(a) >= d(a) + [(k+31)/32]
//
// Example: d=2, k=136
//
//        ECX = (k mod 32) = 8
//        |                                               EAX
//        ||<---------EDX = [k/32]-------->|<---EBX = d--->|
//        ||       |       |       |       |       |       |
// |0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|a a a a|a a a a|
//        a|a a a a|a a a 0| 
void xmul2k(long* a, long k, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    Zbx,ebx
		mov    eax,a
		mov    edx,k
		mov    ebx,da
		
		mov    Zcx,ECX
        mov    ECX,EDX
        shr    EDX,5              ; EDX = [k/32]
        and    ECX,31             ; ECX = k mod 32
        jz     Lmul2k8            ; k = 0 (mod 32): use xmul2d_

        mov    Zsi,ESI
        mov    Zdi,EDI
        lea    EAX,[EAX+EBX*4]    ; EAX = &a_d
        mov    ESI,0              ; a_d = 0
Lmul2k2:
		mov    EDI,[EAX-4]        ; EDI = a_i
        shld   ESI,EDI,CL         ; multiply by 2^(k mod 32)
        mov    [EAX+EDX*4],ESI    ; Store shifted a_{i+1}
        dec    EBX
        mov    ESI,EDI            ; Next a_{i+1}
        lea    EAX,[EAX-4]
        jg     Lmul2k2

        shl    ESI,CL             ; ESI = a_0
        mov    [EAX+EDX*4],ESI    ; Store shifted a_0

        mov    EDI,Zdi
        mov    ESI,Zsi
        mov    ECX,Zcx

		cmp    EDX,0
        jle    Lmovz8
Lmovz2:
		mov    dword ptr [EAX],0 
        dec    EDX
        lea    EAX,[EAX+4]
        jg     Lmovz2
Lmovz8:
Lmul2k8:
		mov    ECX,Zcx            ; Restore ECX
        
		cmp    EDX,0
        jle    Lmul2d8            ; d = 0: nothing to do
        mov    Zcx,ECX
        lea    EAX,[EAX+EBX*4]    ; EAX = &a_d
Lmul2d4:
		mov    ECX,[EAX-4]        ; EAX = &a{i+1}, ECX = a_i
        mov    [EAX+4*EDX-4],ECX  ; Multiply by 2^(32*d)
        dec    EBX
        lea    EAX,[EAX-4]
        jg     Lmul2d4

        mov    ECX,Zcx

		cmp    EDX,0
        jle    Lmovz8a
Lmovz2a:
		mov    dword ptr [EAX],0 
        dec    EDX
        lea    EAX,[EAX+4]
        jg     Lmovz2a
Lmovz8a:
Lmul2d8:
		mov    edx,Zdx
		mov    ebx,Zbx
	}
}
