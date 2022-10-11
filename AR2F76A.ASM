;
; AR2F76A 2.1.5f  TONY FORBES  April 1999
;
; a = b^2 (mod F_12)  using discrete, weighted FFT mod F_7, dimension 64.
;
; Time (P200 MMX under DOS/4GW): 0.000395 sec.
;
; xsqumodF12 (&a, &b);
;
; Registers:   EAX = &a, EDX = &b, a and b may overlap.
;
; Assumptions: 0 <= b <= 2^4096.
;              a has 129 32-bit words and is zero-padded if necessary.
;              b has 129 32-bit words allocated.
;
; Discrete, weighted Fourier Transform for b => b^2 (mod F_12).
;
; F = Fermat number F_7 = 2^128 + 1
; D = 64, the dimension of the Fourier transform
; G = 16, a primitive D'th root of unity modulo F
; H = 4 = sqrt(G), weight generator, H^D = -1 (mod F)
; M = 2^64, base for representation of a
;
; Fourier transform
;
;    F[X](k) = Sum{i=0 to D-1: X(i)*G^(k*i)}  (mod F)
;
; Fast Fourier transform
;
;    X(D, 0, i)   = X(i)
;    X(d, k, i)   = X(2*d, k, i) + G^(d*k)*X(2*d, k, i+d)  (mod F)
;    X(d, k+e, i) = X(2*d, k, i) - G^(d*k)*X(2*d, k, i+d)  (mod F)
;    F[X](k)      = X(1, k, 0)
;
;    i = 0, 1, ..., d - 1
;    k = 0, 1, ..., e - 1
;    d = D/2, D/4, ..., 1, e = D/(2*d)
;
; b    = Sum[i=0 to D-1: B(i)*M^i]
;
; T(i) = H^(D-i) * B(i)  (mod F)
;
; U(0) = F[T](0)^2       (mod F)
; U(k) = F[T](D-k)^2     (mod F), k = 1, 2, ..., D-1
;
; V(i) = 1/D * H^i * F[U](i)  (mod F)
; 
; V(i) = [B**B](i) - [B**B](D+i)  (mod F)  (Convolution Theorem)
;
; W(i) = [B**B](i) - [B**B](D+i)  (mod D)
;
; Y(i) = F*((W(i) - V(i)) mod D) + V(i)  (mod D*F), and
;        (i+1-D)*M^2 < Y(i) < (i+1)*M^2
;
; a = b^2 = Sum[i=0 to D-1: Y(i)*M^i] (mod F_12)
; 
_DATA    segment dword public 'DATA'
         public _AR2XF76data
_AR2XF76data dd 0
         org    _AR2XF76data
ZAptr    dd     0        ; EAX = &a
ZBptr    dd     0        ; EDX = &b
;
LnF      =      5*4      ; length of F
LnM      =      2*4      ; length of M
Lk1      =      16*5*4   ; offset to block 1 in FFT array
Lk2      =      32*5*4   ; offset to block 2 in FFT array
Lk3      =      48*5*4   ; offset to block 3 in FFT array
Lk4      =      64*5*4   ; offset to end of FFT array
;
ZB       dd     5 dup (0)
ZSm1     dd     0
ZS       dd     6 dup (0)
         org    ZSm1
ZW       dd     72 dup (0)
;
ZFd      dd     0
ZFe      dd     0
ZFs      dd     0
ZFk      dd     0
ZFkd     dd     0
ZFi      dd     0
ZFj      dd     0
ZQc      dd     0
;
; Bit reversal table, i => 4 * b(i), 
; where b(i) = i bit-reversed modulo D
;
Tb  dd   0,128,64,192,32,160,96,224
    dd   16,144,80,208,48,176,112,240
    dd   8,136,72,200,40,168,104,232
    dd   24,152,88,216,56,184,120,248
    dd   4,132,68,196,36,164,100,228
    dd   20,148,84,212,52,180,116,244
    dd   12,140,76,204,44,172,108,236
    dd   28,156,92,220,60,188,124,252
;
; Table for controlling X => X^2
;
; X(0) = X(0)^2, 
; X(D - b(k)) = X(k)^2,  k = 1, 2, ..., D-1
;
; k => D - b(k) cycle structure
;
; (0) c = 1 
; (1,32,63) c = 3 
; (2,48,61,17,30,34,47,3,16,62,33,31) c = 12 
; (4,56,57,25,26,42,43,11,12,52,53,21,22,38,39,7,8,60,49,29,18,46,35,15) c = 24 
; (5,24,58,41,27,10,44,51,13,20,54,37,23,6,40,59,9,28,50,45,19,14,36,55) c = 24 
;
Tq  dd   Lk4
    dd   0,Lk4; c = 1 
    dd   20,640,1260,Lk4; c = 3 
    dd   40,960,1220,340,600,680,940,60,320,1240,660,620,Lk4; c = 12 
    dd   80,1120,1140,500,520,840,860,220,240,1040,1060,420 
    dd   440,760,780,140,160,1200,980,580,360,920,700,300,Lk4; c = 24 
    dd   100,480,1160,820,540,200,880,1020,260,400,1080,740 
    dd   460,120,800,1180,180,560,1000,900,380,280,720,1100,Lk4; c = 24
