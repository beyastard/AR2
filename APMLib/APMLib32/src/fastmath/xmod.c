#include "fastmath.h"

// a = (a mod b), q = [a / b]
//
// xmod (&a, &b, d(a), d(b));
//
// Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
// Assumptions: b has no leading zero digits
//              d(a) >= d(b) >= 2,
//              c(a) >= d(a) + 1,
//              c(q) >= d(a) - d(b) + 1
//              Quotient [a/b] has d(a) - d(b) + 1 digits
//
// Example: d(a) = 6, d(b) = 4, d(q) = 3
//
// extra zero digit                    EDI = &a_i
//         |                               | 
// |0 0 0 0|0 a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
//     :       :       :       :           |
//     |  u_0  |  u_1  |  u_2  |           |
//                                         |
//                                         |
//         |0 0 b b|b b b b|b b b b|b b b b|
//             :       :       :
//             |  v_1  |  v_2  |  
//
// Initial choice for q_i is min(2^32-1, [(u_0*2^32 + u_1)/v_1])
void xmod(long* a, long* b, long da, long db)
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
        mov    Zdivq0,1               ; Set &q = 1 for xmod_
		
		mov    Zsi,ESI
        mov    Zdi,EDI

; Initialise pointers
        mov    Zdiva0,EAX             ; Save &a
        mov    Zdivb0,EDX             ; Save &b
        mov    Zdivda,EBX             ; Save d(a)
        mov    Zdivdb,ECX             ; Save d(b)
        mov    dword ptr[EAX+EBX*4],0 ; Extra zero for a
        sub    EBX,ECX                ; EBX = i = d(a) - d(b)
        lea    EDI,[EAX+EBX*4]        ; EDI = &a_i
        mov    Zdivii,EBX             ; Save i
        mov    Zdivai,EDI             ; Save &a_i

; Determine number of high order bits in b, and hence v_1 and v_2
        mov    EBP,[EDX+ECX*4-4]    ; High order b
        mov    ESI,[EDX+ECX*4-8]    ; Next b
        mov    EDI,0
        cmp    ECX,2
        jbe    Ldivc
        mov    EDI,[EDX+ECX*4-12]   ; Next b if d(b) > 2
Ldivc:
		mov    ECX,0                ; ECX = b_z
Ldivd:
		test   EBP,80000000h        ; Test high order bit
        jnz    Ldive                ; EBP = v_1
        inc    ECX                  ; b_z++
        shld   EBP,ESI,1            ; Shift out zero bit
        shld   ESI,EDI,1
        shl    EDI,1    
        jmp    Ldivd
Ldive:
		mov    Zdivbz,ECX           ; b_z = number of high order bits
        mov    Zdivv1,EBP           ; v_1 = high order 32 bits of b
        mov    Zdivv2,ESI           ; v_2 = next 32 bits of b
  
; for (i = d(a) - d(b); i >= 0; i--)
;
; Calculation of q_i is based on Algorithm D in 
; 4.3.1 of D. Knuth - "Semi-numerical Algorithms"
Ldivf:
		mov    EDI,Zdivai           ; EDI = &a_i
        mov    ECX,Zdivdb           ; ECX = d(b)
        lea    ESI,[EDI+ECX*4-12]   ; ESI = &a_{i+d(b)-3}
        mov    EDX,[ESI+12]          ; EDX = a_{i+d(b)}
        mov    EAX,[ESI+8]           ; EAX = a_{i+d(b)-1}
        mov    EBX,0
        cmp    ESI,Zdiva0
        jb     Ldivg                ; i+d(b) < 3
        mov    EBX,[ESI]            ; EBX = a_{i+d(b)-3}
Ldivg:
		mov    ECX,Zdivbz           ; ECX = b_z
        mov    EBP,[ESI+4]           ; EBP = a_{i+d(b)-2}
        shld   EDX,EAX,CL           ; EDX = u_0
        shld   EAX,EBP,CL           ; EAX = u_1
        shld   EBP,EBX,CL           ; EBP = u_2

; q_i = 2^32 - 1 if u_0 = v_1
        mov    EBX,Zdivv1           ; EBX = v_1
        mov    Zdivqi,0FFFFFFFFh    ; Set q_i = 2^32-1
        cmp    EDX,EBX              ; u_0 = v_1 ?
        jae    Ldivm                ; q_i = 2^32 - 1

