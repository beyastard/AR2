;
; AR2F77A 2.1.0m  TONY FORBES  December 1994
;
; a = a^2  (using FFT modulo F_7, dimension 2^7)
;
; xsqufft77 (&a, d, &f);
;
; Registers:   EAX = &a, EDX = d = d(a), 
;              EBX = &f, a 645-digit work area
; Assumptions: 0 <= a < 2^3840, d(a) > 0 
;              a has sufficient room for 242 digits
;
; Finite Fourier Transform for a => a^2
;
; F = the Fourier base, the Fermat number F_7 = 2^128 + 1
; D = 128, the dimension of the Fourier transform
; G = 4, a primitive 128th root of unity modulo F
; M = 2^60, the base for representation of a
;
; F[X](k) = Sum{i=0 to D-1: X(i)G^ki} (mod F)
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
_DATA    segment dword public 'DATA'
         public _AR2XF77data
_AR2XF77data dd 0
         org    _AR2XF77data
Zcx      dd     0        ; Save ECX
Zbp      dd     0        ; Save EBP
Zsi      dd     0        ; Save ESI
Zdi      dd     0        ; Save EDI
ZYptr    dd     0        ; EAX = &a
ZXptr    dd     0        ; EBX = &f
;
LnF      =      5*4      ; length of F
LnM      =      7        ; length of M
Lyh      =      16*15    ; offset to high order y
Lk0      =      0        ; offset to 64-block 0
Lk1      =      32*5*4   ; offset to 64-block 1
Lk2      =      64*5*4   ; offset to 64-block 2
Lk3      =      96*5*4   ; offset to 64-block 3
Lk4      =      128*5*4  ; offset to end of FFT array
;
ZU       DD     5 dup (0)
ZWm1     dd     0
ZW       DD     6 dup (0)
;
ZFd      DD     0
ZFe      DD     0
ZFk      DD     0
ZFkd     DD     0
ZFi      DD     0
ZFj      DD     0
ZFX      DD     0
ZSc      DD     0
ZMa      DD     0
ZMb      DD     0
ZMs      DD     0
;
; Bit reversal table, i => LnF * b(i), 
; where b(i) = i bit-reversed modulo 128
Tb  DW   0,1280,640,1920,320,1600,960,2240 
    DW   160,1440,800,2080,480,1760,1120,2400 
    DW   80,1360,720,2000,400,1680,1040,2320 
    DW   240,1520,880,2160,560,1840,1200,2480 
    DW   40,1320,680,1960,360,1640,1000,2280 
    DW   200,1480,840,2120,520,1800,1160,2440 
    DW   120,1400,760,2040,440,1720,1080,2360 
    DW   280,1560,920,2200,600,1880,1240,2520 
    DW   20,1300,660,1940,340,1620,980,2260 
    DW   180,1460,820,2100,500,1780,1140,2420 
    DW   100,1380,740,2020,420,1700,1060,2340 
    DW   260,1540,900,2180,580,1860,1220,2500 
    DW   60,1340,700,1980,380,1660,1020,2300 
    DW   220,1500,860,2140,540,1820,1180,2460 
    DW   140,1420,780,2060,460,1740,1100,2380 
    DW   300,1580,940,2220,620,1900,1260,2540 