;
ZX       dd     325 dup (0)
_DATA   ends
;
_TEXT    segment dword public 'CODE'
         assume  CS:_TEXT
         assume  DS:DGROUP
_AR2F76A proc    near
;========================================
;
; a = b^2 mod F12 by discrete, weighted Fourier transform
;
         public  xsqumodF12_
         extrn   xsqu_:NEAR
xsqumodF12_:
;
; Save registers and parameters
;
         push   EBX
         push   ECX
         push   EBP
         push   ESI
         push   EDI
         mov    ZAptr,EAX       ; &a
         mov    ZBptr,EDX       ; &b
;
; Deal with b = 2^4096
;
         cmp    dword ptr 512[EDX],1
         jne    Xfft3
; Set a = 1
         mov    dword ptr [EAX],1
         mov    ECX,128
Xfft1:   mov    dword ptr 4[EAX],0
         lea    EAX,4[EAX]
         dec    ECX
         jnz    Xfft1
         jmp    Xfft9
;
; Compute T(i) = H^(D-i) * B(i) and transfer to FFT array
;
Xfft3:   mov    EBP,64          ; EBP = D
         lea    ESI,ZB          ; ESI -> B
         lea    EDI,ZX          ; EDI -> X
Xfft3a:  push   EDX             ; EDX -> B(i)
         push   EBP
         mov    EBX,0[EDX]      ; B = B(i)
         mov    ECX,4[EDX]
         mov    ZB+0,EBX
         mov    ZB+4,ECX
         call   XfftS           ; S = T(i) = H^(D-i) * B(i)
         mov    EAX,ZS+0        ; X(i) = T(i) = H^(D-i) * B(i)
         mov    EDX,ZS+4
         mov    EBX,ZS+8
         mov    0[EDI],EAX
         mov    4[EDI],EDX
         mov    8[EDI],EBX
         mov    EAX,ZS+12
         mov    EBX,ZS+16
         pop    EBP
         pop    EDX
         dec    EBP
         lea    EDX,lnM[EDX]
         mov    12[EDI],EAX
         mov    16[EDI],EBX
         lea    EDI,LnF[EDI]
         jnz    Xfft3a
;
; First FFT
;
         MOV    ZFd,32
         MOV    ZFe,1
         MOV    ZFj,32*LnF
         CALL   XfftA           ; 6-stage FFT
;
; X(0) = U(0) = F[T](0)^2
; X(k) = U(k) = F[T](D-k)^2, i = 1, 2, ..., D-1
;
         CALL   XfftQ
;
; Second FFT
;
         MOV    ZFd,32
         MOV    ZFe,1
         MOV    ZFj,32*LnF
         CALL   XfftA           ; 6-stage FFT
;
; Compute V(i) = 1/D * H^i * U(i)         (mod F)
;              = [B**B](i) - [B**B](D+i)  (mod F)
;
; (Index i is bit-reversed)
;
         xor    EBP,EBP           ; EBP = 4*i
Xfft4d:  push   EBP
         mov    ECX,Tb[EBP]       ; ECX = 4*b(i)
         cmp    EBP,ECX
         jnl    Xfft4h
; X(i) <=> X(b(i))
         lea    ESI,ZX[EBP+EBP*4] ; ESI -> X(i)
         lea    EDI,ZX[ECX+ECX*4] ; EDI -> X(b(i))
         mov    EAX,0[ESI]        ; Swap
         mov    EDX,0[EDI]
         mov    0[EDI],EAX
         mov    0[ESI],EDX
         mov    EAX,4[ESI]
         mov    EDX,4[EDI]
         mov    4[EDI],EAX
         mov    4[ESI],EDX
         mov    EAX,8[ESI]
         mov    EDX,8[EDI]
         mov    8[EDI],EAX
         mov    8[ESI],EDX
         mov    EAX,12[ESI]
         mov    EDX,12[EDI]
         mov    12[EDI],EAX
         mov    12[ESI],EDX
         mov    EAX,16[ESI]
         mov    EDX,16[EDI]
         mov    16[EDI],EAX
         mov    16[ESI],EDX
