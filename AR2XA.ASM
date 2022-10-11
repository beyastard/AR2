;
; AR2XA.ASM  2.2.0F  TONY FORBES  October 1999 
;
; A collection of high-speed routines for doing arithmetic on integer 
; arrays.
;
; There is no array bound checking of any kind. We always assume that the 
; integer arrays are set up properly and that the size parameters have 
; something sensible in them. We also assume that the result integer 
; array has sufficient space to allow the operation to complete 
; successfully. Multiplication requires high order space in the result 
; that is equal in size to the multiplier operand. Division requires 
; one extra high order digit. 
;
; Except where we clearly state otherwise, we are unconcerned with signs. 
; It is up to the caller to decide how to interpret an integer which has 
; the high order bit set. Also we are not concerned with overflow; 
; so add and subtract operations are treated modulo 2^b for some 
; suitable b.
;
; The basic operations, mov, add and sub come in different flavours, 
; depending on the type of second operand, the relative operand sizes, 
; and how they treat the sign bit. In the following scheme it may be 
; helpful to remember that "f" stands for "fixed length", "s" for 
; "signed" and "k" is the initial letter of a word that sounds like 
; "constant".
;
;     1st       2nd      same          |  
;   operand   operand    size   signed |   mov     add     sub
;   -----------------------------------+------------------------
;    array     array      yes     no   |  xmovf   xaddf   xsubf 
;    array     array      no      no   |  xmov    xadd    xsub  
;    array     array      no      yes  |  xmovs   xadds   xsubs 
;    array    constant    no      no   |  xmovk   xaddk   xsubk 
;    array    constant    no      yes  |  xmovks    -       -   
;
; In the routines that recognise signed integers the second operand, 
; if shorter than the first, is extended with sign bits. 
;
; Routines xadd, xadds xaddk, xsub, xsubs and xsubk return the sign bit 
; of the result in register EAX, bit 31, and the carry flag in EAX, bit 8. 
; Routines addf and subf do not. Observe that routines xaddks and xsubks 
; don't exist; there seems to be no need for them. 
;
; The C++ compiler passes words in the order EAX, EDX, EBX, ECX; then 
; any further parameters are placed on the stack.
;
; The compiler insists that we do not alter any register that is not 
; used for passing parameters. PUSH and POP are expensive in clock ticks 
; (and produce catastrophic results when they get out of step), so we often 
; use MOV and designated words to save and restore the contents of registers.
; Conversly, we think we can do what we like with registers that *are* 
; used for passing parameters.
;
; Let a be an integer array. We denote the capacity of a in digits by 
; c(a) and the number of "active" digits by d(a).
;
_DATA   segment dword public 'DATA'
        public  _AR2XAdata
_AR2XAdata  dd  0
            org _AR2XAdata
Zdx     dd      0      ; Save EDX
Zbx     dd      0      ; Save EBX
Zcx     dd      0      ; Save ECX
Zbp     dd      0      ; Save EBP
Zsi     dd      0      ; Save ESI
Zdi     dd      0      ; Save EDI
Zsg     dd      0      ; Sign indicator
Zmulda  dd      0
Zmulb0  dd      0
Zmuldb  dd      0
Zdiva0  dd      0      ; Save &a
Zdivb0  dd      0      ; Save &b
Zdivda  dd      0      ; Save d(a)
Zdivdb  dd      0      ; Save d(b)
Zdivq0  dd      0      ; Save &q
Zdivbz  dd      0      ; b_z = number of high order zero bits of b
Zdivv1  dd      0      ; v_1 = high order 32 bits of b
Zdivv2  dd      0      ; v_2 = next 32 bits of b
Zdivii  dd      0      ; i
Zdivai  dd      0      ; &a_i
Zdivqi  dd      0      ; q_i
_DATA   ends
;
_TEXT   segment dword public 'CODE'
        assume  CS:_TEXT
        assume  DS:DGROUP
_AR2XA  proc    near
;==================================================
;
; Addition, subtraction and related operations
;
;==================================================
;
; a = b, same size
;
; xmovf (&a, &b, d(b));
;
; Registers:   EAX = &a, EDX = &b, EBX = d(b)
; Assumptions: d(a) = d(b) > 0
;
         public xmovf_
xmovf_:  mov    Zcx,ECX
LmovfB:  mov    ECX,[EDX]         ; ECX = b_i
         lea    EDX,4[EDX]
         mov    [EAX],ECX         ; a_i = b_i
         lea    EAX,4[EAX]
         dec    EBX
         jg     LmovfB
         mov    ECX,Zcx
         retn
;==================================================
;
; a = b
; a = b,  b sign extended
;
; xmov  (&a, &b, d(a), d(b));
; xmovs (&a, &b, d(a), d(b));
;
; Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
; Assumptions: d(a) >= d(b) > 0
;
         public xmov_,xmovs_
xmovs_:  mov    Zsg,0FFFFFFFFh
         jmp    LmovA
xmov_:   mov    Zsg,0
LmovA:   mov    Zbp,EBP
;
         sub    EBX,ECX           ; EBX = d(a) - d(b)
LmovB:   mov    EBP,[EDX]         ; EBP = b_i
         lea    EDX,4[EDX]
         mov    [EAX],EBP         ; Move to a
         dec    ECX
         lea    EAX,4[EAX]
         jg     LmovB             ; More digits
;
         cmp    EBX,0 
         jbe    LmovX             ; d(a) = d(b)
         sar    EBP,31 
         and    EBP,Zsg           ; EBP = b sign bits
LmovD:   mov    dword ptr[EAX],0  ; Extend with zero
         dec    EBX
         lea    EAX,4[EAX]
         jg     LmovD             ; More digits
;
LmovX:   mov    EBP,Zbp
         retn