;
; Table for controlling X => X^2
;
; X(0) = X(0)^2, 
; X(128-b(k)) = X(k)^2,  k = 1, 2, ..., 127
;
; k => 128-b(k) cycle structure
;
; (0) c=1 
; (1,64,127) c=3 
; (2,96,125,33,62,66,95,3,32,126,65,63) c=12 
; (4,112,121,49,58,82,91,19,28,100,109,37,46,70,79,7,16,124,97,61,34,94,67,31) c=24 
; (5,48,122,81,59,18,92,99,29,36,110,69,47,6,80,123,17,60,98,93,35,30,68,111) c=24 
; (8,120,113,57,50,90,83,27,20,108,101,45,38,78,71,15) c=16 
; (9,56,114,89,51,26,84,107,21,44,102,77,39,14,72,119) c=16 
; (10,88,115,25,52,106,85,43,22,76,103,13,40,118,73,55) c=16 
; (11,24,116,105,53,42,86,75,23,12,104,117,41,54,74,87) c=16 
;
Tq  DW   Lk4
    DW   0,Lk4 ; c=1 
    DW   20,1280,2540,Lk4 ; c=3 
    DW   40,1920,2500,660,1240,1320,1900,60,640,2520,1300,1260,Lk4 ; c=12 
    DW   80,2240,2420,980,1160,1640,1820,380,560,2000,2180,740  
    DW   920,1400,1580,140,320,2480,1940,1220,680,1880,1340,620,Lk4 ; c=24 
    DW   100,960,2440,1620,1180,360,1840,1980,580,720,2200,1380  
    DW   940,120,1600,2460,340,1200,1960,1860,700,600,1360,2220,Lk4 ; c=24 
    DW   160,2400,2260,1140,1000,1800,1660,540,400,2160,2020,900  
    DW   760,1560,1420,300,Lk4 ; c=16 
    DW   180,1120,2280,1780,1020,520,1680,2140,420,880,2040,1540  
    DW   780,280,1440,2380,Lk4 ; c=16 
    DW   200,1760,2300,500,1040,2120,1700,860,440,1520,2060,260  
    DW   800,2360,1460,1100,Lk4 ; c=16 
    DW   220,480,2320,2100,1060,840,1720,1500,460,240,2080,2340  
    DW   820,1080,1480,1740,Lk4 ; c=16 
;
_DATA   ends
;
_TEXT    segment dword public 'CODE'
         assume  CS:_TEXT
         assume  DS:DGROUP
_AR2F77A proc    near
;========================================
;
; y => y^2 by Finite Fourier Transform
;
         public  xsqufft77_
xsqufft77_:
         mov    Zcx,ECX
         mov    Zbp,EBP
         mov    Zsi,ESI
         mov    Zdi,EDI
         mov    ZYptr,EAX            ; &a
         mov    ZXptr,EBX            ; &f (FFT array)
;
; Clear high order digits, if necessary
         lea    EDI,[EAX+EDX*4]      ; First high order digit
         lea    ESI,480[EAX]         ; Limit
Xfft2:   cmp    EDI,ESI
         jae    Xfft3
         mov    dword ptr [EDI],0
         lea    EDI,4[EDI]
         jmp    Xfft2
Xfft3:
;
; 7-stage Fast Fourier Transform 
         CALL   XfftI                ; d = 64, 32
         MOV    ZFd,16
         MOV    ZFe,4
         MOV    ZFj,16*LnF
         CALL   XfftA
         CALL   XfftR
;
; X^2
         CALL   XfftQ
;
; 7-stage Fast Fourier Transform 
         MOV    ZFd,64
         MOV    ZFe,1
         MOV    ZFj,64*LnF
         CALL   XfftA
         CALL   XfftS
;
; y = X/128
; X(0)/128 => y
; X(i)     => y,  i = 1, 2, ..., 254
; X(127) is always zero
;
;                     0000ddddccccbbbbaaaa  MOV
;                      ccccbbbbaaaa         ADD ADC ADC
;              0000dddd                     ADC 0, MOV 0
;               ccccbbbbaaaa                ADD ADC ADC
;       0000dddd                            ADC 0, MOV 0
;
         mov    ESI,ZXptr
         mov    EDI,ZYptr
         MOV    EAX,0[ESI]
         MOV    EDX,4[ESI]
         SHRD   EAX,EDX,7
         MOV    0[EDI],EAX
         MOV    EAX,8[ESI]
         SHRD   EDX,EAX,7
         MOV    4[EDI],EDX
         MOV    EDX,12[ESI]
         SHRD   EAX,EDX,7
         MOV    8[EDI],EAX
         MOV    EAX,16[ESI]
         SHRD   EDX,EAX,7
         MOV    12[EDI],EDX
         MOV    dword ptr 16[EDI],0
;
         MOV    EBP,2
         add    EDI,LnM
         MOV    ZMs,1
Xfft6:   MOV    SI,Tb[EBP]
         AND    ESI,0FFFFh
         add    ESI,ZXptr
         TEST   ZMs,1
         JZ     Xfft6b
         MOV    EAX,0[ESI]
         AND    EAX,0FFFFFF80h
         MOV    EDX,4[ESI]
         SHRD   EAX,EDX,3
         MOV    ZU+0,EAX
         MOV    EAX,8[ESI]
         SHRD   EDX,EAX,3
         MOV    ZU+4,EDX
         MOV    EDX,12[ESI]
         SHRD   EAX,EDX,3
         MOV    ZU+8,EAX
         MOV    ECX,16[ESI]
         SHRD   EDX,ECX,3
         MOV    ZU+12,EDX
         SHR    ECX,3
         JMP    Xfft6d