; q_i = [(u_0*2^32 + u_1) / v_1]
        mov    ECX,Zdivv2           ; ECX = v_2
        div    EBX                  ; EAX = q_i
        mov    Zdivqi,EAX           ; Save q_i
        mov    ESI,EDX              ; ESI = u_0*2^32 + u_1 - q_i*v_1
;
; If v_2*q_i > (u_0*2^32 + u_1 - q_i*v_1)*2^32 + u_2 then q_i--
        mul    ECX                  ; EDX:EAX = v_2*q_i
;                                   ; ESI     = u_0*2^32 + u_1 - q_i*v_1
;                                   ; EBX     = v_1
;                                   ; ECX     = v_2
;                                   ; EBP     = u_2
Ldivk:
		cmp    EDX,ESI              ; Compare high order
        jb     Ldivm                ; OK
        ja     Ldivl
        cmp    EAX,EBP              ; Compare low order
        jbe    Ldivm                ; OK
Ldivl:
		dec    Zdivqi               ; q_i--
        add    ESI,EBX              ; u_0*2^32 + u_1 - (q_i - 1)*v_1
        jc     Ldivm                ; OK if carry
        sub    EAX,ECX              ; v_2*(q_i - 1)
        sbb    EDX,0 
        jmp    Ldivk                ; Repeat test

; Subtract q_i*b from a_i
Ldivm:
		mov    ESI,Zdivb0           ; ESI = &b
        mov    EBX,Zdivdb           ; EBX = d(b)
        ;inc    Zdivqi               ; For testing "negative result" code
        mov    ECX,Zdivqi           ; ECX = q_i
        xor    EBP,EBP              ; Zero high order digit
Ldivp:
		mov    EAX,[ESI]            ; EAX = b_j
        mul    ECX                  ; Multiply by q_i
        add    EAX,EBP              ; Add last high order digit
        adc    EDX,0                
        mov    EBP,EDX              ; Save high order digit
        mov    EDX,[EDI]            ; Subtract from a_{i+j}
        lea    ESI,[ESI+4]
        sub    EDX,EAX              ; 
        mov    [EDI],EDX
        adc    EBP,0                ; Add borrow from subtract
        lea    EDI,[EDI+4]
        dec    EBX
        jg     Ldivp
        sub    dword ptr [EDI+0],EBP ; Subtract final digit
        jnc    Ldivt                ; Non-negative result

; Negative result: we must add b to a_i
        mov    EDI,Zdivai           ; EDI = &a_i
        mov    ESI,Zdivb0           ; ESI = &b
        mov    EBX,Zdivdb           ; EBX = d(b)
        clc                         ; clear carry
Ldivr:
		mov    EAX,[ESI]            ; b_j
        mov    EDX,[EDI]            ; a_{i+j}
        lea    ESI,[ESI+4]
        adc    EDX,EAX              ; a_{i+j} + b_j + carry 
        dec    EBX
        mov    [EDI],EDX            ; Move to a
        lea    EDI,[EDI+4]
        jg     Ldivr                ; More digits
        adc    dword ptr [EDI],0    ; Final carry
        dec    Zdivqi

; Store quotient digit for xdiv
Ldivt:
		mov    ESI,Zdivq0           ; ESI = &q_0
        cmp    ESI,1
        je     Ldivv                ; No quotient
        mov    EBX,Zdivii           ; EBX = i
        lea    ESI,[ESI+EBX*4]      ; ESI = &q_i
        mov    EAX,Zdivqi
        mov    [ESI],EAX            ; Store quotient

; Next place (xdiv)
        sub    Zdivai,4             ; &a_{i-1}
        dec    Zdivii               ; i--
        jge    Ldivf                ; More places

        mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
		
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx

		jmp    Ldivz

; Next place (xmod)
Ldivv:
		sub    Zdivai,4             ; &a_{i-1}
        dec    Zdivii               ; i--
        jge    Ldivf                ; More places

        mov    EDI,Zdi
        mov    ESI,Zsi
        mov    EBP,Zbp
Ldivz:        
		mov    edx,Zdx
		mov    ebx,Zbx
		mov    ecx,Zcx
	}
}