;==================================================
;
; a = k                    (k = 32-bit integer)
; a = k,  k sign extended  (k = 32-bit integer)
;
; xmovk  (&a, k, d(a));
; xmovks (&a, k, d(a));
;
; Registers:   EAX = &a, EDX = k, EBX = d(a)
; Assumptions: d(a) > 0
;
         public xmovk_,xmovks_
xmovks_: mov    Zsg,0FFFFFFFFh
         jmp    LmovkA
xmovk_:  mov    Zsg,0
LmovkA:  mov    [EAX],EDX         ; Move k to a
         dec    EBX
         jz     LmovkX
         SAR    EDX,31   
         and    EDX,Zsg           ; EDX = s sign bits
LmovkD:  mov    4[EAX],EDX        ; Extend with sign
         lea    EAX,4[EAX]
         dec    EBX
         jg     LmovkD
LmovkX:  retn
;==================================================
;
; a = 0
;
; xmovz  (&a, d(a));
;
; Registers:   EAX = &a, EDX = d(a)
;
         public xmovz_
xmovz_:  cmp    EDX,0
         jle    Lmovz8
Lmovz2:  mov    dword ptr [EAX],0 
         dec    EDX
         lea    EAX,4[EAX]
         jg     Lmovz2
Lmovz8:  retn
;==================================================
;
; a = a + b (mod 2^(32*d(a)))
;
; xaddf (&a, &b, d(b));
;
; Registers:   EAX = &a, EDX = &b, EBX = d(b)
; Assumptions: d(a) = d(b) > 0
;
         public xaddf_
xaddf_:  mov    Zcx,ECX
         mov    Zbp,EBP
;
         CLC                      ; clear carry
LaddfB:  mov    EBP,[EDX]         ; EBP = b_i
         mov    ECX,[EAX]         ; ECX = a_i
         lea    EDX,4[EDX]
         adc    ECX,EBP           ; ECX = a_i + b_i + carry 
         dec    EBX
         mov    [EAX],ECX         ; Move to a
         lea    EAX,4[EAX]
         jg     LaddfB            ; More digits
;
         mov    EBP,Zbp
         mov    ECX,Zcx
         retn
;==================================================
;
; a = a + b (mod 2^(32*d(a)))
; a = a + b (mod 2^(32*d(a))), b sign-extended
;
; f = xadd  (&a, &b, d(a), d(b));
; f = xadds (&a, &b, d(a), d(b));
;
; Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
; Assumptions: d(a) >= d(b) > 0
; Returns:     Result sign bit EAX bit 31
;              Carry flag      EAX bit  8
;
         public xadd_,xadds_
xadds_:  mov    Zsg,0FFFFFFFFh 
         jmp    LaddA
xadd_:   mov    Zsg,0
LaddA:   mov    Zbp,EBP
         mov    Zdi,EDI
;
         mov    EDI,EAX           ; EDI = &a
         sub    EBX,ECX           ; EBX = d(a) - d(b)
         CLC                      ; clear carry
LaddB:   mov    EBP,[EDX]         ; EBP = b_i
         mov    EAX,[EDI]         ; EAX = a_i
         lea    EDX,4[EDX]
         adc    EAX,EBP           ; EAX = a_i + b_i + carry 
         dec    ECX
         mov    [EDI],EAX         ; Move to a
         lea    EDI,4[EDI]
         jg     LaddB             ; More digits
         lahf                     ; Carry flag to AH
;
         cmp    EBX,0 
         jbe    LaddX             ; d(a) = d(b)
         sar    EBP,31 
         and    EBP,Zsg           ; EBP = b sign bits
         sahf                     ; Restore carry
LaddD:   mov    EAX,[EDI]         ; EAX = a_i
         adc    EAX,EBP           ; EAX = a_i + b sign bits + carry 
         dec    EBX
         mov    [EDI],EAX         ; Move to a
         lea    EDI,4[EDI]
         jg     LaddD             ; More sign bits
         lahf                     ; Carry flag to AH
;
LaddX:   mov    EDI,Zdi
         mov    EBP,Zbp
         retn
;==================================================
;
; a = a + k (mod 2^(32*d)), k = 32-bit integer
;
; xaddk (&a, k, d(a));
;
; Registers:   EAX = &a, EDX = k, EBX = d = d(a)
; Assumptions: k >= 0, d(a) > 0
; Returns:     Result sign bit EAX bit 31
;              Carry flag      EAX bit  8
; 
         public xaddk_
xaddk_:  mov    Zdi,EDI
;
         mov    EDI,EAX           ; EDI = &a
         mov    EAX,[EDI]
         add    EAX,EDX  
         mov    [EDI],EAX         ; a_0 = a_0 + k
         jnc    LaddkX
         dec    EBX
         jz     LaddkX            ; Only one digit
LaddkK:  mov    EAX,4[EDI]        ; a_i
         add    EAX,1             
         mov    4[EDI],EAX        ; Move to a
         jnc    LaddkX
         dec    EBX
         lea    EDI,4[EDI]
         jg     LaddkK            ; More
;
LaddkX:  lahf    
         mov    EDI,Zdi
         retn
;==================================================
;
; a = a - b (mod 2^(32*d(a)))
;
; xsubf (&a, &b, d(b));
;
; Registers:   EAX = &a, EDX = &b, EBX = d(b)
; Assumptions: d(a) = d(b) > 0
;
         public xsubf_
xsubf_:  mov    Zcx,ECX
         mov    Zbp,EBP
;
         CLC                      ; clear carry
LsubfB:  mov    EBP,[EDX]         ; EBP = b_i
         mov    ECX,[EAX]         ; ECX = a_i
         lea    EDX,4[EDX]
         sbb    ECX,EBP           ; ECX = a_i - b_i - borrow
         dec    EBX
         mov    [EAX],ECX         ; Move to a
         lea    EAX,4[EAX]
         jg     LsubfB            ; More digits
;
         mov    EBP,Zbp
         mov    ECX,Zcx
         retn