Xfft6b:  MOV    EAX,0[ESI]
         MOV    EDX,4[ESI]
         SHRD   EAX,EDX,7
         MOV    ZU+0,EAX
         MOV    EAX,8[ESI]
         SHRD   EDX,EAX,7
         MOV    ZU+4,EDX
         MOV    EDX,12[ESI]
         SHRD   EAX,EDX,7
         MOV    ZU+8,EAX
         MOV    EAX,16[ESI]
         SHRD   EDX,EAX,7
         XOR    ECX,ECX
         MOV    ZU+12,EDX
Xfft6d:  MOV    EAX,ZU+0
         ADD    0[EDI],EAX
         MOV    EAX,ZU+4
         ADC    4[EDI],EAX
         MOV    EAX,ZU+8
         ADC    8[EDI],EAX
         MOV    EAX,ZU+12
         ADC    EAX,0
         MOV    12[EDI],EAX
         ADC    ECX,0
         MOV    16[EDI],ECX
         ADD    EBP,2
         ADD    EDI,LnM
         ADD    EDI,ZMs
         XOR    ZMs,1
         CMP    EBP,2*126
         JLE    Xfft6
;
         mov    EDI,Zdi
         mov    ESI,Zsi
         mov    EBP,Zbp
         mov    ECX,Zcx
         retn   
;========================================
; FFT special first step: y => X, d = 32
;
; X(32,0,i) = y(i) +      y(i+32)
; X(32,1,i) = y(i) + G^32 y(i+32)
; X(32,2,i) = y(i) -      y(i+32)
; X(32,3,i) = y(i) - G^32 y(i+32)
;
; i = 0, 1, ..., 31
;
; 0 <= y(i) < 2^60
;
XfftI:   mov    ESI,ZYptr               ; ESI -> y
         mov    EDI,ZXptr
         add    ESI,31*LnM+15
         add    EDI,31*LnF
         MOV    ZMs,1
;
; Sort out y words
XfftId:  MOV    EAX,0[ESI]
         MOV    ECX,4[ESI]
         MOV    EBX,0+Lyh[ESI]
         MOV    EDX,4+Lyh[ESI]
         TEST   ZMs,1
         JZ     XfftIb                  ; Byte boundary
         SHRD   EAX,ECX,4               ; Half byte
         SHR    ECX,4
         SHRD   EBX,EDX,4
         SHR    EDX,4
XfftIb:  MOV    ZMa,EAX                 ; EAX = ZMa = y0(i)
         AND    ECX,0FFFFFFFh           ; ECX = y1(i)
         MOV    ZMb,EBX                 ; EBX = ZMb = y0(i+d)
         AND    EDX,0FFFFFFFh           ; EDX = y1(i+d)
         XOR    ZMs,1
         SUB    ESI,ZMs
         MOV    EBP,0                   ; Zero
;
; X(32,2,i) = y(i) - y(i+32)  [at offset Lk1]
; X(32,3,i) = y(i)
         MOV    EBX,ECX                 ; EBX = y1(i)
         MOV    0+Lk3[EDI],EAX          ; y0(i)
         SUB    EAX,ZMb                 ; EAX = y0(i) - y0(i+d)
         MOV    0+Lk1[EDI],EAX          ; y0(i) - y0(i+d)
         MOV    4+Lk3[EDI],ECX          ; y1(i)
         SBB    EBX,EDX
         MOV    4+Lk1[EDI],EBX          ; y1(i) - y1(i+d)
         MOV    EAX,EBP
         SBB    EAX,EBP                 ; EAX = Carry
         MOV    8+Lk1[EDI],EAX          ; Carry
         MOV    12+Lk1[EDI],EAX         ; Carry
         MOV    16+Lk1[EDI],EAX         ; Carry
