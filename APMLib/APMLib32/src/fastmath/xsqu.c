#include "fastmath.h"

// a = a^2
//
// xsqu (&a, d(a));
//
// Registers:   EAX = &a, EDX = d(a)
// Assumptions: d(a) > 0, c(a) >= 2*d(a)
void xsqu(long* a, long da)
{
	__asm
	{
		mov    Zdx,edx
		mov    eax,a
		mov    edx,da
		
		mov    Zbx,EBX
        mov    Zcx,ECX
        mov    Zbp,EBP
        mov    Zsi,ESI
        mov    Zdi,EDI

        mov    EBP,EAX            ; EBP = &a
        lea    EDI,[EBP+EDX*8-4]  ; EDI = &a + 8*d(a) - 4
Lsqub:
		mov    dword ptr[EDI],0   ; Zero out high order half
        sub    EDI,4              ;      of product area,
        dec    EDX                ;      leaving EDI correclty 
        jg     Lsqub              ;      pointing to a_{d-1}
Lsquc:
		mov    ECX,[EDI]          ; ECX = a_i
        cmp    ECX,0
        je     Lsquu              ; Don't bother if a_i = 0
        mov    EBX,EDI            ; EBX = &a_i 
        sub    EBX,EBP            ; EBX = &a_i - &a
        mov    dword ptr[EDI],0   ; Zap a_i in a
        mov    EAX,ECX 
        mul    ECX                ; EAX:EDX = a_i^2
        add    [EDI+EBX+0],EAX
        adc    [EDI+EBX+4],EDX
        adc    dword ptr [EDI+EBX+8],0
        jc     Lsque
Lsquf:
		sub    EBX,4              ; EBX = &a_{i-1} - &a
        jb     Lsquz              ; i = 0, all done
Lsqui:
		mov    EAX,[EBP+EBX]      ; EAX = a_j
        mul    ECX                ; EAX:EDX = a_i*a_j
        add    EAX,EAX
        adc    EDX,EDX
        adc    dword ptr [EDI+EBX+8],0
        jc     Lsqum
Lsqun:
		add    [EDI+EBX+0],EAX
        adc    [EDI+EBX+4],EDX
        adc    dword ptr [EDI+EBX+8],0
        jc     Lsqup
Lsquq:
		sub    EBX,4              ; Next a_j
        jnb    Lsqui   
Lsquu:
		sub    EDI,4              ; Next a_i
        cmp    EDI,EBP
        jnb    Lsquc
Lsquz:
		mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
        mov    ECX,Zcx
        mov    EBX,Zbx
		jmp    Lsqur
Lsque:
		lea    EAX,[EDI+EBX+12]
xcar1_:
		add    dword ptr[EAX],1
		lea    EAX,[EAX+4]
		jc     xcar1_
        jmp    Lsquf
Lsqum:
		mov    Zsg,EAX
        lea    EAX,[EDI+EBX+12]
xcar2_:
		add    dword ptr[EAX], 1
		lea    EAX,[EAX+4]
		jc     xcar2_
        mov    EAX,Zsg
        jmp    Lsqun
Lsqup:
		lea    EAX,[EDI+EBX+12]
xcar3_:
		add    dword ptr[EAX], 1
		lea    EAX, [EAX + 4]
		jc     xcar3_
        jmp    Lsquq
Lsqur:
		mov    edx,Zdx
	}
}