;==================================================
;
; a = a - b (mod 2^(32*d(a)))
; a = a - b (mod 2^(32*d(a))), b sign-extended
;
; f = xsub  (&a, &b, d(a), d(b));
; f = xsubs (&a, &b, d(a), d(b));
;
; Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
; Assumptions: d(a) >= d(b) > 0
; Returns:     Result sign bit EAX bit 31
;              Carry flag      EAX bit  8
;
         public xsub_,xsubs_
xsubs_:  mov    Zsg,0FFFFFFFFh 
         jmp    LsubA
xsub_:   mov    Zsg,0
LsubA:   mov    Zbp,EBP
         mov    Zdi,EDI
;
         mov    EDI,EAX           ; EDI = &a
         sub    EBX,ECX           ; EBX = d(a) - d(b)
         CLC                      ; clear carry
LsubB:   mov    EBP,[EDX]         ; EBP = b_i
         mov    EAX,[EDI]         ; EAX = a_i
         lea    EDX,4[EDX]
         sbb    EAX,EBP           ; EAX = a_i - b_i - carry
         dec    ECX
         mov    [EDI],EAX         ; Move to a
         lea    EDI,4[EDI]
         jg     LsubB             ; More digits
         lahf                     ; Carry flag to AH
;
         cmp    EBX,0 
         jbe    LsubX             ; d(a) = d(b)
         sar    EBP,31 
         and    EBP,Zsg           ; EBP = b sign bits
         sahf                     ; Restore carry flag
LsubD:   mov    EAX,[EDI]         ; EAX = a_i
         sbb    EAX,EBP           ; EAX = a_i - b sign bits - carry 
         dec    EBX
         mov    [EDI],EAX         ; Move to a
         lea    EDI,4[EDI]
         jg     LsubD             ; More sign bits
         lahf                     ; Carry flag to AH
;
LsubX:   mov    EDI,Zdi
         mov    EBP,Zbp
         retn
;==================================================
;
; a = a - k (mod 2^(32*d)), k = 32-bit integer
;
; xsubk (&a, k, d(a));
;
; Registers:   EAX = &a, EDX = k, EBX = d(a)
; Assumptions: k >= 0, d(a) > 0
; Returns:     Result sign bit EAX bit 31
;              Carry flag      EAX bit  8
; 
         public xsubk_
xsubk_:  mov    Zdi,EDI
;
         mov    EDI,EAX           ; EDI = &a
         mov    EAX,[EDI]
         sub    EAX,EDX  
         mov    [EDI],EAX         ; a_0 = a_0 - k
         jnc    LsubkX
         dec    EBX
         jz     LsubkX            ; Only one digit
LsubkK:  mov    EAX,4[EDI]        ; a_i
         sub    EAX,1             
         mov    4[EDI],EAX        ; Move to a
         jnc    LsubkX
         dec    EBX
         lea    EDI,4[EDI]
         jg     LsubkK            ; More
;
LsubkX:  lahf    
         mov    EDI,Zdi
         retn
;==================================================
;
; a = 2^(32*d(a)) - a
;
; xneg (&a, d(a));
;
; Registers:   EAX = &a, EDX = d(a)
; Assumptions: d(a) > 0
;
         public xneg_
xneg_:   mov    Zbx,EBX
         mov    EBX,[EAX]         ; EBX = a_0
         not    EBX
         add    EBX,1             ; EBX = -a_0
         dec    EDX
         mov    [EAX],EBX
         jz     LnegX
LnegB:   mov    EBX,4[EAX]        ; EBX = a_i
         not    EBX               ; Flip a_i bits
         adc    EBX,0             ; Add carry
         dec    EDX
         mov    4[EAX],EBX
         lea    EAX,4[EAX]
         jg     LnegB
LnegX:   mov    EBX,Zbx
         retn
;==================================================
;
; flags(a - b)
; flags(a - b), shorter of a and b is sign-extended
;
; flags = xcmp  (&a, &b, d(a), d(b));
; flags = xcmps (&a, &b, d(a), d(b));
;
; Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
; Assumptions: d(a), d(b) > 0
; Returns:     EAX = SignFlag    0x00008000
;                    ZeroFlag    0x00004000
;                    AuxFlag     0x00001000
;                    ParityFlag  0x00000200
;                    CarryFlag   0x00000100
;
         public xcmp_,xcmps_
xcmps_:  mov    Zsg,0FFFFFFFFh 
         jmp    LcmpA
xcmp_:   mov    Zsg,0
;
LcmpA:   mov    Zsi,ESI
         mov    Zdi,EDI
         lea    ESI,-4[EAX+EBX*4] ; ESI = &a_{d(a)-1}
         lea    EDI,-4[EDX+ECX*4] ; EDI = &b_{d(b)-1}
         sub    ECX,EBX
         je     LcmpK             ; same size
         jb     LcmpF             ; b shorter than a
;
; a is shorter than b, EBX = d(a), ECX = d(b) - d(a)
         mov    EAX,[ESI]
         sar    EAX,31
         and    EAX,Zsg           ; EAX = a sign
LcmpD:   mov    EDX,[EDI]         ; Pick up b_i
         cmp    EAX,EDX           ; Compare a sign with b_i
         lea    EDI,-4[EDI]
         jne    LcmpX
         dec    ECX
         jnz    LcmpD
         jmp    LcmpK
;
; b is shorter than a, EBX = d(a), ECX = -(d(a) - d(b))
LcmpF:   add    EBX,ECX           ; EBX = d(a)
         mov    EDX,[EDI]
         sar    EDX,31
         and    EDX,Zsg           ; EDX = b sign
LcmpH:   mov    EAX,[ESI]         ; Pick up a_i
         cmp    EAX,EDX           ; Compare a_i with b sign
         lea    ESI,-4[ESI]
         jne    LcmpX
         inc    ECX
         jnz    LcmpH