;
; X(32,0,i) = y(i) +      y(i+32)
; X(32,1,i) = y(i) + G^32 y(i+32)  [at offset Lk2]
         MOV    EAX,ZMa                 ; EAX = y0(i)
         MOV    EBX,ZMb                 ; EBX = y0(i+d)
         MOV    0+Lk2[EDI],EAX          ; y0(i)
         MOV    8+Lk2[EDI],EBX          ; y0(i+d)
         ADD    EAX,EBX                 ; EAX = y0(i) + y0(i+d)
         MOV    8+Lk0[EDI],EBP
         MOV    0+Lk0[EDI],EAX          ; y0(i) + y0(i+d)
         MOV    4+Lk2[EDI],ECX          ; y1(i) 
         MOV    12+Lk2[EDI],EDX         ; y1(i+d)
         ADC    ECX,EDX                 ; ECX = y1(i) + y1(i+d)
         MOV    12+Lk0[EDI],EBP
         MOV    4+Lk0[EDI],ECX          ; y1(i) + y1(i+d)
         MOV    16+Lk0[EDI],EBP
         MOV    16+Lk2[EDI],EBP
;
; Subtract  G^32 y(i+32)  from  X(32,3,i)
         MOV    EAX,EBP
         MOV    EBX,EBP
         SUB    EAX,ZMb
         MOV    8+Lk3[EDI],EAX
         SBB    EBX,EDX
         MOV    12+Lk3[EDI],EBX
         SBB    EBP,EBP
         MOV    16+Lk3[EDI],EBP
; Next i
         mov    EAX,ZXptr
         SUB    EDI,LnF
         SUB    ESI,LnM  
         cmp    EDI,EAX
         JNL    XfftId
         RETN
;====================================================
; FFT general step
;
; X(d,k,i)   = X(2d,k,i) + G^(kd) X(2d,k,i+d)
; X(d,k+e,i) = X(2d,k,i) - G^(kd) X(2d,k,i+d)
;
; k = 0, ..., e-1,  i = 0, ..., d-1
;
; ZFd = d, ZFe = e, ZFj = offset to X(2d,k,i+d)
; ZFk = k,          ZFi = counter for i
;
         ALIGN  4
XfftAj   DD     XfftA00
         DD     XfftA01
         DD     XfftA02
         DD     XfftA03
         DD     XfftA04
         DD     XfftA05
         DD     XfftA06
         DD     XfftA07
         DD     XfftA08
         DD     XfftA09
         DD     XfftA10
         DD     XfftA11
         DD     XfftA12
         DD     XfftA13
         DD     XfftA14
         DD     XfftA15
XfftA:   MOV    ZFk,0          ; k
         MOV    ZFkd,0         ; kd
XfftAb:  MOV    EBX,ZFk        ; EBX = k
         MOV    EBP,ZFkd       ; EBX = kd
         MOV    DI,Tb[EBX*2]   ;  DI = LnF * (k bit-reversed)
         SHR    EBP,2          ; EBP = [kd/4]
         AND    EDI,0FFFFh
         MOV    EAX,ZFd        ; d
         add    EDI,ZXptr      ; EDI -> X(2d,k,i)
         MOV    EBP,XfftAj[EBP*4]
         MOV    ZFi,EAX        ; counter for i
         MOV    ZFX,EBP        ; EBP = code offset for [kd/4]
;
XfftAd:  MOV    ESI,ZFj
         ADD    ESI,EDI        ; ESI -> X(2d,k,i+d)
         MOV    EBP,ZFX
         MOV    EDX,16[ESI]
         MOV    EAX,0          ; EAX = 0
         SAR    EDX,31         ; EDX = sign
         JMP    EBP
;
; X(d,k+e,i) = X(2d,k,i) - G^kd X(2d,k,i+d)
XfftAsub:
         MOV    EAX,0[EDI]
         MOV    EBX,4[EDI]
         SUB    EAX,ZW+0
         MOV    ECX,8[EDI]
         SBB    EBX,ZW+4
         MOV    0[ESI],EAX
         SBB    ECX,ZW+8
         MOV    EAX,12[EDI]
         MOV    4[ESI],EBX
         SBB    EAX,ZW+12
         MOV    EBX,16[EDI]
         MOV    8[ESI],ECX
         SBB    EBX,ZW+16
         MOV    12[ESI],EAX
         MOV    16[ESI],EBX
