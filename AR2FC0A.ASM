;
; AR2FC0A  2.1.5f  TONY FORBES  April 1999
;
; a = a^2  (using FFT modulo F_12)
;
; xsqufftC0 (&a, d, &f);
;
; Registers:   EAX = &a, EDX = d = d(a), 
;              EBX = &f, a 528513-digit work area (4097*129*4 = 2114052 bytes)
; Assumptions: d(a) > 0
;              a has sufficient space according to the 
;              following scheme:
;              
;                 0 < d(a) <=    504  :  cap(a) >= 2*d(a)
;               504 < d(a) <=   1008  :  cap(a) >= 2016
;              1008 < d(a) <=   2016  :  cap(a) >= 4032
;              2016 < d(a) <=   4032  :  cap(a) >= 8064
;              4032 < d(a) <=   8064  :  cap(a) >= 16128
;              8064 < d(a) <=  16128  :  cap(a) >= 32256
;             16128 < d(a) <=  32256  :  cap(a) >= 64512
;             32256 < d(a) <=  64512  :  cap(a) >= 129024
;             64512 < d(a) <= 129024  :  cap(a) >= 258048
;            129024 < d(a)            :  cap(a) >= 2*d(a)
;
;              max(2*d(a), min(4*d(a), 258048)) is sufficient
;
; Finite Fourier Transform for a => a^2.
;
; F  the Fermat number F_12 =  2^4096 + 1 (129 words).
; D  the dimension of the Fourier transform.
; G  a Dth root of unity modulo F.
; M  the base for representation of a, m = 2^2016 (63 words).
;
; Parameters for FFT with F = 2^4096 + 1 and M = 2^2016
;
;   log2 D     D           G           Nmax       Words
;    
;     12     4096          4         2^4128768   129024
;     11     2048         16         2^2064384    64512
;     10     1024        256         2^1032192    32256
;      9      512      65536         2^516096     16128
;      8      256       2^32         2^258048      8064
;      7      128       2^64         2^129024      4032
;      6       64      2^128         2^64512       2016
;      5       32      2^256         2^32256       1008
;
; F[X](k) = Sum[i=0 to D-1: X(i)G^k (mod F)
;
; Fast Fourier Transform
;
;    X(D,k,i) = X(i)
;    X(d,k,i) = X(2d,k,i) + X(2d,k,i+d)G^dk  (mod F)
;    F[X](k)  = X(1,k,0)
;
; Convolution Theorem:  F[(F[X]'.F[X]')] = D X*X,
;
; where  F[X]'  is the permutation of the vector  F[X]
; defined by  F[X]'(0) = F[X](0), F[X]'(i) = F[X](D-i),
; i = 1, 2, ..., D-1.
;
; a   = Sum[i=0 to D/2-1: 2^(iM) X(i)]
; a^2 = Sum[i=0 to D-2: 2^(iM) [X*X](i)]
; 
_DATA   segment dword public 'DATA'
         public _AR2FC0data
_AR2FC0data dd 0
         org    _AR2FC0data
Zcx      dd     0          ; Save ECX
Zbp      dd     0          ; Save EBP
Zsi      dd     0          ; Save ESI
Zdi      dd     0          ; Save EDI
ZYptr    dd     0          ; EAX = &a
ZXptr    dd     0          ; EBX = &f
;
; Constants for FFT array
LnF      =      129*4      ; length of F
LnM      =      63*4       ; length of M
Lk0      =      0          ; offset to block 0
Lk1      =      1024*129*4 ; offset to block 1
Lk2      =      2048*129*4 ; offset to block 2
Lk3      =      3072*129*4 ; offset to block 3
Lk4      =      4096*129*4 ; offset to end of FFT array
Li4      =      4096       ; index of end of FFT array
;
; FFT work areas
ZU       DD     257 dup (0)
ZW       DD     129 dup (0)
;
; Global FFT variables
ZGDim    DD     0        ; Dimension D
ZGlgD    DD     0        ; log2 D
ZGlgF    DD     0        ; log2 4096/D
ZGLbF    DD     0        ; F-block size = LnF*4096/D 
;
; Variables for XfftA
ZGe      DD     0
ZFd      DD     0
ZFi      DD     0
ZFe      DD     0
ZFb      DD     0
ZFj      DD     0
ZFk      DD     0
ZFkd     DD     0
;
_DATA   ends
;
_TEXT    segment dword public 'CODE'
         assume  CS:_TEXT
         assume  DS:DGROUP
_AR2FC0A proc    near
;========================================
;
; initialisation
         public xsqufftC0_
         extrn  xsqu_:NEAR
         extrn  xsqumodF12_:NEAR
xsqufftC0_:
;
;========================================
;
; a => a^2 by Finite Fourier Transform
;
; Choose FFT dimension or traditional method
;
; EDX = number of digits
;
         CMP    EDX,504
         JBE    xsqu_
         MOV    ZGlgD,5     ; Dimension 32
         CMP    EDX,1008
         JBE    Xfft
         MOV    ZGlgD,6     ; Dimension 64
         CMP    EDX,2016
         JBE    Xfft
         MOV    ZGlgD,7     ; Dimension 128
         CMP    EDX,4032
         JBE    Xfft
         MOV    ZGlgD,8     ; Dimension 256
         CMP    EDX,8064
         JBE    Xfft
         MOV    ZGlgD,9     ; Dimension 512
         CMP    EDX,16128
         JBE    Xfft
         MOV    ZGlgD,10    ; Dimension 1024
         CMP    EDX,32256
         JBE    Xfft
         MOV    ZGlgD,11    ; Dimension 2048
         CMP    EDX,64512
         JBE    Xfft
         MOV    ZGlgD,12    ; Dimension 4096
         CMP    EDX,129024
         JA     xsqu_       ; Too big for FFT
;
; Save registers
Xfft:    mov    Zcx,ECX
         mov    Zbp,EBP
         mov    Zsi,ESI
         mov    Zdi,EDI
         mov    ZYptr,EAX            ; &a
         mov    ZXptr,EBX            ; &f (fft array)
;
; Clear high order digits
         lea    EDI,[EAX+EDX*4]      ; First high order digit
         lea    ESI,[EAX+EDX*8]      ; Limit
Xfft1:   cmp    EDI,ESI
         jae    Xfft2
         mov    dword ptr [EDI],0
         lea    EDI,4[EDI]
         jmp    Xfft1
;
; Set up FFT parameters
Xfft2:   MOV    ECX,ZGlgD
         MOV    EAX,1
         SHL    EAX,CL
         MOV    ZGDim,EAX   ; Dimension
         NEG    ECX
         ADD    ECX,12
         MOV    EAX,LnF
         SHL    EAX,CL
         MOV    ZGLbF,EAX   ; LnF*4096/D
         MOV    ZGlgF,ECX   ; log2 4096/D
;
; First Fast Fourier Transform 
         CALL   XfftI       ; D/2 and D/4
         MOV    EAX,ZGDim
         SHR    EAX,3       ; D/8
         MOV    ZFd,EAX
         MOV    ZFb,512
         MOV    ZFj,512*LnF
         MOV    ZGe,4
Xfft4:   MOV    EAX,ZGe
         MOV    ZFe,EAX
         CALL   XfftA
         SHL    ZGe,1
         SHR    ZFj,1
         SHR    ZFb,1
         SHR    ZFd,1
         JNZ    Xfft4
;
; Reduce (mod F)
         CALL   XfftR
;
; X(i)^2 and permute X(i) <=> X(D - i)
         CALL   XfftQ
;
; Second Fast Fourier Transform 
         MOV    EAX,ZGDim
         SHR    EAX,1       ; D/2
         MOV    ZFd,EAX
         MOV    ZFb,2048
         MOV    ZFj,2048*LnF
         MOV    ZGe,1
Xfft6:   MOV    EAX,ZGe
         MOV    ZFe,EAX
         CALL   XfftA
         SHL    ZGe,1
         SHR    ZFj,1
         SHR    ZFb,1
         SHR    ZFd,1
         JNZ    Xfft6
;
; Reduce (mod F) such that X(i) = 0 (mod D) 
         CALL   XfftS
;
; y = X/D,  X(i) < D^2/2 M^2, i = 0, 1, ..., D-2,  X(D-1) = 0
;
         MOV    ECX,ZGlgD
         mov    ESI,ZXptr
         mov    EDI,ZYptr
         MOV    EAX,0[ESI]
;
         mov    EBP,4
Xfft6a:  MOV    EDX,[ESI+EBP]
         SHRD   EAX,EDX,CL
         MOV    -4[EDI+EBP],EAX
         MOV    EAX,4[ESI+EBP]
         SHRD   EDX,EAX,CL
         MOV    [EDI+EBP],EDX
         add    EBP,8
         cmp    EBP,512
         jb     Xfft6a
;
         MOV    dword ptr 512[EDI],0
         add    EDI,LnM
         MOV    EBX,2
         MOV    EBP,ZGDim
         SUB    EBP,2
Xfft8:   push   EBP
; ESI = &f + b(EBX)*129*4
         lea    EAX,8192[EBX]
         xor    ESI,ESI
         shr    EAX,1
Xfft8b:  rcl    ESI,1
         shr    EAX,1
         jnz    Xfft8b
         mov    EAX,ESI
         shl    ESI,9
         lea    ESI,[ESI+EAX*4]         
         add    ESI,ZXptr
;
         MOV    EAX,0[ESI]
;
         mov    EBP,4
Xfft8d:  MOV    EDX,[ESI+EBP]
         SHRD   EAX,EDX,CL
         MOV    ZU-4[EBP],EAX
         MOV    EAX,4[ESI+EBP]
         SHRD   EDX,EAX,CL
         MOV    ZU[EBP],EDX
         add    EBP,8
         cmp    EBP,512
         jb     Xfft8d
;
         MOV    EAX,ZU
         ADD    [EDI],EAX
;
         mov    EBP,4
Xfft8f:  MOV    EAX,ZU[EBP]
         ADC    [EDI+EBP],EAX
         lea    EBP,4[EBP]
         lea    EAX,1-260[EBP]
         dec    EAX
         jnz    Xfft8f
;
Xfft8h:  MOV    EAX,ZU[EBP]
         ADC    EAX,0
         MOV    [EDI+EBP],EAX
         lea    EBP,4[EBP]
         lea    EAX,1-512[EBP]
         dec    EAX
         jnz    Xfft8h
;
         MOV    dword ptr 512[EDI],0
         ADD    EDI,LnM
         pop    EBP
         ADD    EBX,2
         DEC    EBP
         JNZ    Xfft8
;
; All done, go back
Xfft9:   mov    EDI,Zdi
         mov    ESI,Zsi
         mov    EBP,Zbp
         mov    ECX,Zcx
         retn   
;========================================
; FFT special first step: y => X, d = D/4
;
; X(D/4, 0, i) = y(i) +         y(i + D/4)
; X(D/4, 1, i) = y(i) + G^(D/4)*y(i + D/4)
; X(D/4, 2, i) = y(i) -         y(i + D/4)
; X(D/4, 3, i) = y(i) - G^(D/4)*y(i + D/4)
;
; i = 0, 1, ..., D/4 - 1
;
; 0 <= y(i) < M
; G^(D/4) = sqrt(F-1) = 2^2048
;
XfftI:   mov    ESI,ZYptr
         mov    EDI,ZXptr
         MOV    EBX,LnM
         MOV    ECX,ZGlgD
         SUB    ECX,2
         SHL    EBX,CL    ; D/4*LnM
         add    EBX,ESI
;
; X(D/4, 2, i) = y(i) - y(i + D/4)
; X(D/4, 3, i) = y(i)
; 
; -M < X(D/4, 2, i) < M
;
XfftId:  mov    EBP,4
         MOV    EAX,[ESI]
         MOV    Lk3[EDI],EAX      ; Offset of X(D/4, 3, 0) is Lk3
         SUB    EAX,[EBX]
         MOV    Lk1[EDI],EAX      ; Offset of X(D/4, 2, 0) is Lk1
;
XfftIf:  MOV    EAX,[ESI+EBP]
         MOV    Lk3[EDI+EBP],EAX
         SBB    EAX,[EBX+EBP]
         MOV    Lk1[EDI+EBP],EAX
         lea    EBP,4[EBP]
         lea    EAX,1-252[EBP]
         dec    EAX
         jnz    XfftIf
;
         MOV    EAX,0
         mov    EBP,252+Lk1
         mov    dword ptr 252+Lk3[EDI],0
         SBB    EAX,0
XfftIh:  MOV    [EDI+EBP],EAX
         lea    EBP,4[EBP]
         cmp    EBP,512+Lk1
         jbe    XfftIh
;
; X(D/4,3,i) -= G^(D/4) y(i+D/4)
;
; -M*(F-1)/2 < X(D/4, 3, i) < M
;
         mov    EBP,4
         MOV    EAX,0
         SUB    EAX,[EBX]
         MOV    256+Lk3[EDI],EAX
;
XfftIj:  MOV    EAX,0
         SBB    EAX,[EBX+EBP]
         MOV    256+Lk3[EDI+EBP],EAX
         lea    EBP,4[EBP]
         lea    EAX,1-252[EBP]
         dec    EAX
         jnz    XfftIj
;
         MOV    EAX,0
         SBB    EAX,0
         MOV    508+Lk3[EDI],EAX
         MOV    512+Lk3[EDI],EAX
;
; X(D/4, 0, i) = y(i) +         y(i + D/4)
; X(D/4, 1, i) = y(i) + G^(D/4)*y(i + D/4)
;
; 0 <= X(D/4, 0, i) < 2*M
; 0 <= X(D/4, 1, i) < M*(F-1)/2
;
         mov    EBP,4
         MOV    EAX,[ESI]
         MOV    EDX,[EBX]
         MOV    Lk2[EDI],EAX     ; Offset of X(D/4, 1, 0) is Lk2
         MOV    256+Lk2[EDI],EDX
         ADD    EAX,EDX
         MOV    Lk0[EDI],EAX     ; Offset of X(D/4, 0, 0) is Lk0
         MOV    dword ptr 256+Lk0[EDI],0
XfftIm:  MOV    EAX,[ESI+EBP]
         MOV    EDX,[EBX+EBP]
         MOV    Lk2[EDI+EBP],EAX
         MOV    256+Lk2[EDI+EBP],EDX
         ADC    EAX,EDX
         MOV    Lk0[EDI+EBP],EAX
         MOV    dword ptr 256+Lk0[EDI+EBP],0
         lea    EBP,4[EBP]
         lea    EAX,1-252[EBP]
         dec    EAX
         jnz    XfftIm
;
         mov    EAX,0
         adc    EAX,0
         MOV    dword ptr 252+Lk2[EDI],0
         MOV    dword ptr 508+Lk2[EDI],0
         MOV    dword ptr 252+Lk0[EDI],EAX
         MOV    dword ptr 508+Lk0[EDI],0
         MOV    dword ptr 512+Lk2[EDI],0
         MOV    dword ptr 512+Lk0[EDI],0
; Next i
         mov    EAX,ZXptr
         ADD    EDI,ZGLbF
         add    EAX,Lk1
         ADD    ESI,LnM  
         ADD    EBX,LnM
         CMP    EDI,EAX
         jb     XfftId
         RETN
;====================================================
; FFT general step
;
; X(d, k, i)   = X(2d, k, i) + G^(kd)*X(2d, k, i+d)
; X(d, k+e, i) = X(2d, k, i) - G^(kd)*X(2d, k, i+d)
;
; k = 0, ..., e-1,  i = 0, ..., d-1
;
; ZFd  = d,            ZFe = e, 
; ZFb  = d*4096/D,     ZFj = offset to X(2d, k, i+d)
; ZFkd = kd
;
XfftA:   mov    ZFk,0          ; k
         mov    ZFkd,0         ; kd
XfftAa:  xor    EDI,EDI        ; EDI = &f + b(k)*129*4
         MOV    ZU+0,0         ; Clear low-order U
         mov    EAX,ZFk
         MOV    ZU+4,0
         lea    EAX,4096[EAX]
         MOV    ZU+8,0
         shr    EAX,1
XfftAb:  rcl    EDI,1
         shr    EAX,1
         jnz    XfftAb   
         mov    EAX,EDI
         MOV    ZU+12,0
         shl    EDI,9
         MOV    ZU+16,0
         lea    EDI,[EDI+EAX*4]         
         MOV    ZU+20,0
         add    EDI,ZXptr
;
         MOV    ZU+24,0
         mov    EAX,ZFd
         MOV    ZU+28,0
         mov    ZFi,EAX        ; i  = 0, ..., d-1
         MOV    ZU+32,0
         mov    EBP,40
         MOV    ZU+36,0
XfftAc:  MOV    ZU[EBP],0
         MOV    ZU+4[EBP],0
         lea    EBP,8[EBP]
         cmp    EBP,512
         jb     XfftAc
;
XfftAd:  MOV    ESI,ZFj
         ADD    ESI,EDI        ; ESI -> X(2d, k, i+d)
         mov    EBX,ZFkd
         shr    EBX,2          ; EBX = [kd/4]
; Clear high-order U
         MOV    EAX,512[ESI]
         SAR    EAX,31
         mov    EBP,516
XfftAe:  MOV    ZU[EBP],EAX
         MOV    ZU+4[EBP],EAX
         lea    EBP,8[EBP]
         cmp    EBP,1028
         jb     XfftAe
         TEST   byte ptr ZFkd,3
         JNZ    XfftAshf
; U = G^kd X(2d, k, i+d),  kd = 0 (mod 4)
         mov    EBP,0
XfftAf:  mov    EAX,0[ESI+EBP]
         mov    EDX,4[ESI+EBP]
         mov    ZU+0[EBX+EBP],EAX
         mov    ZU+4[EBX+EBP],EDX
         lea    EBP,8[EBP]
         cmp    EBP,512
         jb     XfftAf
;
         mov    EAX,512[ESI]
         mov    ZU+512[EBX],EAX
XfftAshfret:
; Reduce G^kd X(2d, k, i+d)
         mov    EAX,ZU+0
;
         mov    EDX,ZU+512+0
         mov    EBX,ZU+4
         sub    EAX,EDX
         mov    ECX,ZU+512+4
         mov    ZW+0,EAX
         sbb    EBX,ECX
         mov    EAX,ZU+8
         mov    ZW+4,EBX
;
         mov    EBP,12
XfftAh:  mov    EDX,ZU+512-4[EBP]
         mov    EBX,ZU[EBP]
         sbb    EAX,EDX
         mov    ECX,ZU+512[EBP]
         mov    ZW-4[EBP],EAX
         sbb    EBX,ECX
         mov    EAX,ZU+4[EBP]
         mov    ZW[EBP],EBX
         lea    EBP,8[EBP]
         lea    EDX,1-508[EBP]
         dec    EDX
         jnz    XfftAh
;
         mov    EDX,ZU+512+504
         mov    EBX,ZU+508
         sbb    EAX,EDX
         mov    ECX,ZU+512+508
         mov    ZW+504,EAX
         sbb    EBX,ECX
         mov    EAX,0
         mov    ZW+508,EBX
;
         sbb    EAX,ZU+512+512
         mov    ZW+512,EAX
; X(d, k+e, i) = X(2d, k, i) - G^kd*X(2d, k, i+d)
         mov    EAX,0[EDI]
;
         mov    EDX,ZW+0
         mov    EBX,4[EDI]
         sub    EAX,EDX
         mov    ECX,ZW+4
         mov    0[ESI],EAX
         sbb    EBX,ECX
         mov    EAX,8[EDI]
         mov    4[ESI],EBX
;
         mov    EBP,12
XfftAj:  mov    EDX,ZW-4[EBP]
         mov    EBX,[EDI+EBP]
         sbb    EAX,EDX
         mov    ECX,ZW[EBP]
         mov    -4[ESI+EBP],EAX
         sbb    EBX,ECX
         mov    EAX,4[EDI+EBP]
         mov    [ESI+EBP],EBX
         lea    EBP,8[EBP]
         lea    EDX,1-516[EBP]
         dec    EDX
         jnz    XfftAj
;
         sbb    EAX,ZW+512
         mov    512[ESI],EAX
; X(d, k, i) = X(2d, k, i) + G^kd*X(2d, k, i+d)
         mov    EAX,0[EDI]
;
         mov    EDX,ZW+0
         mov    EBX,4[EDI]
         add    EAX,EDX
         mov    ECX,ZW+4
         mov    0[EDI],EAX
         adc    EBX,ECX
         mov    EAX,8[EDI]
         mov    4[EDI],EBX
;
         mov    EBP,12
XfftAm:  mov    EDX,ZW-4[EBP]
         mov    EBX,[EDI+EBP]
         adc    EAX,EDX
         mov    ECX,ZW[EBP]
         mov    -4[EDI+EBP],EAX
         adc    EBX,ECX
         mov    EAX,4[EDI+EBP]
         mov    [EDI+EBP],EBX
         lea    EBP,8[EBP]
         lea    EDX,1-516[EBP]
         dec    EDX
         jnz    XfftAm
;
         adc    EAX,ZW+512
         mov    512[EDI],EAX
; Next i
         ADD    EDI,ZGLbF
         DEC    ZFi
         JNZ    XfftAd
; Next k
         inc    ZFk
         mov    EAX,ZFb
         ADD    ZFkd,EAX
         DEC    ZFe
         JNZ    XfftAa
         RETN
;
; U = G^kd X(2d, k, i+d),  (kd mod 4) = 1, 2 or 3
XfftAshf:
         mov    ECX,ZFkd
         AND    ECX,3
         add    ECX,ECX
         MOV    EAX,512[ESI]
;
         mov    EBP,508
XfftAp:  MOV    EDX,[ESI+EBP]
         SHLD   EAX,EDX,CL
         MOV    ZU+4[EBX+EBP],EAX
         MOV    EAX,-4[ESI+EBP]
         SHLD   EDX,EAX,CL
         MOV    ZU+0[EBX+EBP],EDX
         sub    EBP,8
         ja     XfftAp
;
         SHL    EAX,CL
         MOV    ZU+0[EBX],EAX
         jmp    XfftAshfret
;====================================================
; Reduce X(i) modulo F, i = 0, 1, ..., D-1
;
; Before:  -2^4127 < X(i) < 2^4127
; After:   0 <= X(i) <= 2^4096  
;
; Let X = X(i) = A + 2^4096 B
;
; If X >= 0 then X = A - B (mod F)
;           If A - B < 0 then add F
;
; If X <  0 then X = A + 2^4096*B - 2^4128
;                  = A + 2^32 - B (mod F)
;
XfftR:   MOV    EDI,ZXptr
         MOV    ECX,ZGDim
;
XfftRb:  MOV    EAX,512[EDI]
         CMP    EAX,0
         JE     XfftRx
         MOV    dword ptr 512+[EDI],0
         JL     XfftRn
; X(i) >= 0
         SUB    0[EDI],EAX
         SBB    dword ptr 4[EDI],0
         JNC    XfftRx
         SBB    dword ptr 8[EDI],0
         JNC    XfftRx
;
         lea    EBX,12[EDI]
         mov    EBP,128-3
XfftRc:  SBB    dword ptr [EBX],0
         JNC    XfftRx
         lea    EBX,4[EBX]
         dec    EBP
         jnz    XfftRc
;
         ADC    dword ptr 0[EDI],0
         JNC    XfftRx
         ADC    dword ptr 4[EDI],0
         JNC    XfftRx
XfftRg:  ADC    dword ptr 8[EDI],0
         JNC    XfftRx
;
         lea    EBX,12[EDI]
         mov    EBP,128-3
XfftRi:  ADC    dword ptr [EBX],0
         JNC    XfftRx
         lea    EBX,4[EBX]
         dec    EBP
         jnz    XfftRi
; Must be  a + 2^4096,  a < 2^32 
         ADC    dword ptr 512[EDI],0
         TEST   dword ptr 0[EDI],0FFFFFFFFh
         JZ     XfftRx
         DEC    dword ptr 0[EDI]
         MOV    dword ptr 512[EDI],0
         JMP    XfftRx
; X(i) < 0
XfftRn:  SUB    0[EDI],EAX
         JC     XfftRx
         ADD    dword ptr 4[EDI],1
         JC     XfftRg
;
XfftRx:  ADD    EDI,ZGLbF
         DEC    ECX
         JNZ    XfftRb
         RETN
;====================================================
; Determine X(i) from X(i) modulo F and D, 
; i = 0, 1, ..., D-2
;
; Before:  -2^4127 < X(i) < 2^4127
; After:   0 <= X(i) <= D*2^4096, D|X(i)  
;
; As XfftR, but must add  cF  if  X(i) = -c (mod D)
;
XfftS:   MOV    EDI,ZXptr
         MOV    ECX,ZGDim
         DEC    ECX
;
XfftSb:  MOV    EAX,512[EDI]
         CMP    EAX,0
         JE     XfftSt
         MOV    dword ptr 512+[EDI],0
         JL     XfftSn
; X(i) >= 0
         SUB    0[EDI],EAX
         SBB    dword ptr 4[EDI],0
         JNC    XfftSt
         SBB    dword ptr 8[EDI],0
         JNC    XfftSt
;
         lea    EBX,12[EDI]
         mov    EBP,128-3
XfftSc:  SBB    dword ptr [EBX],0
         JNC    XfftSt
         lea    EBX,4[EBX]
         dec    EBP
         jnz    XfftSc
;
         ADC    dword ptr 0[EDI],0
         JNC    XfftSt
         ADC    dword ptr 4[EDI],0
         JNC    XfftSt
XfftSg:  ADC    dword ptr 8[EDI],0
         JNC    XfftSt
;
         lea    EBX,12[EDI]
         mov    EBP,128-3
XfftSi:  ADC    dword ptr [EBX],0
         JNC    XfftSt
         lea    EBX,4[EBX]
         dec    EBP
         jnz    XfftSi
; Must be  a + 2^4096,  a < 2^32 
         ADC    dword ptr 512[EDI],0
         TEST   dword ptr 0[EDI],0FFFFFFFFh
         JZ     XfftSt
         DEC    dword ptr 0[EDI]
         MOV    dword ptr 512[EDI],0
         JMP    XfftSt
; X(i) < 0
XfftSn:  SUB    0[EDI],EAX
         JC     XfftSt
         ADD    dword ptr 4[EDI],1
         JC     XfftSg
;
XfftSt:  MOV    EAX,0[EDI]
         MOV    EDX,ZGDim
         DEC    EDX           ; D - 1
         AND    EAX,EDX
         JZ     XfftSx
         NEG    EAX
         ADC    EAX,EDX       ; D - X
         ADD    512[EDI],EAX
         ADD    0[EDI],EAX
         ADC    dword ptr 4[EDI],0
         JNC    XfftSx
         ADC    dword ptr 8[EDI],0
         JNC    XfftSx
;
         lea    EBX,12[EDI]
         mov    EBP,128-2
XfftSv:  ADC    dword ptr [EBX],0
         JNC    XfftSx
         lea    EBX,4[EBX]
         dec    EBP
         jnz    XfftSv
XfftSx:  ADD    EDI,ZGLbF
         DEC    ECX
         JNZ    XfftSb
         RETN
;====================================================
;
; X(i) = X(D-b(i))^2
;
XfftQ:   
;
; X(i) = X(b(i))
         mov    EBX,1
XfftQa:  mov    ESI,EBX
         mov    EAX,EBX
         xor    EDI,EDI
         add    EAX,ZGDim
         shr    EAX,1
XfftQc:  rcl    EDI,1
         shr    EAX,1
         jnz    XfftQc
         cmp    ESI,EDI
         jnb    XfftQg
; Swap X(i) and X(b(i) if i < b(i)
         mov    ECX,ZGlgF
         shl    ESI,CL
         shl    EDI,CL
         mov    EAX,ESI
         mov    EDX,EDI
         shl    ESI,9
         shl    EDI,9
         lea    ESI,[ESI+EAX*4]         
         lea    EDI,[EDI+EDX*4]         
         add    ESI,ZXptr
         add    EDI,ZXptr
         mov    EBP,512
XfftQe:  mov    EAX,[ESI+EBP]
         mov    EDX,[EDI+EBP]
         mov    [EDI+EBP],EAX
         mov    [ESI+EBP],EDX
         sub    EBP,4
         jnb    XfftQe
;
XfftQg:  inc    EBX
         cmp    EBX,ZGDim
         jb     XfftQa
;
; X(0) = X(0)^2 (mod F)
         mov    EAX,ZXptr
         mov    EDX,ZXptr
         call   xsqumodF12_
;
; X(2048) = X(2048)^2 (mod F)
         mov    EAX,ZXptr
         mov    EDX,ZXptr
         add    EAX,Lk2
         add    EDX,Lk2
         call   xsqumodF12_
;
; X(D - i) = X(i)^2 (mod F)
; X(i) = X(D - i)^2 (mod F)
         mov    EBX,Lk2
         sub    EBX,ZGLbF
XfftQm:  mov    EAX,ZXptr
         mov    EDX,ZXptr
         add    EAX,Lk4
         add    EDX,EBX      ; EDX -> X(i)
         sub    EAX,EBX      ; EAX -> X(D-i)
; ZW = X(D-i)
         mov    EBP,512
XfftQn:  mov    ECX,[EAX+EBP]
         mov    ZW[EBP],ECX
         sub    EBP,4
         jnb    XfftQn
;
         push   EDX          ; EDX -> X(i)
         call   xsqumodF12_  ; X(D-i) = X(i)^2
         lea    EDX,ZW
         pop    EAX          ; EAX -> X(i)
         call   xsqumodF12_  ; X(i) = X(D-i)^2
;
         sub    EBX,ZGLbF
         ja     XfftQm
         retn
;====================================================
;
_AR2FC0A endp
_TEXT    ends
         END    