;
LcmpK:   mov    EAX,[ESI]         ; same size
         mov    EDX,[EDI]
         lea    ESI,-4[ESI]
         cmp    EAX,EDX           ; Compare a_i with b_i
         lea    EDI,-4[EDI]
         jne    LcmpX
         dec    EBX
         jg     LcmpK
         cmp    EAX,EDX           ; Equal
;
LcmpX:   lahf   
         mov    EDI,Zdi
         mov    ESI,Zsi
         retn
;==================================================
;
; high-order digit of a
;
; hdigit = xsig (&a, d(a));
;
; Registers:   EAX = &a, EDX = d(a)
; Assumptions: d(a) > 0
; Returns:     EAX = high order digit of a 
;              (a = 0 iff EAX = 0)
;
         public xsig_
xsig_:   mov    Zbx,EBX
         lea    EBX,-4[EAX+EDX*4] ; EBX = &a_{d-1}
Lsig2:   mov    EAX,[EBX]
         lea    EBX,-4[EBX]
         cmp    EAX,0
         jne    Lsig8
         dec    EDX
         jg     Lsig2
Lsig8:   mov    EBX,Zbx
         retn
;==================================================
;
; Number of significant digits of a
;
; n = xdig (d(a), &a);
;
; Registers:   EAX = d(a), EDX = &a
; Assumptions: d(a) > 0
; Returns:     EAX = number of significant digits of a 
;
         public xdig_
xdig_:   cmp    EAX,1
         jbe    Ldig8
         cmp    dword ptr -4[EDX+EAX*4],0
         jne    Ldig8
         dec    EAX
         jmp    xdig_
Ldig8:   retn
;==================================================
;
; Add one to [EAX] and propogate ad infinitum
;
xcar:    add    dword ptr[EAX],1
         lea    EAX,4[EAX]
         jc     xcar
         retn
;==================================================
;
; Subtract one from [EAX] and propogate ad infinitum
;
xbor:    sub    dword ptr[EAX],1
         lea    EAX,4[EAX]
         jc     xbor
         retn
;==================================================
;
; Multiplication, division and related operations
;
;==================================================
; 
; a = a^2
;
; xsqu (&a, d(a));
;
; Registers:   EAX = &a, EDX = d(a)
; Assumptions: d(a) > 0, c(a) >= 2*d(a)
;
         public xsqu_
xsqu_:   mov    Zbx,EBX
         mov    Zcx,ECX
         mov    Zbp,EBP
         mov    Zsi,ESI
         mov    Zdi,EDI
;
         mov    EBP,EAX            ; EBP = &a
         lea    EDI,-4[EBP+EDX*8]  ; EDI = &a + 8*d(a) - 4
;
Lsqub:   mov    dword ptr[EDI],0   ; Zero out high order half
         sub    EDI,4              ;      of product area,
         dec    EDX                ;      leaving EDI correclty 
         jg     Lsqub              ;      pointing to a_{d-1}
; 
Lsquc:   mov    ECX,[EDI]          ; ECX = a_i
         cmp    ECX,0
         je     Lsquu              ; Don't bother if a_i = 0
         mov    EBX,EDI            ; EBX = &a_i 
         sub    EBX,EBP            ; EBX = &a_i - &a
         mov    dword ptr[EDI],0   ; Zap a_i in a
         mov    EAX,ECX 
         mul    ECX                ; EAX:EDX = a_i^2
         add    0[EDI+EBX],EAX
         adc    4[EDI+EBX],EDX
         adc    dword ptr 8[EDI+EBX],0
         jc     Lsque
Lsquf:   sub    EBX,4              ; EBX = &a_{i-1} - &a
         jb     Lsquz              ; i = 0, all done
Lsqui:   mov    EAX,[EBP+EBX]      ; EAX = a_j
         mul    ECX                ; EAX:EDX = a_i*a_j
         add    EAX,EAX
         adc    EDX,EDX
         adc    dword ptr 8[EDI+EBX],0
         jc     Lsqum
Lsqun:   add    0[EDI+EBX],EAX
         adc    4[EDI+EBX],EDX
         adc    dword ptr 8[EDI+EBX],0
         jc     Lsqup
Lsquq:   sub    EBX,4              ; Next a_j
         jnb    Lsqui   
Lsquu:   sub    EDI,4              ; Next a_i
         cmp    EDI,EBP
         jnb    Lsquc
;
Lsquz:   mov    EDI,Zdi
         mov    ESI,Zsi
         mov    EBP,Zbp
         mov    ECX,Zcx
         mov    EBX,Zbx
         retn
;
Lsque:   lea    EAX,12[EDI+EBX]
         call   xcar
         jmp    Lsquf
Lsqum:   mov    Zsg,EAX
         lea    EAX,12[EDI+EBX]
         call   xcar
         mov    EAX,Zsg
         jmp    Lsqun
Lsqup:   lea    EAX,12[EDI+EBX]
         call   xcar
         jmp    Lsquq
;==================================================
; 
; a = a * b
;
; xmul (&a, &b, d(a), d(b));
; 
; Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
; Assumptions: d(a), d(b) > 0, c(a) >= d(a) + d(b)
;
;                      EDI ->
;                       |
;   |xxx|xxx|xxx|xxx|a_4|a_3|a_2|a_1|a_0|
;
;   *                   |b_3|b_2|b_1|b_0|
;                                       |
;                                   <- ESI
;                                         
;   |ppp|ppp|ppp|ppp|ppp|                  = a_4 * b
;   |ccc|ppp|ppp|ppp|ppp|ppp|              + a_3 * b
;   |ccc|ccc|ppp|ppp|ppp|ppp|ppp|          + a_2 * b
;   |ccc|ccc|ccc|ppp|ppp|ppp|ppp|ppp|      + a_1 * b
;   |ccc|ccc|ccc|ccc|ppp|ppp|ppp|ppp|ppp|  + a_0 * b
; 
         public xmul_
xmul_:   mov    Zbp,EBP
         mov    Zsi,ESI
         mov    Zdi,EDI