;
; X(d,k,i) = X(2d,k,i) + G^kd X(2d,k,i+d)
         MOV    EAX,ZW+0
         MOV    EBX,ZW+4
         ADD    0[EDI],EAX
         MOV    ECX,ZW+8
         ADC    4[EDI],EBX
         MOV    EAX,ZW+12
         ADC    8[EDI],ECX
         MOV    EBX,ZW+16
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
; W = X(2d,k,i+d),  [kd/4] = 0
XfftA00:
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         MOV    ECX,8[ESI]
         MOV    ZW+0,EAX
         MOV    ZW+4,EBX
         MOV    ZW+8,ECX
         MOV    EAX,12[ESI]
         MOV    EBX,16[ESI]
         MOV    ZW+12,EAX
         MOV    ZW+16,EBX
         TEST   ZFkd,3
         JZ     XfftAsub
;
; W = G^(kd-[kd/4]*4) W = G^kd X(2d,k,i+d)
XfftAshl:
         MOV    ECX,ZFkd
         MOV    EAX,ZW+16
         AND    CL,3
         MOV    EDX,ZW+12
         ADD    CL,CL
         SHLD   EAX,EDX,CL
         MOV    EBX,ZW+8
         MOV    ZW+16,EAX
         SHLD   EDX,EBX,CL
         MOV    EAX,ZW+4
         MOV    ZW+12,EDX
         SHLD   EBX,EAX,CL
         MOV    EDX,ZW+0
         MOV    ZW+8,EBX
         SHLD   EAX,EDX,CL
         SHL    EDX,CL
         MOV    ZW+4,EAX
         MOV    ZW+0,EDX
         JMP    XfftAsub