;
Xfft4h:  lea    ESI,ZX[EBP+EBP*4] ; ESI -> X(i) = U(i)
         shr    EBP,2             ; EBP = i
         add    EBP,125           ; 1/D = H^125
         CALL   XfftS             ; S = 1/D * H^i * U(i)
         mov    EAX,ZS+0
         mov    EDX,ZS+4
         mov    EBX,ZS+8
         mov    0[ESI],EAX
         mov    4[ESI],EDX
         mov    8[ESI],EBX
         mov    EAX,ZS+12
         mov    EDX,ZS+16
         mov    12[ESI],EAX
         mov    16[ESI],EDX
;
         pop    EBP
         add    EBP,4
         cmp    EBP,4*64
         jb     Xfft4d
;
; Compute W(i) = [B**B](i) - [B**B](D+i)  (mod D)
;
; Set up W: 64 blocks of 18 bits = 36 words
;
; Bits 0-5: B(i), bits 6-17: zeros
;
         mov    ESI,ZBptr
         lea    EDI,ZW 
         mov    ECX,16              ; Do 4 at a time
Xfft5b:  mov    EAX,0[ESI]          ; EAX = B(i)
         mov    EDX,8[ESI]          ; EDX = B(i+1)
         and    EAX,63              ; EAX = B(i) mod D
         and    EDX,63              ; EDX = B(i+1) mod D
         shl    EDX,18
         or     EDX,EAX
         mov    0[EDI],EDX          ; 00000000bbbbbb000000000000bbbbbb
         mov    EAX,16[ESI]         ; EAX = B(i+2)
         mov    EDX,24[ESI]         ; EDX = B(i+3)
         and    EAX,63              ; EAX = B(i+2) mod D
         and    EDX,63              ; EDX = B(i+3) mod D
         shl    EAX,4
         shl    EDX,22
         or     EDX,EAX
         mov    4[EDI],EDX          ; 0000bbbbbb000000000000bbbbbb0000
         mov    byte ptr 8[EDI],0
         lea    ESI,32[ESI]
         lea    EDI,9[EDI]
         dec    ECX
         jnz    Xfft5b
; Square W (36 words)
         lea    EAX,ZW
         mov    EDX,36
         call   xsqu_
;
; Compute Y(i) = [B**B](i) - [B**B](D+i)
;              = F*((W(i) - V(i)) mod D) + V(i)  (mod D*F),
;                (i+1-D)*M^2 < Y(i) < (i+1)*M^2
;
         lea    ESI,ZX
         mov    EDI,0
Xfft5f:  lea    EBP,[EDI+EDI*8]
         add    EBP,EBP             ; EBP = 18*i
         call   XmodF
         mov    ECX,EBP             ; ECX = 18*i
; Extract [B**B](i) and [B**B](D+i)
         shr    EBP,5
         and    ECX,31
         mov    EAX,ZW+0[EBP*4]
         mov    EDX,ZW+4[EBP*4]
         shrd   EAX,EDX,CL          ; EAX = [B**B](i) (mod D)
         mov    EDX,ZW+144[EBP*4]
         mov    EBX,ZW+148[EBP*4]
         shrd   EDX,EBX,CL          ; EDX = [B**B](D+i) (mod D)
         sub    EAX,EDX             ; EAX = W(i) (mod D)
         sub    EAX,[ESI]
         and    EAX,63              ; EAX = W(i) - V(i) mod D