;
         mov    Zmulda,EBX         ; d(a)
         mov    Zmulb0,EDX         ; &b
         mov    Zmuldb,ECX         ; d(b)
;
; Compute a_{d(a)-1} * b and move into a
         lea    EDI,-4[EAX+EBX*4]  ; EDI = &a{d(a)-1}
         mov    ESI,EDX            ; ESI = &b
         mov    EBX,[EDI]          ; EBX = a_{d(a)-1}
         xor    EBP,EBP            ; EBP = carry
Lmuld:   mov    EAX,[ESI]          ; ESI = b_j
         lea    ESI,4[ESI]
         mul    EBX                ; EDX:EAX = a_{d(a)-1}*b_j
         add    EAX,EBP            ; Add carry
         mov    EBP,EDX            ; Save new carry
         mov    [EDI],EAX          ; Move product into a
         adc    EBP,0
         lea    EDI,4[EDI]
         dec    ECX
         jg     Lmuld
         mov    [EDI],EBP          ; Final carry
         dec    Zmulda
         jz     Lmulx
;
; Compute a_i * b and accumulate into a
Lmulh:   mov    ECX,Zmuldb         ; ECX = d(b)
         xor    EBP,EBP            ; EBP = carry
         lea    EAX,4[ECX*4]       ; EAX = 4*d(b) + 4
         sub    EDI,EAX            ; EDI = &a_i
         mov    ESI,Zmulb0         ; ESI = &b
         mov    EBX,[EDI]          ; EBX = a_i
         mov    dword ptr [EDI],0  ; New a_i = 0
;
; Compute a_i * b_j and add to a_{i+j}
Lmulk:   mov    EAX,[ESI]          ; EAX = b_j
         lea    ESI,4[ESI]
         mul    EBX                ; EDX:EAX = a_i * b_j
         add    EAX,EBP            ; Add carry
         mov    EBP,EDX            ; Save new carry
         mov    EDX,[EDI]          ; EDX = a_{i+j}
         adc    EBP,0
         add    EAX,EDX            ; EAX = a_i*b_j + a_{i+j}
         adc    EBP,0
         mov    [EDI],EAX          ; New a_{i+j}
         lea    EDI,4[EDI]
         dec    ECX
         jg     Lmulk
         add    [EDI],EBP          ; Add final carry
         adc    dword ptr 4[EDI],0
         jc     Lmulp
;
; Next a_i
Lmulq:   dec    Zmulda
         jg     Lmulh
;
Lmulx:   mov    EDI,Zdi
         mov    ESI,Zsi
         mov    EBP,Zbp
         retn
;
Lmulp:   lea    EAX,8[EDI]
         call   xcar
         jmp    Lmulq
;==================================================
; 
; a = a * k,  (k = 32-bit integers)
;
; xmulk (&a, k, d(a));
;
; Registers:   EAX = &a, EDX = k, EBX = d(a)
; Assumptions: d(a) > 0, c(a) > d(a)
;
         public xmulk_
xmulk_:  mov    Zbp,EBP
         mov    Zsi,ESI
;
         mov    ESI,EAX            ; ESI = &a
         mov    ECX,EDX            ; ECX = k
         xor    EBP,EBP
Lmulk2:  mov    EAX,[ESI]          ; EAX = a_i
         mul    ECX                ; Multiply by k
         add    EAX,EBP            ; Add carry
         adc    EDX,0  
         mov    EBP,EDX            ; Save carry
         mov    [ESI],EAX
         lea    ESI,4[ESI]
         dec    EBX
         jg     Lmulk2
         mov    [ESI],EDX          ; Final high order digit
;
         mov    ESI,Zsi
         mov    EBP,Zbp
         retn
;==================================================
; 
; a = a * 2^k       (k = 32-bit integer)
;
; xmul2k (&a, k, d(a));
;
; Registers:   EAX = &a, EDX = k, EBX = d(a)
; Assumptions: d(a) > 0, c(a) >= d(a) + [(k+31)/32]
;
; Example: d=2, k=136
;
;        ECX = (k mod 32) = 8
;        |                                               EAX
;        ||<---------EDX = [k/32]-------->|<---EBX = d--->|
;        ||       |       |       |       |       |       |
; |0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|a a a a|a a a a|
;        a|a a a a|a a a 0| 
;
         public xmul2k_
xmul2k_: mov    Zcx,ECX
         mov    ECX,EDX
         shr    EDX,5              ; EDX = [k/32]
         and    ECX,31             ; ECX = k mod 32
         jz     Lmul2k8            ; k = 0 (mod 32): use xmul2d_
;
         mov    Zsi,ESI
         mov    Zdi,EDI
         lea    EAX,[EAX+EBX*4]    ; EAX = &a_d
         mov    ESI,0              ; a_d = 0
;
Lmul2k2: mov    EDI,-4[EAX]         ; EDI = a_i
         shld   ESI,EDI,CL         ; multiply by 2^(k mod 32)
         mov    [EAX+EDX*4],ESI    ; Store shifted a_{i+1}
         dec    EBX
         mov    ESI,EDI            ; Next a_{i+1}
         lea    EAX,-4[EAX]
         jg     Lmul2k2
;
         shl    ESI,CL             ; ESI = a_0
         mov    [EAX+EDX*4],ESI    ; Store shifted a_0
;
         mov    EDI,Zdi
         mov    ESI,Zsi
         mov    ECX,Zcx
         jmp    xmovz_             ; ZAP EDX words from [EAX]
;
Lmul2k8: mov    ECX,Zcx            ; Restore ECX
;        jmp    xmul2k_            ; Drop into xmul2k_
;==================================================
;
; a = a * 2^(32*d)  (d = 32-bit integer)
;
; xmul2d (&a, d, d(a));
;
; Registers:   EAX = &a, EDX = d, EBX = d(a)
; Assumptions: d(a) > 0, c(a) >= d(a) + d
;
         public xmul2d_