;
; W = G^4 X(2d,k,i+d),  [kd/4] = 1
XfftA01:
         MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,16[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,EDX
         MOV    EAX,8[ESI]
         MOV    ZW+1,EBX
         SBB    EAX,EDX
         MOV    ZW+5,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+9,EAX
         SBB    ECX,EDX
         MOV    ZW+13,EBP
         MOV    ZW+17,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^8 X(2d,k,i+d),  [kd/4] = 2
XfftA02: 
         MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,16[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,EDX
         MOV    EAX,8[ESI]
         MOV    ZW+2,EBX
         SBB    EAX,EDX
         MOV    ZW+6,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+10,EAX
         SBB    ECX,EDX
         MOV    ZW+14,EBP
         MOV    ZW+18,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^12 X(2d,k,i+d),  [kd/4] = 3
XfftA03: 
         MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,16[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,EDX
         MOV    EAX,8[ESI]
         MOV    ZW+3,EBX
         SBB    EAX,EDX
         MOV    ZW+7,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+11,EAX
         SBB    ECX,EDX
         MOV    ZW+15,EBP
         MOV    ZW+19,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^16 X(2d,k,i+d),  [kd/4] = 4
XfftA04:
         MOV    EBX,0[ESI]
         SUB    EAX,12[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,16[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,EDX
         MOV    EAX,8[ESI]
         MOV    ZW+4,EBX
         SBB    EAX,EDX
         MOV    EBX,0
         MOV    ZW+8,ECX
         SBB    EBX,EDX
         MOV    ZW+12,EAX
         MOV    ZW+16,EBX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^20 X(2d,k,i+d),  [kd/4] = 5
XfftA05:
         MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,12[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,16[ESI]
         MOV    EAX,4[ESI]
         MOV    ZW+1,EBX
         SBB    EAX,EDX
         MOV    ZW+5,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+9,EAX
         SBB    ECX,EDX
         MOV    ZW+13,EBP
         MOV    ZW+17,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^24 X(2d,k,i+d),  [kd/4] = 6
XfftA06: 
         MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,12[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,16[ESI]
         MOV    EAX,4[ESI]
         MOV    ZW+2,EBX
         SBB    EAX,EDX
         MOV    ZW+6,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+10,EAX
         SBB    ECX,EDX
         MOV    ZW+14,EBP
         MOV    ZW+18,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^28 X(2d,k,i+d),  [kd/4] = 7
XfftA07: 
         MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,12[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,16[ESI]
         MOV    EAX,4[ESI]
         MOV    ZW+3,EBX
         SBB    EAX,EDX
         MOV    ZW+7,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+11,EAX
         SBB    ECX,EDX
         MOV    ZW+15,EBP
         MOV    ZW+19,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^32 X(2d,k,i+d),  [kd/4] = 8
XfftA08:
         MOV    EBX,0
         SUB    EAX,8[ESI]
         MOV    ECX,0[ESI]
         SBB    EBX,12[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,16[ESI]
         MOV    EAX,4[ESI]
         MOV    ZW+4,EBX
         SBB    EAX,EDX
         MOV    EBX,0
         MOV    ZW+8,ECX
         SBB    EBX,EDX
         MOV    ZW+12,EAX
         MOV    ZW+16,EBX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^36 X(2d,k,i+d),  [kd/4] = 9
XfftA09:
         MOV    ECX,4[ESI]
         MOV    EBP,4[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,12[ESI]
         MOV    EAX,0[ESI]
         MOV    ZW+1,EBX
         SBB    EAX,16[ESI]
         MOV    ZW+5,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+9,EAX
         SBB    ECX,EDX
         MOV    ZW+13,EBP
         MOV    ZW+17,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^40 X(2d,k,i+d),  [kd/4] = 10
XfftA10: 
         MOV    ECX,4[ESI]
         MOV    EBP,4[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,12[ESI]
         MOV    EAX,0[ESI]
         MOV    ZW+2,EBX
         SBB    EAX,16[ESI]
         MOV    ZW+6,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+10,EAX
         SBB    ECX,EDX
         MOV    ZW+14,EBP
         MOV    ZW+18,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^44 X(2d,k,i+d),  [kd/4] = 11
XfftA11: 
         MOV    ECX,4[ESI]
         MOV    EBP,4[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,12[ESI]
         MOV    EAX,0[ESI]
         MOV    ZW+3,EBX
         SBB    EAX,16[ESI]
         MOV    ZW+7,ECX
         SBB    EBP,EDX
         MOV    ECX,0
         MOV    ZW+11,EAX
         SBB    ECX,EDX
         MOV    ZW+15,EBP
         MOV    ZW+19,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^48 X(2d,k,i+d),  [kd/4] = 12
XfftA12:
         MOV    EBX,0
         SUB    EAX,4[ESI]
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,12[ESI]
         MOV    EAX,0[ESI]
         MOV    ZW+4,EBX
         SBB    EAX,16[ESI]
         MOV    EBX,0
         MOV    ZW+8,ECX
         SBB    EBX,EDX
         MOV    ZW+12,EAX
         MOV    ZW+16,EBX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^52 X(2d,k,i+d),  [kd/4] = 13
XfftA13:
         MOV    ECX,0[ESI]
         MOV    EBP,0[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,4[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,8[ESI]
         MOV    EAX,0
         MOV    ZW+1,EBX
         SBB    EAX,12[ESI]
         MOV    ZW+5,ECX
         SBB    EBP,16[ESI]
         MOV    ECX,0
         MOV    ZW+9,EAX
         SBB    ECX,EDX
         MOV    ZW+13,EBP
         MOV    ZW+17,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^56 X(2d,k,i+d),  [kd/4] = 14
XfftA14: 
         MOV    ECX,0[ESI]
         MOV    EBP,0[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,4[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,8[ESI]
         MOV    EAX,0
         MOV    ZW+2,EBX
         SBB    EAX,12[ESI]
         MOV    ZW+6,ECX
         SBB    EBP,16[ESI]
         MOV    ECX,0
         MOV    ZW+10,EAX
         SBB    ECX,EDX
         MOV    ZW+14,EBP
         MOV    ZW+18,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^60 X(2d,k,i+d),  [kd/4] = 15
XfftA15: 
         MOV    ECX,0[ESI]
         MOV    EBP,0[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,4[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,8[ESI]
         MOV    EAX,0
         MOV    ZW+3,EBX
         SBB    EAX,12[ESI]
         MOV    ZW+7,ECX
         SBB    EBP,16[ESI]
         MOV    ECX,0
         MOV    ZW+11,EAX
         SBB    ECX,EDX
         MOV    ZW+15,EBP
         MOV    ZW+19,ECX
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;====================================================
; Reduce X(i) modulo F, i = 0, 1, ..., 127
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
XfftR:   mov    EDI,ZXptr
         add    EDI,127*LnF

XfftRb:  MOV    EAX,16[EDI]
         MOV    dword ptr 16[EDI],0
         CMP    EAX,0
         JE     XfftRx
         JL     XfftRn
; X(i) >= 0
         SUB    0[EDI],EAX
         JNC    XfftRx
         SBB    dword ptr 4[EDI],0
         JNC    XfftRx
         SBB    dword ptr 8[EDI],0
         JNC    XfftRx
         SBB    dword ptr 12[EDI],0
         JNC    XfftRx
         ADC    dword ptr 0[EDI],0
         JNC    XfftRx
         ADC    dword ptr 4[EDI],0
         JNC    XfftRx
XfftRg:  ADC    dword ptr 8[EDI],0
         JNC    XfftRx
         ADC    dword ptr 12[EDI],0
         JNC    XfftRx
; Must be  a + 2^128,  a < 2^32 
         ADC    dword ptr 16[EDI],0
         TEST   dword ptr 0[EDI],0FFFFFFFFh
         JZ     XfftRx
         DEC    dword ptr 0[EDI]
         MOV    dword ptr 16[EDI],0
         JMP    XfftRx
; X(i) < 0
XfftRn:  SUB    0[EDI],EAX
         JC     XfftRx
         ADD    dword ptr 4[EDI],1
         JC     XfftRg
;
XfftRx:  MOV    EAX,ZXptr
         SUB    EDI,LnF
         cmp    EDI,EAX
         JNL    XfftRb
         RETN
;====================================================
; Determine X(i) from X(i) modulo F and D, 
; i = 0, 1, ..., 126
;
; Before:  -2^159 < X(i) < 2^159
; After:   0 <= X(i) <= 2^133, 128|X(i)  
;
; As XfftR except that we must add  b(2^128 + 1) 
; if  X(i) = -b (mod 128)
;
XfftS:   mov    EDI,ZXptr
         add    EDI,126*LnF
XfftSb:  MOV    EAX,16[EDI]
         MOV    dword ptr 16[EDI],0
         CMP    EAX,0
         JE     XfftSt
         JL     XfftSn
; X(i) >= 0
         SUB    0[EDI],EAX
         JNC    XfftSt
         SBB    dword ptr 4[EDI],0
         JNC    XfftSt
         SBB    dword ptr 8[EDI],0
         JNC    XfftSt
         SBB    dword ptr 12[EDI],0
         JNC    XfftSt
         ADC    dword ptr 0[EDI],0
         JNC    XfftSt
         ADC    dword ptr 4[EDI],0
         JNC    XfftSt
XfftSg:  ADC    dword ptr 8[EDI],0
         JNC    XfftSt
         ADC    dword ptr 12[EDI],0
         JNC    XfftSt
; Must be  a + 2^128,  a < 2^32 
         ADC    dword ptr 16[EDI],0
         TEST   dword ptr 0[EDI],0FFFFFFFFh
         JZ     XfftSx
         DEC    dword ptr 0[EDI]
         MOV    dword ptr 16[EDI],0
         JMP    XfftSt
; X(i) < 0
XfftSn:  SUB    0[EDI],EAX
         JC     XfftSt
         ADD    dword ptr 4[EDI],1
         JC     XfftSg
;
XfftSt:  MOV    EAX,0[EDI]
         AND    EAX,127
         JZ     XfftSx
         NEG    EAX
         ADD    EAX,128
         ADD    16[EDI],EAX
         ADD    0[EDI],EAX
         JNC    XfftSx
         ADC    dword ptr 4[EDI],0
         JNC    XfftSx
         ADC    dword ptr 8[EDI],0
         JNC    XfftSx
         ADC    dword ptr 12[EDI],0
         JNC    XfftSx
         ADC    dword ptr 16[EDI],0
XfftSx:  MOV    EAX,ZXptr
         SUB    EDI,LnF
         cmp    EDI,EAX
         JNL    XfftSb
         RETN
;====================================================
; X(i) => X(i)^2
;
; 0 <= X(i) <= 2^128
; ZX[ESI] -> X(i),  ZX[EDI] -> X(i)^2
;
XfftQ:   MOV    ZSc,2*(128+9)
XfftQb:  MOV    EBX,ZSc
         MOV    DI,Tq[EBX]
         MOV    SI,Tq-2[EBX]
         mov    EAX,ZXptr
         AND    EDI,0FFFFh
         AND    ESI,0FFFFh
         CMP    EDI,Lk4
         lea    EDI,[EDI+EAX]
         lea    ESI,[ESI+EAX]
         JE     XfftQs
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
  MOV  ZW,EBP
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
  ADD  ECX,ZW
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
XfftQk:  SUB    ZSc,2
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

_AR2F77A endp
_TEXT    ends
         end