; Add F*((W(i) - V(i) mod D) to V(i)
         mov    EDX,16[ESI]
         add    0[ESI],EAX
         adc    dword ptr 4[ESI],0
         adc    dword ptr 8[ESI],0
         adc    dword ptr 12[ESI],0
         adc    EDX,EAX
; Adjust Y(i)
         lea    EBP,-64[EDI]
         cmp    EDX,EDI             ; EDI = i
         jle    Xfft5h              ; Y(i) < (i+1)*M^2
         sub    dword ptr 0[ESI],64 ; Y(i) >= (i+1)*M^2 - Subtract D*F
         sbb    dword ptr 4[ESI],0
         sbb    dword ptr 8[ESI],0
         sbb    dword ptr 12[ESI],0
         sbb    EDX,64
         jmp    Xfft5j
Xfft5h:  cmp    EDX,EBP             ; EBP = i-D
         jge    Xfft5j              ; Y(i) > (i+1-D)*M^2
         add    dword ptr 0[ESI],64 ; Y(i) <= (i+1-D)*M^2 - Add D*F
         adc    dword ptr 4[ESI],0
         adc    dword ptr 8[ESI],0
         adc    dword ptr 12[ESI],0
         adc    EDX,64
Xfft5j:  mov    16[ESI],EDX
;
         lea    ESI,LnF[ESI]
         inc    EDI
         cmp    EDI,64
         jb     Xfft5f
;
; Return a = b^2 (mod F_12)                 
;                                       
;                     seeeddddccccbbbbaaaa  mov
;                     ccccbbbbaaaa          add adc adc
;             seeedddd                      adc s, adc s
;             ccccbbbbaaaa                  add adc adc 
;     seeedddd                              adc s, adc s
;
         lea    ESI,ZX
         mov    EDI,ZAptr
; A(0)
         mov    EAX,0[ESI]
         mov    EDX,4[ESI]
         mov    EBX,8[ESI]
         mov    0[EDI],EAX
         mov    4[EDI],EDX
         mov    8[EDI],EBX
         mov    EAX,12[ESI]
         mov    EBP,16[ESI]
         mov    16[EDI],EBP
         mov    12[EDI],EAX
         sar    EBP,31               ; EBP = sgn(Y(0))
         add    ESI,LnF
         add    EDI,LnM
; A(1), A(2), ..., A(61)
         mov    ECX,61
Xfft8b:  mov    EAX,0[ESI]
         mov    EDX,0[EDI]
         push   ECX
         mov    EBX,4[ESI]
         mov    ECX,4[EDI]
         add    EDX,EAX
         adc    ECX,EBX
         mov    0[EDI],EDX
         mov    4[EDI],ECX
         mov    EAX,8[ESI]
         mov    EDX,8[EDI]
         mov    EBX,12[ESI]
         mov    ECX,16[ESI]
         adc    EDX,EAX
         adc    EBX,EBP              ; Add sign
         adc    EBP,ECX              ; Add to sign
         mov    8[EDI],EDX
         mov    16[EDI],EBP          ; High-order digit
         mov    12[EDI],EBX
         sar    EBP,31               ; EBP = new sign
         pop    ECX
         lea    ESI,LnF[ESI]
         lea    EDI,LnM[EDI]
         dec    ECX
         jnz    Xfft8b
; A(62) .   .   .EDX|EAX.AAA:AAA.AAA:<- EDI
         mov    EAX,0[ESI]
         mov    EDX,4[ESI]
         mov    EBX,8[ESI]
         add    EAX,0[EDI]
         adc    EDX,4[EDI]
         adc    EBX,8[EDI]
         mov    0[EDI],EAX
         mov    4[EDI],EDX
         mov    8[EDI],EBX
         mov    EAX,12[ESI]
         mov    EDX,16[ESI]
         adc    EAX,EBP              ; Add sign
         adc    EDX,EBP              ; Add sign
; A(63) .ECX.EBX.EDX|EAX.AAA:AAA.AAA:<- EDI
         mov    EBX,20[ESI]
         mov    EBP,EDX
         mov    ECX,8[EDI]
         sar    EBP,31               ; New sign
         add    ECX,EBX
         mov    EBX,24[ESI]
         mov    8[EDI],ECX
         mov    ECX,28[ESI]
         adc    EAX,EBX
         adc    EDX,ECX
         mov    12[EDI],EAX
         mov    dword ptr 16[EDI],0  ; Clear high-order digit
         mov    EBX,32[ESI]
         mov    ECX,36[ESI]
         adc    EBX,EBP              ; Add sign
         adc    ECX,EBP              ; Add sign
; Wrap last three digits
         mov    EDI,ZAptr
         cmp    ECX,0
         jl     Xfft8n
; ECX.EBX.EDX >= 0
         sub    0[EDI],EDX
         sbb    4[EDI],EBX
         sbb    8[EDI],ECX
         sbb    dword ptr 12[EDI],0
         jnc    Xfft8x
         call   XsubC
; ECX.EBX.EDX < 0
Xfft8n:  sub    0[EDI],EDX
         sbb    4[EDI],EBX
         sbb    8[EDI],ECX
         jc     Xfft8x
         add    dword ptr 12[EDI],1
         jnc    Xfft8x
         call   XaddC
Xfft8x:
;
; All done; restore registers and go back
Xfft9:   pop    EDI
         pop    ESI
         pop    EBP
         pop    ECX
         pop    EBX
Xretn:   retn   
;
;====================================================
; Add carry to 16[EDI] and tidy up
;
XaddC:   mov    ECX,129-4
XaddCb:  adc    dword ptr 16[EDI],0
         jnc    Xretn
         lea    EDI,4[EDI]
         dec    ECX
         jnz    XaddCb
         call   XsubF12       ; Overflow: subtract F12
         jnc    Xretn
         call   XaddF12       ; Negative: add F12
         retn   
;====================================================
; Subtract carry from 16[EDI] and tidy up
;
XsubC:   mov    ECX,129-4
XsubCb:  sbb    dword ptr 16[EDI],0
         jnc    Xretn
         lea    EDI,4[EDI]
         dec    ECX
         jnz    XsubCb
         call   XaddF12       ; Negative: add F12
         retn     
;====================================================
; Add F12 to A
;
XaddF12:  mov    EDI,ZAptr
          mov    ECX,129
          add    dword ptr 512[EDI],1
XaddF12b: add    dword ptr [EDI],1 
          jnc    Xretn
          lea    EDI,4[EDI]
          dec    ECX
          jnz    XaddF12b
          retn   
;====================================================
; Subtract F12 from A and set carry if negative
;
XsubF12:  mov    EDI,ZAptr
          mov    ECX,129
          sub    dword ptr 512[EDI],1
XsubF12b: sub    dword ptr [EDI],1 
          jnc    Xretn
          lea    EDI,4[EDI]
          dec    ECX
          jnz    XsubF12b
          retn   
;====================================================
; FFT general step
;
; X(d, k, i)   = X(2d, k, i) + G^(kd) * X(2d, k, i+d)
; X(d, k+e, i) = X(2d, k, i) - G^(kd) * X(2d, k, i+d)
;
; k = 0, ..., e-1,  i = 0, ..., d-1
;
; ZFd = d, ZFe = e, ZFj = offset to X(2d, k, i+d)
; ZFk = k,          ZFi = counter for i
;
XfftA:   MOV    ZFk,0             ; k
         MOV    ZFkd,0            ; kd
XfftAb:  MOV    EBX,ZFk           ; EBX = k
         MOV    EDI,Tb[EBX*4]     ; EDI = 4 * (k bit-reversed)
         MOV    EAX,ZFd           ; d
         lea    EDI,ZX[EDI+EDI*4] ; EDI -> X(2d, k, i)
         MOV    ZFi,EAX           ; counter for i
;
XfftAd:  MOV    ESI,ZFj
         MOV    EBP,ZFkd          ; EBX = kd
         ADD    ESI,EDI           ; ESI -> X(2d, k, i+d)
         add    EBP,EBP
         call   XfftS
;
; X(d, k+e, i) = X(2d, k, i) - G^kd X(2d, k, i+d)
         MOV    EAX,0[EDI]
         MOV    EBX,4[EDI]
         SUB    EAX,ZS+0
         MOV    ECX,8[EDI]
         SBB    EBX,ZS+4
         MOV    0[ESI],EAX
         SBB    ECX,ZS+8
         MOV    EAX,12[EDI]
         MOV    4[ESI],EBX
         SBB    EAX,ZS+12
         MOV    EBX,16[EDI]
         MOV    8[ESI],ECX
         SBB    EBX,ZS+16
         MOV    12[ESI],EAX
         MOV    16[ESI],EBX
;
; X(d, k, i) = X(2d, k, i) + G^kd X(2d, k, i+d)
         MOV    EAX,ZS+0
         MOV    EBX,ZS+4
         ADD    0[EDI],EAX
         MOV    ECX,ZS+8
         ADC    4[EDI],EBX
         MOV    EAX,ZS+12
         ADC    8[EDI],ECX
         MOV    EBX,ZS+16
         ADC    12[EDI],EAX
         ADC    16[EDI],EBX
; Next i
         ADD    EDI,LnF
         DEC    ZFi
         JNZ    XfftAd
; Next k
         MOV    EAX,ZFk
         MOV    EBX,ZFd
         INC    EAX
         ADD    ZFkd,EBX        ; kd = kd + d
         MOV    ZFk,EAX         ; k  = k + 1
         CMP    EAX,ZFe
         JL     XfftAb
; Next stage
         SHL    ZFe,1
         SHR    ZFj,1
         SHR    ZFd,1
         JNZ    XfftA
         RETN
;
; Multiply X(i) by H^s
;
; ESI -> X(i), EBP  = s
;
; ZS   = H^s * X(i)
;
; EDI unchanged
;
XfftS:   mov    ZFs,EBP
         mov    EDX,16[ESI]
         and    EBP,60
         mov    EAX,0          ; EAX = 0
         mov    EBP,XfftSj[EBP]
         sar    EDX,31         ; EDX = sign of X(i)
         JMP    EBP
;
         ALIGN  4
XfftSj   DD     XfftS00
         DD     XfftS01
         DD     XfftS02
         DD     XfftS03
         DD     XfftS04
         DD     XfftS05
         DD     XfftS06
         DD     XfftS07
         DD     XfftS08
         DD     XfftS09
         DD     XfftS10
         DD     XfftS11
         DD     XfftS12
         DD     XfftS13
         DD     XfftS14
         DD     XfftS15
;
; S = H^(s-[s/4]*4) * S
XfftSs:  test   ZFs,3
         jz     XfftSv
         MOV    ECX,ZFs
         MOV    EAX,ZS+16
         AND    CL,3
         MOV    EDX,ZS+12
         ADD    CL,CL
         SHLD   EAX,EDX,CL
         MOV    EBX,ZS+8
         MOV    ZS+16,EAX
         SHLD   EDX,EBX,CL
         MOV    EAX,ZS+4
         MOV    ZS+12,EDX
         SHLD   EBX,EAX,CL
         MOV    EDX,ZS+0
         MOV    ZS+8,EBX
         SHLD   EAX,EDX,CL
         SHL    EDX,CL
         MOV    ZS+4,EAX
         MOV    ZS+0,EDX
;
; S = -S
XfftSv:  test   ZFs,64
         jz     XfftSx
         not    ZS+16
         not    ZS+12
         not    ZS+8
         not    ZS+4
         neg    ZS+0
         jc     XfftSx
         add    ZS+4,1
         adc    ZS+8,0
         adc    ZS+12,0
         adc    ZS+16,0
XfftSx:  retn   
;
; S = X(i),  [s/4] = 0
XfftS00: MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         MOV    ECX,8[ESI]
         MOV    ZS+0,EAX
         MOV    ZS+4,EBX
         MOV    ZS+8,ECX
         MOV    EAX,12[ESI]
         MOV    EBX,16[ESI]
         MOV    ZS+12,EAX
         MOV    ZS+16,EBX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^4*X(i),  [s/4] = 1
XfftS01: MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,16[ESI]
         MOV    ZS-3,EAX
         SBB    ECX,EDX
         MOV    EAX,8[ESI]
         MOV    ZS+1,EBX
         SBB    EAX,EDX
         MOV    ZS+5,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+9,EAX
         SBB    ECX,EDX
         MOV    ZS+13,EBP
         MOV    ZS+17,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^8*X(i),  [s/4] = 2
XfftS02: MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,16[ESI]
         MOV    ZS-2,EAX
         SBB    ECX,EDX
         MOV    EAX,8[ESI]
         MOV    ZS+2,EBX
         SBB    EAX,EDX
         MOV    ZS+6,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+10,EAX
         SBB    ECX,EDX
         MOV    ZS+14,EBP
         MOV    ZS+18,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^12*X(i),  [s/4] = 3
XfftS03: MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,16[ESI]
         MOV    ZS-1,EAX
         SBB    ECX,EDX
         MOV    EAX,8[ESI]
         MOV    ZS+3,EBX
         SBB    EAX,EDX
         MOV    ZS+7,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+11,EAX
         SBB    ECX,EDX
         MOV    ZS+15,EBP
         MOV    ZS+19,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^16*X(i),  [s/4] = 4
XfftS04: MOV    EBX,0[ESI]
         SUB    EAX,12[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,16[ESI]
         MOV    ZS+0,EAX
         SBB    ECX,EDX
         MOV    EAX,8[ESI]
         MOV    ZS+4,EBX
         SBB    EAX,EDX
         MOV    EBX,0
         MOV    ZS+8,ECX
         SBB    EBX,EDX
         MOV    ZS+12,EAX
         MOV    ZS+16,EBX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^20*X(i),  [s/4] = 5
XfftS05: MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,12[ESI]
         MOV    ZS-3,EAX
         SBB    ECX,16[ESI]
         MOV    EAX,4[ESI]
         MOV    ZS+1,EBX
         SBB    EAX,EDX
         MOV    ZS+5,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+9,EAX
         SBB    ECX,EDX
         MOV    ZS+13,EBP
         MOV    ZS+17,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^24*X(i),  [s/4] = 6
XfftS06: MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,12[ESI]
         MOV    ZS-2,EAX
         SBB    ECX,16[ESI]
         MOV    EAX,4[ESI]
         MOV    ZS+2,EBX
         SBB    EAX,EDX
         MOV    ZS+6,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+10,EAX
         SBB    ECX,EDX
         MOV    ZS+14,EBP
         MOV    ZS+18,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^28*X(i),  [s/4] = 7
XfftS07: MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,12[ESI]
         MOV    ZS-1,EAX
         SBB    ECX,16[ESI]
         MOV    EAX,4[ESI]
         MOV    ZS+3,EBX
         SBB    EAX,EDX
         MOV    ZS+7,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+11,EAX
         SBB    ECX,EDX
         MOV    ZS+15,EBP
         MOV    ZS+19,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^32*X(i),  [s/4] = 8
XfftS08: MOV    EBX,0
         SUB    EAX,8[ESI]
         MOV    ECX,0[ESI]
         SBB    EBX,12[ESI]
         MOV    ZS+0,EAX
         SBB    ECX,16[ESI]
         MOV    EAX,4[ESI]
         MOV    ZS+4,EBX
         SBB    EAX,EDX
         MOV    EBX,0
         MOV    ZS+8,ECX
         SBB    EBX,EDX
         MOV    ZS+12,EAX
         MOV    ZS+16,EBX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^36*X(i),  [s/4] = 9
XfftS09: MOV    ECX,4[ESI]
         MOV    EBP,4[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZS-3,EAX
         SBB    ECX,12[ESI]
         MOV    EAX,0[ESI]
         MOV    ZS+1,EBX
         SBB    EAX,16[ESI]
         MOV    ZS+5,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+9,EAX
         SBB    ECX,EDX
         MOV    ZS+13,EBP
         MOV    ZS+17,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^40*X(i),  [s/4] = 10
XfftS10: MOV    ECX,4[ESI]
         MOV    EBP,4[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZS-2,EAX
         SBB    ECX,12[ESI]
         MOV    EAX,0[ESI]
         MOV    ZS+2,EBX
         SBB    EAX,16[ESI]
         MOV    ZS+6,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+10,EAX
         SBB    ECX,EDX
         MOV    ZS+14,EBP
         MOV    ZS+18,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^44*X(i),  [s/4] = 11
XfftS11: MOV    ECX,4[ESI]
         MOV    EBP,4[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZS-1,EAX
         SBB    ECX,12[ESI]
         MOV    EAX,0[ESI]
         MOV    ZS+3,EBX
         SBB    EAX,16[ESI]
         MOV    ZS+7,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZS+11,EAX
         SBB    ECX,EDX
         MOV    ZS+15,EBP
         MOV    ZS+19,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^48*X(i),  [s/4] = 12
XfftS12: MOV    EBX,0
         SUB    EAX,4[ESI]
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZS+0,EAX
         SBB    ECX,12[ESI]
         MOV    EAX,0[ESI]
         MOV    ZS+4,EBX
         SBB    EAX,16[ESI]
         MOV    EBX,0
         MOV    ZS+8,ECX
         SBB    EBX,EDX
         MOV    ZS+12,EAX
         MOV    ZS+16,EBX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^52*X(i),  [s/4] = 13
XfftS13: MOV    ECX,0[ESI]
         MOV    EBP,0[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,4[ESI]
         MOV    ZS-3,EAX
         SBB    ECX,8[ESI]
         MOV    EAX,0
         MOV    ZS+1,EBX
         SBB    EAX,12[ESI]
         MOV    ZS+5,ECX
         SBB    EBP,16[ESI]
         MOV    ECX,0
         MOV    ZS+9,EAX
         SBB    ECX,EDX
         MOV    ZS+13,EBP
         MOV    ZS+17,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^56*X(i),  [s/4] = 14
XfftS14: MOV    ECX,0[ESI]
         MOV    EBP,0[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,4[ESI]
         MOV    ZS-2,EAX
         SBB    ECX,8[ESI]
         MOV    EAX,0
         MOV    ZS+2,EBX
         SBB    EAX,12[ESI]
         MOV    ZS+6,ECX
         SBB    EBP,16[ESI]
         MOV    ECX,0
         MOV    ZS+10,EAX
         SBB    ECX,EDX
         MOV    ZS+14,EBP
         MOV    ZS+18,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;
; S = H^60*X(i),  [s/4] = 15
XfftS15: MOV    ECX,0[ESI]
         MOV    EBP,0[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,4[ESI]
         MOV    ZS-1,EAX
         SBB    ECX,8[ESI]
         MOV    EAX,0
         MOV    ZS+3,EBX
         SBB    EAX,12[ESI]
         MOV    ZS+7,ECX
         SBB    EBP,16[ESI]
         MOV    ECX,0
         MOV    ZS+11,EAX
         SBB    ECX,EDX
         MOV    ZS+15,EBP
         MOV    ZS+19,ECX
         TEST   ZFs,64+3
         jnz    XfftSs
         retn   
;====================================================
; Reduce X(i) modulo F
;
; ESI -> X(i), EAX is used
;
; Before:  -2^159 < X(i) < 2^159
; After:   0 <= X(i) <= 2^128
;
; Let X = X(i) = A + 2^128 B
;
; If X >= 0 then X = A - B (mod F)
;           If A - B < 0 then add 2^128 + 1
;
; If X <  0 then X = A + 2^128 B - 2^160
;                  = A + 2^32 - B (mod F)
;
XmodF:   MOV    EAX,16[ESI]
         MOV    dword ptr 16[ESI],0
         CMP    EAX,0
         JE     XmodFx
         JL     XmodFn
; X(i) >= 0
         SUB    0[ESI],EAX
         JNC    XmodFx
         SBB    dword ptr 4[ESI],0
         JNC    XmodFx
         SBB    dword ptr 8[ESI],0
         JNC    XmodFx
         SBB    dword ptr 12[ESI],0
         JNC    XmodFx
         ADC    dword ptr 0[ESI],0
         JNC    XmodFx
         ADC    dword ptr 4[ESI],0
         JNC    XmodFx
XmodFg:  ADC    dword ptr 8[ESI],0
         JNC    XmodFx
         ADC    dword ptr 12[ESI],0
         JNC    XmodFx
; Must be  a + 2^128,  a < 2^32 
         ADC    dword ptr 16[ESI],0
         TEST   dword ptr 0[ESI],0FFFFFFFFh
         JZ     XmodFx
         DEC    dword ptr 0[ESI]
         MOV    dword ptr 16[ESI],0
         JMP    XmodFx
; X(i) < 0
XmodFn:  SUB    0[ESI],EAX
         JC     XmodFx
         ADD    dword ptr 4[ESI],1
         JC     XmodFg
XmodFx:  retn
;====================================================
;
; X(i) = X(D-b(i))^2 (mod F)
;
XfftQ:   mov    ZQc,4*(64+5)
XfftQb:  mov    EBX,ZQc
         mov    EDI,Tq[EBX]
         mov    ESI,Tq-4[EBX]
         CMP    EDI,Lk4
         lea    EDI,ZX[EDI]
         lea    ESI,ZX[ESI]
         JE     XfftQs
         call   XmodF
         TEST   BYTE PTR[16+ESI],1
         JNZ    XfftQu
;-1  0  3 
  MOV  EAX,0[ESI]
  MOV  EDX,12[ESI]
  MOV  ECX,0
  MOV  EBX,0
  MUL  EDX
  MOV  EBP,EAX
  SUB  ECX,EDX
  SBB  EBX,0
  ADD  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
;-1  1  2 
  MOV  EAX,4[ESI]
  MOV  EDX,8[ESI]
  MUL  EDX
  ADD  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  ADD  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  MOV  ZS,EBP
; 0  0  0 
  MOV  EAX,0[ESI]
  MOV  EBP,EBX
  SAR  EBP,31
  MUL  EAX
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
; 0  3  1 
  MOV  EAX,12[ESI]
  MOV  EDX,4[ESI]
  MUL  EDX
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
; 0  2  2 
  MOV  EAX,8[ESI]
  MUL  EAX
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  MOV  0[EDI],ECX
; 1  0  1 
  MOV  EAX,0[ESI]
  MOV  EDX,4[ESI]
  MOV  ECX,EBP
  SAR  ECX,31
  MUL  EDX
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
; 1  3  2 
  MOV  EAX,12[ESI]
  MOV  EDX,8[ESI]
  MUL  EDX
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
  MOV  4[EDI],EBX
; 2  0  2 
  MOV  EAX,0[ESI]
  MOV  EDX,8[ESI]
  MOV  EBX,ECX
  SAR  EBX,31
  ADD  ECX,ZS
  ADC  EBX,0
  MUL  EDX
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
; 2  1  1 
  MOV  EAX,4[ESI]
  MUL  EAX
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
; 2  3  3 
  MOV  EAX,12[ESI]
  MUL  EAX
  SUB  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  MOV  8[EDI],EBP
  MOV  12[EDI],ECX
  MOV  16[EDI],EBX
; 486 clock ticks: 214 
;
XfftQk:  SUB    ZQc,4
         JNZ    XfftQb
         RETN
;
; Start of cycle
XfftQs:  MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         MOV    ECX,8[ESI]
         MOV    0[EDI],EAX
         MOV    4[EDI],EBX
         MOV    8[EDI],ECX
         MOV    EAX,12[ESI]
         MOV    EBX,16[ESI]
         MOV    12[EDI],EAX
         MOV    16[EDI],EBX
         JMP    XfftQk
;
; (2^256)^2 = 1 (mod F)
XfftQu:  MOV    dword ptr 0[EDI],1
         MOV    dword ptr 4[EDI],0
         MOV    dword ptr 8[EDI],0
         MOV    dword ptr 12[EDI],0
         MOV    dword ptr 16[EDI],0
         JMP    XfftQk
;====================================================

_AR2F76A endp
_TEXT    ends
         end