xmul2d_: cmp    EDX,0
         jle    Lmul2d8            ; d = 0: nothing to do
         mov    Zcx,ECX
         lea    EAX,[EAX+EBX*4]    ; EAX = &a_d
;
Lmul2d4: mov    ECX,-4[EAX]        ; EAX = &a{i+1}, ECX = a_i
         mov    -4[EAX+4*EDX],ECX  ; Multiply by 2^(32*d)
         dec    EBX
         lea    EAX,-4[EAX]
         jg     Lmul2d4
;
         mov    ECX,Zcx
         jmp    xmovz_             ; ZAP EDX words from [EAX]
;
Lmul2d8: retn
;==================================================
;
; a = (a mod b), q = [a / b]
;
; xdiv (&a, &b, d(a), d(b), &q);
; xmod (&a, &b, d(a), d(b));
;
; Registers:   EAX = &a, EDX = &b, EBX = d(a), ECX = d(b)
;              4[ESP] = &q (xdiv only)
; Assumptions: b has no leading zero digits
;              d(a) >= d(b) >= 2,
;              c(a) >= d(a) + 1,
;              c(q) >= d(a) - d(b) + 1
;              Quotient [a/b] has d(a) - d(b) + 1 digits
;
; Example: d(a) = 6, d(b) = 4, d(q) = 3
;
; extra zero digit                    EDI = &a_i
;         |                               | 
; |0 0 0 0|0 a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
;     :       :       :       :           |
;     |  u_0  |  u_1  |  u_2  |           |
;                                         |
;                                         |
;         |0 0 b b|b b b b|b b b b|b b b b|
;             :       :       :
;             |  v_1  |  v_2  |  
;
; Initial choice for q_i is min(2^32-1, [(u_0*2^32 + u_1)/v_1])
;
         public xdiv_,xmod_
xmod_:   mov    Zbp,EBP
         mov    Zdivq0,1               ; Set &q = 1 for xmod_
         jmp    Ldivb
xdiv_:   mov    Zbp,EBP
         mov    EBP,4[ESP]
         mov    Zdivq0,EBP             ; Save &q for xdiv_
Ldivb:   mov    Zsi,ESI
         mov    Zdi,EDI
;
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
;
; Determine number of high order bits in b, and hence v_1 and v_2
         mov    EBP,-4[EDX+ECX*4]    ; High order b
         mov    ESI,-8[EDX+ECX*4]    ; Next b
         mov    EDI,0
         cmp    ECX,2
         jbe    Ldivc
         mov    EDI,-12[EDX+ECX*4]   ; Next b if d(b) > 2
Ldivc:   mov    ECX,0                ; ECX = b_z
Ldivd:   test   EBP,80000000h        ; Test high order bit
         jnz    Ldive                ; EBP = v_1
         inc    ECX                  ; b_z++
         shld   EBP,ESI,1            ; Shift out zero bit
         shld   ESI,EDI,1
         shl    EDI,1    
         jmp    Ldivd
Ldive:   mov    Zdivbz,ECX           ; b_z = number of high order bits
         mov    Zdivv1,EBP           ; v_1 = high order 32 bits of b
         mov    Zdivv2,ESI           ; v_2 = next 32 bits of b
;          
; for (i = d(a) - d(b); i >= 0; i--)
;
; Calculation of q_i is based on Algorithm D in 
; 4.3.1 of D. Knuth's "Semi-numerical Algorithms"
Ldivf:   mov    EDI,Zdivai           ; EDI = &a_i
         mov    ECX,Zdivdb           ; ECX = d(b)
         lea    ESI,-12[EDI+ECX*4]   ; ESI = &a_{i+d(b)-3}
         mov    EDX,12[ESI]          ; EDX = a_{i+d(b)}
         mov    EAX,8[ESI]           ; EAX = a_{i+d(b)-1}
         mov    EBX,0
         cmp    ESI,Zdiva0
         jb     Ldivg                ; i+d(b) < 3
         mov    EBX,[ESI]            ; EBX = a_{i+d(b)-3}
Ldivg:   mov    ECX,Zdivbz           ; ECX = b_z
         mov    EBP,4[ESI]           ; EBP = a_{i+d(b)-2}
         shld   EDX,EAX,CL           ; EDX = u_0
         shld   EAX,EBP,CL           ; EAX = u_1
         shld   EBP,EBX,CL           ; EBP = u_2
;
; q_i = 2^32 - 1 if u_0 = v_1
         mov    EBX,Zdivv1           ; EBX = v_1
         mov    Zdivqi,0FFFFFFFFh    ; Set q_i = 2^32-1
         cmp    EDX,EBX              ; u_0 = v_1 ?
         jae    Ldivm                ; q_i = 2^32 - 1
;
; q_i = [(u_0*2^32 + u_1) / v_1]
         mov    ECX,Zdivv2           ; ECX = v_2
         div    EBX                  ; EAX = q_i
         mov    Zdivqi,EAX           ; Save q_i
         mov    ESI,EDX              ; ESI = u_0*2^32 + u_1 - q_i*v_1
;
; If v_2*q_i > (u_0*2^32 + u_1 - q_i*v_1)*2^32 + u_2 then q_i--
         mul    ECX                  ; EDX:EAX = v_2*q_i
;                                    ; ESI     = u_0*2^32 + u_1 - q_i*v_1
;                                    ; EBX     = v_1
;                                    ; ECX     = v_2
;                                    ; EBP     = u_2
Ldivk:   cmp    EDX,ESI              ; Compare high order
         jb     Ldivm                ; OK
         ja     Ldivl
         cmp    EAX,EBP              ; Compare low order
         jbe    Ldivm                ; OK
Ldivl:   dec    Zdivqi               ; q_i--
         add    ESI,EBX              ; u_0*2^32 + u_1 - (q_i - 1)*v_1
         jc     Ldivm                ; OK if carry
         sub    EAX,ECX              ; v_2*(q_i - 1)
         sbb    EDX,0 
         jmp    Ldivk                ; Repeat test
;
; Subtract q_i*b from a_i
Ldivm:   mov    ESI,Zdivb0           ; ESI = &b
         mov    EBX,Zdivdb           ; EBX = d(b)
;        inc    Zdivqi               ; For testing "negative result" code
         mov    ECX,Zdivqi           ; ECX = q_i
         xor    EBP,EBP              ; Zero high order digit
Ldivp:   mov    EAX,[ESI]            ; EAX = b_j
         mul    ECX                  ; Multiply by q_i
         add    EAX,EBP              ; Add last high order digit
         adc    EDX,0                
         mov    EBP,EDX              ; Save high order digit
         mov    EDX,[EDI]            ; Subtract from a_{i+j}
         lea    ESI,4[ESI]
         sub    EDX,EAX              ; 
         mov    [EDI],EDX
         adc    EBP,0                ; Add borrow from subtract
         lea    EDI,4[EDI]
         dec    EBX
         jg     Ldivp
         sub    dword ptr 0[EDI],EBP ; Subtract final digit
         jnc    Ldivt                ; Non-negative result
;
; Negative result: we must add b to a_i
         mov    EDI,Zdivai           ; EDI = &a_i
         mov    ESI,Zdivb0           ; ESI = &b
         mov    EBX,Zdivdb           ; EBX = d(b)
         CLC                         ; clear carry
Ldivr:   mov    EAX,[ESI]            ; b_j
         mov    EDX,[EDI]            ; a_{i+j}
         lea    ESI,4[ESI]
         adc    EDX,EAX              ; a_{i+j} + b_j + carry 
         dec    EBX
         mov    [EDI],EDX            ; Move to a
         lea    EDI,4[EDI]
         jg     Ldivr                ; More digits
         adc    dword ptr [EDI],0    ; Final carry
         dec    Zdivqi
;
; Store quotient digit for xdiv
Ldivt:   mov    ESI,Zdivq0           ; ESI = &q_0
         cmp    ESI,1
         je     Ldivv                ; No quotient
         mov    EBX,Zdivii           ; EBX = i
         lea    ESI,[ESI+EBX*4]      ; ESI = &q_i
         mov    EAX,Zdivqi
         mov    [ESI],EAX            ; Store quotient
;
; Next place (xdiv)
         sub    Zdivai,4             ; &a_{i-1}
         dec    Zdivii               ; i--
         jge    Ldivf                ; More places
;
         mov    EDI,Zdi
         mov    ESI,Zsi
         mov    EBP,Zbp
         retn   4                    ; One item on stack
;
; Next place (xmod)
Ldivv:   sub    Zdivai,4             ; &a_{i-1}
         dec    Zdivii               ; i--
         jge    Ldivf                ; More places
;
         mov    EDI,Zdi
         mov    ESI,Zsi
         mov    EBP,Zbp
         retn                        ; Nothing on stack
;==================================================
; 
; a = [a / k]  (k = 32-bit integer)
;
; r = xdivk (&a, k, d(a));
;
; Registers:   EAX = &a, EDX = k, EBX = d(a)
; Assumptions: k > 0, d(a) > 0
; Returns:     (a mod k) in EAX
; 
         public xdivk_
xdivk_:  mov    Zcx,ECX
         mov    Zdi,EDI
;
         lea    EDI,-4[EAX+EBX*4]  ; EDI = &a_{d-1}
         mov    ECX,EDX            ; ECX = k
         xor    EDX,EDX            ; EDX = 0
Ldivk2:  mov    EAX,[EDI]          ; EAX = a_i
         div    ECX                ; EAX = [(2^32*EDX + a_i)/k]
         mov    [EDI],EAX          ; EDX = (2^32*EDX + a_i) mod k]
         sub    EDI,4
         dec    EBX
         jg     Ldivk2
         mov    EAX,EDX            ; Remainder
;
         mov    ECX,Zcx
         mov    EDI,Zdi
         retn
;==================================================
; 
; a = [a/2^k]  (k = 32-bit integer)
;
; xdiv2k (&a, k, d(a));
;
; Registers:   EAX = &a, EDX = k, EBX = d(a)
; Assumptions: 0 <= k < 32*d(a), d(a) > 0
;
; Example: d=6, k=104  
;                       ECX = (k mod 32) = 8
;                        |
; |a a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
; |       |       |       |       |       |       |
; |<---EBX = d - [k/32]-->|<-----EDX = [k/32]---->|
;                                                EAX
;
         public xdiv2k_
xdiv2k_: mov    Zcx,ECX
         mov    Zsi,ESI
         mov    ECX,EDX
         mov    Zdi,EDI
         and    ECX,31             ; ECX = k mod 32
         shr    EDX,5              ; EDX = [k/32]
         sub    EBX,EDX            ; EBX = d - [k/32]
         dec    EBX
         jz     Ldiv2k6            ; Only 1 digit
Ldiv2k4: mov    ESI,0[EAX+EDX*4]   ; ESI = a_i
         mov    EDI,4[EAX+EDX*4]   ; EDI = a_{i+1}
         shrd   ESI,EDI,CL         ; Divide by 2^(k mod 32)
         mov    [EAX],ESI          ; Store quotient in a
         dec    EBX
         lea    EAX,4[EAX]
         jg     Ldiv2k4
;
Ldiv2k6: mov    ESI,0[EAX+EDX*4]   ; ESI = last a_i
         mov    EDI,Zdi
         shr    ESI,CL             ; Divide by 2^(k mod 32)
         mov    ECX,Zcx
         mov    [EAX],ESI          ; Store last quotient in a
         mov    ESI,Zsi
         retn
;==================================================
;
; a = a * 2^(32*d)  (d = 32-bit integer)
;
; xdiv2d (&a, d, d(a));
;
; Registers:   EAX = &a, EDX = d, EBX = d(a)
; Assumptions: 0 <= d < d(a), d(a) > 0
;
         public xdiv2d_
xdiv2d_: cmp    EDX,0
         jle    Ldiv2d8            ; d = 0: nothing to do
         mov    Zcx,ECX
         sub    EBX,EDX            ; EBX = d(a) - d
;
Ldiv2d4: mov    ECX,[EAX+EDX*4]    ; EAX = &a, ECX = a_i
         mov    [EAX],ECX          ; Divide by 2^(32*d)
         dec    EBX
         lea    EAX,4[EAX]
         jg     Ldiv2d4
;
         mov    ECX,Zcx
Ldiv2d8: retn
;==================================================
; 
; a = (a mod k)  (k = 32-bit integer)
;
; xmod (&a, k, d(a));
;
; Registers:   EAX = &a, EDX = k, EBX = d(a)
; Assumptions: k > 0, d(a) > 0
; 
         public xmodk_
xmodk_:  mov    Zcx,ECX
         mov    Zdi,EDI
;
         lea    EDI,-4[EAX+EBX*4]  ; EDI = &a_{d-1}
         mov    ECX,EDX            ; ECX = k
         xor    EDX,EDX            ; EDX = 0
Lmodk2:  mov    EAX,[EDI]          ; EAX = a_i
         div    ECX                ; EAX = [(2^32*EDX + a_i)/k]
         sub    EDI,4
         dec    EBX
         jg     Lmodk2
         mov    4[EDI],EDX         ; Remainder
;
         mov    ECX,Zcx
         mov    EDI,Zdi
         retn
;==================================================
; 
; a = a mod 2^k,  q = [a/2^k]  (k = 32-bit integer)
;
; xsep2k (&a, &q, k, d(a));
;
; Registers:   EAX = &a, EDX = &q, EBX = k, ECX = d(a)
; Assumptions: 0 <= k < 32*d, d(a) > 0, c(q) >= d(a) - [k/32]
;
; Example: d=6, k=104
;                       (k mod 32) = 8
;                        |
; |h h h h|h h h h|h h h l|l l l l|l l l l|l l l l|
; |       |       |       |       |       |       a
; |<----- d - [k/32] ---->|<------ [k/32] ------->|
;                        
;                 |0 0 0 l|l l l l|l l l l|l l l l|
;                 |       |       |       |       a
;
;                         |0 h h h|h h h h|h h h h|
;                         |       |       |       q
;
         public xsep2k_
xsep2k_: 
         mov    Zbp,EBP
         mov    Zsi,ESI
         mov    Zdi,EDI
;
         mov    EDI,EDX            ; EDI = &q
         mov    EBP,ECX            ; EBP = d
         mov    ECX,EBX
         and    ECX,31             ; ECX = k mod 32
         shr    EBX,5              ; EBX = [k/32]
         lea    ESI,[EAX+EBX*4]    ; ESI = &a + 4*[k/32]
         sub    EBP,EBX            ; EBP = d - [k/32]
;
         mov    EDX,1
         mov    EAX,[ESI]          ; EAX = a_[32/k]
         shl    EDX,CL 
         dec    EDX                ; EDX = 2^(k mod 32) - 1
         and    EDX,EAX            ; EDX = a_[32/k] & 2^(k mod 32) - 1
         mov    [ESI],EDX          ; Put back
         dec    EBP
         jz     Lsep8
         mov    EDX,4[ESI]         ; EDX = a_[32/k + 1]
         shrd   EAX,EDX,CL         ; Divide by 2^(k mod 32)
         mov    [EDI],EAX          ; Store quotient in q
         dec    EBP
         lea    ESI,4[ESI]
         lea    EDI,4[EDI]
         jz     Lsep6
;
Lsep4:   mov    EAX,0[ESI]         ; EAX = next a_i
         mov    EDX,4[ESI]         ; EDX = a_{i+1}
         shrd   EAX,EDX,CL         ; Divide by 2^(k mod 32)
         mov    dword ptr [ESI],0  ; Zap a_i
         mov    [EDI],EAX          ; Store quotient in q
         dec    EBP
         lea    ESI,4[ESI]
         lea    EDI,4[EDI]
         jg     Lsep4
;
Lsep6:   mov    EAX,[ESI]          ; EAX = last a_i
         mov    dword ptr [ESI],0  ; Zap a_i
Lsep8:   shr    EAX,CL             ; Divide by 2^(k mod 32)
         mov    [EDI],EAX          ; Store last quotient in q
;
         mov    EBP,Zbp
         mov    EDI,Zdi
         mov    ESI,Zsi
         retn
;==================================================
;
; nxtprm = xnxtprmk(k);
;
; Registers:   EAX = k
; Assumptions: k >= 0
; Returns:     EAX = next prime after k if k < 2^32
;              EAX = 1 otherwise
;
         public xnxtprmk_
xnxtprmk_: 
         cmp    EAX,2
         jb     Lnxtp2
         je     Lnxtp3
         mov    Zdx,EDX
         mov    Zbx,EBX
         mov    Zcx,ECX
         mov    EBX,EAX
         dec    EBX
         or     EBX,1
;
Lnxtpa:  add    EBX,2      ; next k
         jc     Lnxtpx     ; Return 1 if overlflow
         mov    ECX,1      ; ECX = c
Lnxtpb:  add    ECX,2      ; Next c
         mov    EAX,EBX
         xor    EDX,EDX
         div    ECX        ; EAX = [k/c], EDX = k mod c
         or     EDX,EDX
         jz     Lnxtpa     ; No remainder
         cmp    ECX,EAX
         jb     Lnxtpb     ; c < quotient
;
Lnxtpx:  mov    EAX,EBX    ; Return k
         mov    ECX,Zcx
         mov    EBX,Zbx
         mov    EDX,Zdx
         retn   
Lnxtp2:  mov    EAX,2
         retn   
Lnxtp3:  mov    EAX,3
         retn   
;==================================================
_AR2XA   endp
_TEXT    ends
         end

