;
; AR2F88A 2.1.0m  TONY FORBES  December 1994
;
; a = a^2  (using FFT modulo F_8, dimension 2^8)
;
; xsqufft88 (&a, d, &f);
;
; Registers:   EAX = &a, EDX = d = d(a),
;              EBX = &f, a 2313-digit work area
; Assumptions: 0 <= a < 2^15360, d(a) > 0 
;              a has sufficient room for 962 digits
;
; Finite Fourier Transform for a => a^2
;
; F = the Fourier base, the Fermat number F_8 = 2^256 + 1
; D = 256, the dimension of the Fourier transform
; G = 4, a primitive 256th root of unity modulo F
; M = 2^120, the base for representation of a
;
; F[X](k) = Sum[i=0 to D-1: X(i)G^ki] (mod F)
;
; Fast Fourier Transform
;
; X(D,k,i)      = X(i)
; X(d,k,i)      = X(2d,k,i) + G^dk X(2d,k,i+d)  (mod F)
; X(d,k+D/2d,i) = X(2d,k,i) - G^dk X(2d,k,i+d)  (mod F)
; F[X](k)       = X(1,k,0)
;
; d = D/2, D/4, ..., 1 
; i = 0, 1, ..., d-1
; k = 0, 1, ..., D/2d - 1
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
         public _AR2F88data
_AR2F88data dd 0
         org    _AR2F88data
Zcx      dd     0        ; Save ECX
Zbp      dd     0        ; Save EBP
Zsi      dd     0        ; Save ESI
Zdi      dd     0        ; Save EDI
ZYptr    dd     0        ; EAX = &a
ZXptr    dd     0        ; EBX = &f
;
LnF      =      9*4      ; length of F
LnM      =      15       ; length of M
Lyh      =      64*15    ; offset to high order y
Lk0      =      0        ; offset to 128-block 0
Lk1      =      64*9*4   ; offset to 128-block 1
Lk2      =      128*9*4  ; offset to 128-block 2
Lk3      =      192*9*4  ; offset to 128-block 3
Lk4      =      256*9*4  ; offset to end of FFT array
;
ZU       DD     17 dup (0)
ZWm1     dd     0
ZW       DD     10 dup (0)
;
ZFd      DD     0
ZFe      DD     0
ZFk      DD     0
ZFkd     DD     0
ZFi      DD     0
ZFj      DD     0
ZFX      DD     0
ZSc      DD     0
;
; Bit reversal table, i => LnF * b(i), 
; where b(i) = i bit-reversed modulo 256
Tb  DW   0,4608,2304,6912,1152,5760,3456,8064 
    DW   576,5184,2880,7488,1728,6336,4032,8640 
    DW   288,4896,2592,7200,1440,6048,3744,8352 
    DW   864,5472,3168,7776,2016,6624,4320,8928 
    DW   144,4752,2448,7056,1296,5904,3600,8208 
    DW   720,5328,3024,7632,1872,6480,4176,8784 
    DW   432,5040,2736,7344,1584,6192,3888,8496 
    DW   1008,5616,3312,7920,2160,6768,4464,9072 
    DW   72,4680,2376,6984,1224,5832,3528,8136 
    DW   648,5256,2952,7560,1800,6408,4104,8712 
    DW   360,4968,2664,7272,1512,6120,3816,8424 
    DW   936,5544,3240,7848,2088,6696,4392,9000 
    DW   216,4824,2520,7128,1368,5976,3672,8280 
    DW   792,5400,3096,7704,1944,6552,4248,8856 
    DW   504,5112,2808,7416,1656,6264,3960,8568 
    DW   1080,5688,3384,7992,2232,6840,4536,9144 
    DW   36,4644,2340,6948,1188,5796,3492,8100 
    DW   612,5220,2916,7524,1764,6372,4068,8676 
    DW   324,4932,2628,7236,1476,6084,3780,8388 
    DW   900,5508,3204,7812,2052,6660,4356,8964 
    DW   180,4788,2484,7092,1332,5940,3636,8244 
    DW   756,5364,3060,7668,1908,6516,4212,8820 
    DW   468,5076,2772,7380,1620,6228,3924,8532 
    DW   1044,5652,3348,7956,2196,6804,4500,9108 
    DW   108,4716,2412,7020,1260,5868,3564,8172 
    DW   684,5292,2988,7596,1836,6444,4140,8748 
    DW   396,5004,2700,7308,1548,6156,3852,8460 
    DW   972,5580,3276,7884,2124,6732,4428,9036 
    DW   252,4860,2556,7164,1404,6012,3708,8316 
    DW   828,5436,3132,7740,1980,6588,4284,8892 
    DW   540,5148,2844,7452,1692,6300,3996,8604 
    DW   1116,5724,3420,8028,2268,6876,4572,9180 
;
; Table for controlling X => X^2
;
; X(0) = X(0)^2, 
; X(256-b(k)) = X(k)^2,  k = 1, 2, ..., 255
;
; k => 256-b(k) cycle structure
;
; (0) c=1 
; (1,128,255) c=3 
; (2,192,253,65,126,130,191,3,64,254,129,127) c=12 
; (4,224,249,97,122,162,187,35,60,196,221,69,94,134,159,7,32,252,193,125,66,190,131,63) c=24 
; (5,96,250,161,123,34,188,195,61,68,222,133,95,6,160,251,33,124,194,189,67,62,132,223) c=24 
; (8,240,241,113,114,178,179,51,52,212,213,85,86,150,151,23,24,232,233,105,106,170,171,43,44,204,205,77,78,142,143,15,16,248,225,121,98,186,163,59,36,220,197,93,70,158,135,31) c=48 
; (9,112,242,177,115,50,180,211,53,84,214,149,87,22,152,231,25,104,234,169,107,42,172,203,45,76,206,141,79,14,144,247,17,120,226,185,99,58,164,219,37,92,198,157,71,30,136,239) c=48 
; (10,176,243,49,116,210,181,83,54,148,215,21,88,230,153,103,26,168,235,41,108,202,173,75,46,140,207,13,80,246,145,119,18,184,227,57,100,218,165,91,38,156,199,29,72,238,137,111) c=48 
; (11,48,244,209,117,82,182,147,55,20,216,229,89,102,154,167,27,40,236,201,109,74,174,139,47,12,208,245,81,118,146,183,19,56,228,217,101,90,166,155,39,28,200,237,73,110,138,175) c=48 
;
Tq  DW   Lk4
    DW   0,Lk4 ; c=1 
    DW   36,4608,9180,Lk4 ; c=3 
    DW   72,6912,9108,2340,4536,4680,6876,108,2304,9144,4644,4572,Lk4 ; c=12 
    DW   144,8064,8964,3492,4392,5832,6732,1260,2160,7056,7956,2484  
    DW   3384,4824,5724,252,1152,9072,6948,4500,2376,6840,4716,2268,Lk4 ; c=24 
    DW   180,3456,9000,5796,4428,1224,6768,7020,2196,2448,7992,4788  
    DW   3420,216,5760,9036,1188,4464,6984,6804,2412,2232,4752,8028,Lk4 ; c=24 
    DW   288,8640,8676,4068,4104,6408,6444,1836,1872,7632,7668,3060  
    DW   3096,5400,5436,828,864,8352,8388,3780,3816,6120,6156,1548  
    DW   1584,7344,7380,2772,2808,5112,5148,540,576,8928,8100,4356  
    DW   3528,6696,5868,2124,1296,7920,7092,3348,2520,5688,4860,1116,Lk4 ; c=48 
    DW   324,4032,8712,6372,4140,1800,6480,7596,1908,3024,7704,5364  
    DW   3132,792,5472,8316,900,3744,8424,6084,3852,1512,6192,7308  
    DW   1620,2736,7416,5076,2844,504,5184,8892,612,4320,8136,6660  
    DW   3564,2088,5904,7884,1332,3312,7128,5652,2556,1080,4896,8604,Lk4 ; c=48 
    DW   360,6336,8748,1764,4176,7560,6516,2988,1944,5328,7740,756  
    DW   3168,8280,5508,3708,936,6048,8460,1476,3888,7272,6228,2700  
    DW   1656,5040,7452,468,2880,8856,5220,4284,648,6624,8172,2052  
    DW   3600,7848,5940,3276,1368,5616,7164,1044,2592,8568,4932,3996,Lk4 ; c=48 
    DW   396,1728,8784,7524,4212,2952,6552,5292,1980,720,7776,8244  
    DW   3204,3672,5544,6012,972,1440,8496,7236,3924,2664,6264,5004  
    DW   1692,432,7488,8820,2916,4248,5256,6588,684,2016,8208,7812  
    DW   3636,3240,5976,5580,1404,1008,7200,8532,2628,3960,4968,6300,Lk4 ; c=48 
;
_DATA   ends
;
_TEXT    segment dword public 'CODE'
         assume  CS:_TEXT
         assume  DS:DGROUP
_AR2F88A proc    near
;========================================
;
; y => y^2 by Finite Fourier Transform
;
         public xsqufft88_
xsqufft88_:
         mov    Zcx,ECX
         mov    Zbp,EBP
         mov    Zsi,ESI
         mov    Zdi,EDI
         mov    ZYptr,EAX            ; &a
         mov    ZXptr,EBX            ; &f (FFT array)
;
; Clear high order digits, if necessary
         lea    EDI,[EAX+EDX*4]      ; First high order digit
         lea    ESI,1920[EAX]        ; Limit
Xfft2:   cmp    EDI,ESI
         jae    Xfft3
         mov    dword ptr [EDI],0
         lea    EDI,4[EDI]
         jmp    Xfft2
Xfft3:
;
; 8-stage Fast Fourier Transform 
         CALL   XfftI      ; d = 128, 64
         MOV    ZFd,32
         MOV    ZFe,4
         MOV    ZFj,32*LnF
         CALL   XfftA
         mov    EDI,ZXptr
         add    EDI,255*LnF
         CALL   XfftR
;
; X^2
         CALL   XfftQ
;
; 8-stage Fast Fourier Transform 
         MOV    ZFd,128
         MOV    ZFe,1
         MOV    ZFj,128*LnF
         CALL   XfftA
         mov    EDI,ZXptr
         add    EDI,254*LnF
         CALL   XfftR
;
; y = X/256
; X(0)/256 => y
; X(i)     => y,  i = 1, 2, ..., 254
; X(255) is always zero
;
; hhhhggggffffeeeeddddccccbbbbaaaa000ihhhhggggffffeeeeddddccccbbbbaaaa
; MOV Y,X+1                       0000ihhhhggggffffeeeeddddccccbbbbaaa
;                                   eeeeddddccccbbbbaaaa    ADD/ADC
;                   000ihhhhggggffff                        ADC 0, MOV
;                    eeeeddddccccbbbbaaaa                   ADD/ADC
;    000ihhhhggggffff                                       ADC 0, MOV
;
         mov    ESI,ZXptr
         mov    EDI,ZYptr
         MOV    EAX,1+0[ESI]
         MOV    EBX,1+4[ESI]
         MOV    ECX,1+8[ESI]
         MOV    0[EDI],EAX
         MOV    4[EDI],EBX
         MOV    8[EDI],ECX
         MOV    EAX,1+12[ESI]
         MOV    EBX,1+16[ESI]
         MOV    ECX,1+20[ESI]
         MOV    12[EDI],EAX
         MOV    16[EDI],EBX
         MOV    20[EDI],ECX
         MOV    EAX,1+24[ESI]
         MOV    EBX,1+28[ESI]
         MOV    24[EDI],EAX
         MOV    28[EDI],EBX
         MOV    dword ptr 32[EDI],0
         MOV    EBP,2
         add    EDI,LnM-1
Xfft6:   MOV    SI,Tb[EBP]
         AND    ESI,0FFFFh
         add    ESI,ZXptr
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         ADD    0[EDI],EAX
         MOV    ECX,8[ESI]
         ADC    4[EDI],EBX
         MOV    EAX,12[ESI]
         ADC    8[EDI],ECX
         MOV    EBX,16[ESI]
         ADC    12[EDI],EAX
         MOV    ECX,20[ESI]
         ADC    16[EDI],EBX
         MOV    EAX,24[ESI]
         ADC    ECX,0
         MOV    EBX,28[ESI]
         MOV    20[EDI],ECX
         ADC    EAX,0
         MOV    ECX,32[ESI]
         ADC    EBX,0
         MOV    24[EDI],EAX
         ADC    ECX,0
         MOV    28[EDI],EBX
         MOV    32[EDI],ECX
         ADD    EBP,2
         ADD    EDI,LnM
         CMP    EBP,2*254
         JLE    Xfft6
;
         mov    EDI,Zdi
         mov    ESI,Zsi
         mov    EBP,Zbp
         mov    ECX,Zcx
         retn   
;========================================
; FFT special first step: y => X, d = 64
;
; X(64,0,i) = y(i) +      y(i+64)
; X(64,1,i) = y(i) + G^64 y(i+64)
; X(64,2,i) = y(i) -      y(i+64)
; X(64,3,i) = y(i) - G^64 y(i+64)
;
; i = 0, 1, ..., 63
;
; 0 <= y(i) < 2^120
;
XfftI:   mov    ESI,ZYptr               ; ESI -> y
         mov    EDI,ZXptr
         add    ESI,63*LnM
         add    EDI,63*LnF
;
; Pick up high order words
XfftId:  MOV    ECX,12[ESI]
         MOV    EDX,12+Lyh[ESI]
         AND    ECX,00FFFFFFh           ; ECX = y3(i)
         AND    EDX,00FFFFFFh           ; EDX = y3(i+d)
;
; X(64,2,i) = y(i) - y(i+64)  [at offset Lk1]
; X(64,3,i) = y(i)
; -2^120 < X(64,2,i) < 2^120
         MOV    EAX,0[ESI]              ; EAX = y0(i)
         MOV    EBX,4[ESI]              ; EBX = y1(i)
         MOV    EBP,8[ESI]              ; EBP = y2(i)
         MOV    0+Lk3[EDI],EAX          ; y0(i)
         MOV    4+Lk3[EDI],EBX          ; y1(i)
         SUB    EAX,0+Lyh[ESI]          ; EAX = y0(i) - y0(i+d)
         MOV    8+Lk3[EDI],EBP          ; y2(i)
         SBB    EBX,4+Lyh[ESI]          ; EBX = y1(i) - y1(i+d)
         MOV    0+Lk1[EDI],EAX          ; y0(i) - y0(i+d)
         SBB    EBP,8+Lyh[ESI]          ; EBP = y2(i) - y2(i+d)
         MOV    4+Lk1[EDI],EBX          ; y1(i) - y1(i+d)
;
         MOV    EAX,ECX                 ; EAX = y3(i)
         MOV    EBX,0
         SBB    EAX,EDX                 ; EAX = y3(i) - y3(i+d)
         MOV    12+Lk3[EDI],ECX         ; y3(i)
         SBB    EBX,0                   ; EBX = carry
         MOV    8+Lk1[EDI],EBP          ; y2(i) - y2(i+d)
         MOV    12+Lk1[EDI],EAX         ; y3(i) - y3(i+d)
         MOV    16+Lk1[EDI],EBX         ; carry
         MOV    20+Lk1[EDI],EBX         ; carry
         MOV    24+Lk1[EDI],EBX         ; carry
         MOV    28+Lk1[EDI],EBX         ; carry
         MOV    32+Lk1[EDI],EBX         ; carry
;
; X(64,0,i) = y(i) +      y(i+64)
; X(64,1,i) = y(i) + G^64 y(i+64)  [at offset Lk2]
; 0 <= X(64,0,i) < 2^121
; 0 <= X(64,1,i) < 2^248
         MOV    EAX,0[ESI]              ; y0(i)
         MOV    EBX,0+Lyh[ESI]          ; y0(i+d)
         MOV    0+Lk2[EDI],EAX          ; y0(i)
         MOV    16+Lk2[EDI],EBX         ; y0(i+d)
         ADD    EAX,EBX
         MOV    dword ptr 16+Lk0[EDI],0
         MOV    0+Lk0[EDI],EAX          ; y0(i) + y0(i+d)
         MOV    dword ptr 20+Lk0[EDI],0
;
         MOV    EAX,4[ESI]              ; y1(i)
         MOV    EBX,4+Lyh[ESI]          ; y1(i+d)
         MOV    4+Lk2[EDI],EAX          ; y1(i)
         MOV    20+Lk2[EDI],EBX         ; y1(i+d)
         ADC    EAX,EBX
         MOV    dword ptr 24+Lk0[EDI],0
         MOV    4+Lk0[EDI],EAX          ; y1(i) + y1(i+d)
         MOV    dword ptr 28+Lk0[EDI],0
;
         MOV    EAX,8[ESI]              ; y2(i)
         MOV    EBX,8+Lyh[ESI]          ; y2(i+d)
         MOV    8+Lk2[EDI],EAX          ; y2(i)
         MOV    24+Lk2[EDI],EBX         ; y2(i+d)
         ADC    EAX,EBX
         MOV    dword ptr 32+Lk0[EDI],0
         MOV    8+Lk0[EDI],EAX          ; y2(i) + y2(i+d)
;
         MOV    12+Lk2[EDI],ECX         ; y3(i)
         MOV    28+Lk2[EDI],EDX         ; y3(i+d)
         ADC    ECX,EDX
         MOV    dword ptr 32+Lk2[EDI],0
         MOV    12+Lk0[EDI],ECX         ; y3(i) + y3(i+d)
;
; Subtract  G^64 y(i+64)  from  X(64,3,i)
; -2^248 < X(64,3,i) < 2^120
         MOV    EAX,0
         MOV    EBX,0
         SUB    EAX,0+Lyh[ESI]          ; EAX = -y0(i+d)
         MOV    EBP,0
         SBB    EBX,4+Lyh[ESI]          ; EBX = -y1(i+d)
         MOV    16+Lk3[EDI],EAX         ; -y0(i+d)
         SBB    EBP,8+Lyh[ESI]          ; EBP = -y2(i+d)
         MOV    20+Lk3[EDI],EBX         ; -y1(i+d)
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,EDX                 ; EAX = -y3(i+d)
         MOV    24+Lk3[EDI],EBP         ; -y2(i+d)
         SBB    EBX,0                   ; EBX = carry
         MOV    28+Lk3[EDI],EAX         ; -y3(i+d)
         MOV    32+Lk3[EDI],EBX         ; carry
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
         DD     XfftA16
         DD     XfftA17
         DD     XfftA18
         DD     XfftA19
         DD     XfftA20
         DD     XfftA21
         DD     XfftA22
         DD     XfftA23
         DD     XfftA24
         DD     XfftA25
         DD     XfftA26
         DD     XfftA27
         DD     XfftA28
         DD     XfftA29
         DD     XfftA30
         DD     XfftA31
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
         MOV    EDX,32[ESI]
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
         MOV    4[ESI],EBX
         MOV    8[ESI],ECX
;
         MOV    EAX,12[EDI]
         MOV    EBX,16[EDI]
         SBB    EAX,ZW+12
         MOV    ECX,20[EDI]
         SBB    EBX,ZW+16
         MOV    12[ESI],EAX
         SBB    ECX,ZW+20
         MOV    16[ESI],EBX
         MOV    20[ESI],ECX
;
         MOV    EAX,24[EDI]
         MOV    EBX,28[EDI]
         SBB    EAX,ZW+24
         MOV    ECX,32[EDI]
         SBB    EBX,ZW+28
         MOV    24[ESI],EAX
         SBB    ECX,ZW+32
         MOV    28[ESI],EBX
         MOV    32[ESI],ECX
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
         MOV    ECX,ZW+20
         ADC    16[EDI],EBX
         MOV    EAX,ZW+24
         ADC    20[EDI],ECX
         MOV    EBX,ZW+28
         ADC    24[EDI],EAX
         MOV    ECX,ZW+32
         ADC    28[EDI],EBX
         ADC    32[EDI],ECX
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
;
         MOV    EAX,12[ESI]
         MOV    EBX,16[ESI]
         MOV    ECX,20[ESI]
         MOV    ZW+12,EAX
         MOV    ZW+16,EBX
         MOV    ZW+20,ECX
;
         MOV    EAX,24[ESI]
         MOV    EBX,28[ESI]
         MOV    ECX,32[ESI]
         MOV    ZW+24,EAX
         MOV    ZW+28,EBX
         MOV    ZW+32,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
;
; W = G^(kd-[kd/4]*4) W = G^kd X(2d,k,i+d)
XfftAshl:
         MOV    ECX,ZFkd
         MOV    EAX,ZW+32
         AND    CL,3
         MOV    EDX,ZW+28
         ADD    CL,CL
;
         SHLD   EAX,EDX,CL
         MOV    EBX,ZW+24
         MOV    ZW+32,EAX
         SHLD   EDX,EBX,CL
         MOV    EAX,ZW+20
         MOV    ZW+28,EDX
         SHLD   EBX,EAX,CL
         MOV    EDX,ZW+16
         MOV    ZW+24,EBX
;
         SHLD   EAX,EDX,CL
         MOV    EBX,ZW+12
         MOV    ZW+20,EAX
         SHLD   EDX,EBX,CL
         MOV    EAX,ZW+8
         MOV    ZW+16,EDX
         SHLD   EBX,EAX,CL
         MOV    EDX,ZW+4
         MOV    ZW+12,EBX
;
         SHLD   EAX,EDX,CL
         MOV    EBX,ZW+0
         MOV    ZW+8,EAX
         SHLD   EDX,EBX,CL
         MOV    ZW+4,EDX
         SHL    EBX,CL
         MOV    ZW+0,EBX
         JMP    XfftAsub
;
; W = G^4 X(2d,k,i+d),  [kd/4] = 1
XfftA01:
         MOV    ECX,28[ESI]
         MOV    EBP,28[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,32[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,EDX
         MOV    ZW+1,EBX
         MOV    ZW+5,ECX
;
         MOV    EAX,8[ESI]
         MOV    EBX,12[ESI]
         SBB    EAX,EDX
         MOV    ECX,16[ESI]
         SBB    EBX,EDX
         MOV    ZW+9,EAX
         SBB    ECX,EDX
         MOV    ZW+13,EBX
         MOV    ZW+17,ECX
;
         MOV    EAX,20[ESI]
         MOV    EBX,24[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+21,EAX
         SBB    EBP,EDX
         MOV    ZW+25,EBX
         SBB    ECX,EDX
         MOV    ZW+29,EBP
         MOV    ZW+33,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^8 X(2d,k,i+d),  [kd/4] = 2
XfftA02: 
         MOV    ECX,28[ESI]
         MOV    EBP,28[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,32[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,EDX
         MOV    ZW+2,EBX
         MOV    ZW+6,ECX
;
         MOV    EAX,8[ESI]
         MOV    EBX,12[ESI]
         SBB    EAX,EDX
         MOV    ECX,16[ESI]
         SBB    EBX,EDX
         MOV    ZW+10,EAX
         SBB    ECX,EDX
         MOV    ZW+14,EBX
         MOV    ZW+18,ECX
;
         MOV    EAX,20[ESI]
         MOV    EBX,24[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+22,EAX
         SBB    EBP,EDX
         MOV    ZW+26,EBX
         SBB    ECX,EDX
         MOV    ZW+30,EBP
         MOV    ZW+34,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^12 X(2d,k,i+d),  [kd/4] = 3
XfftA03: 
         MOV    ECX,28[ESI]
         MOV    EBP,28[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,32[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,EDX
         MOV    ZW+3,EBX
         MOV    ZW+7,ECX
;
         MOV    EAX,8[ESI]
         MOV    EBX,12[ESI]
         SBB    EAX,EDX
         MOV    ECX,16[ESI]
         SBB    EBX,EDX
         MOV    ZW+11,EAX
         SBB    ECX,EDX
         MOV    ZW+15,EBX
         MOV    ZW+19,ECX
;
         MOV    EAX,20[ESI]
         MOV    EBX,24[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+23,EAX
         SBB    EBP,EDX
         MOV    ZW+27,EBX
         SBB    ECX,EDX
         MOV    ZW+31,EBP
         MOV    ZW+35,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^16 X(2d,k,i+d),  [kd/4] = 4
XfftA04:
         MOV    EBX,0[ESI]
         SUB    EAX,28[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,32[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,EDX
         MOV    ZW+4,EBX
         MOV    ZW+8,ECX
;
         MOV    EAX,8[ESI]
         MOV    EBX,12[ESI]
         SBB    EAX,EDX
         MOV    ECX,16[ESI]
         SBB    EBX,EDX
         MOV    ZW+12,EAX
         SBB    ECX,EDX
         MOV    ZW+16,EBX
         MOV    ZW+20,ECX
;
         MOV    EAX,20[ESI]
         MOV    EBX,24[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+24,EAX
         SBB    ECX,EDX
         MOV    ZW+28,EBX
         MOV    ZW+32,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^20 X(2d,k,i+d),  [kd/4] = 5
XfftA05:
         MOV    ECX,24[ESI]
         MOV    EBP,24[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,28[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,32[ESI]
         MOV    ZW+1,EBX
         MOV    ZW+5,ECX
;
         MOV    EAX,4[ESI]
         MOV    EBX,8[ESI]
         SBB    EAX,EDX
         MOV    ECX,12[ESI]
         SBB    EBX,EDX
         MOV    ZW+9,EAX
         SBB    ECX,EDX
         MOV    ZW+13,EBX
         MOV    ZW+17,ECX
;
         MOV    EAX,16[ESI]
         MOV    EBX,20[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+21,EAX
         SBB    EBP,EDX
         MOV    ZW+25,EBX
         SBB    ECX,EDX
         MOV    ZW+29,EBP
         MOV    ZW+33,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^24 X(2d,k,i+d),  [kd/4] = 6
XfftA06: 
         MOV    ECX,24[ESI]
         MOV    EBP,24[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,28[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,32[ESI]
         MOV    ZW+2,EBX
         MOV    ZW+6,ECX
;
         MOV    EAX,4[ESI]
         MOV    EBX,8[ESI]
         SBB    EAX,EDX
         MOV    ECX,12[ESI]
         SBB    EBX,EDX
         MOV    ZW+10,EAX
         SBB    ECX,EDX
         MOV    ZW+14,EBX
         MOV    ZW+18,ECX
;
         MOV    EAX,16[ESI]
         MOV    EBX,20[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+22,EAX
         SBB    EBP,EDX
         MOV    ZW+26,EBX
         SBB    ECX,EDX
         MOV    ZW+30,EBP
         MOV    ZW+34,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^28 X(2d,k,i+d),  [kd/4] = 7
XfftA07: 
         MOV    ECX,24[ESI]
         MOV    EBP,24[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0[ESI]
         SBB    EBX,28[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,32[ESI]
         MOV    ZW+3,EBX
         MOV    ZW+7,ECX
;
         MOV    EAX,4[ESI]
         MOV    EBX,8[ESI]
         SBB    EAX,EDX
         MOV    ECX,12[ESI]
         SBB    EBX,EDX
         MOV    ZW+11,EAX
         SBB    ECX,EDX
         MOV    ZW+15,EBX
         MOV    ZW+19,ECX
;
         MOV    EAX,16[ESI]
         MOV    EBX,20[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+23,EAX
         SBB    EBP,EDX
         MOV    ZW+27,EBX
         SBB    ECX,EDX
         MOV    ZW+31,EBP
         MOV    ZW+35,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^32 X(2d,k,i+d),  [kd/4] = 8
XfftA08:
         MOV    EBX,0
         SUB    EAX,24[ESI]
         MOV    ECX,0[ESI]
         SBB    EBX,28[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,32[ESI]
         MOV    ZW+4,EBX
         MOV    ZW+8,ECX
;
         MOV    EAX,4[ESI]
         MOV    EBX,8[ESI]
         SBB    EAX,EDX
         MOV    ECX,12[ESI]
         SBB    EBX,EDX
         MOV    ZW+12,EAX
         SBB    ECX,EDX
         MOV    ZW+16,EBX
         MOV    ZW+20,ECX
;
         MOV    EAX,16[ESI]
         MOV    EBX,20[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+24,EAX
         SBB    ECX,EDX
         MOV    ZW+28,EBX
         MOV    ZW+32,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^36 X(2d,k,i+d),  [kd/4] = 9
XfftA09:
         MOV    ECX,20[ESI]
         MOV    EBP,20[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,24[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,28[ESI]
         MOV    ZW+1,EBX
         MOV    ZW+5,ECX
;
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         SBB    EAX,32[ESI]
         MOV    ECX,8[ESI]
         SBB    EBX,EDX
         MOV    ZW+9,EAX
         SBB    ECX,EDX
         MOV    ZW+13,EBX
         MOV    ZW+17,ECX
;
         MOV    EAX,12[ESI]
         MOV    EBX,16[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+21,EAX
         SBB    EBP,EDX
         MOV    ZW+25,EBX
         SBB    ECX,EDX
         MOV    ZW+29,EBP
         MOV    ZW+33,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^40 X(2d,k,i+d),  [kd/4] = 10
XfftA10: 
         MOV    ECX,20[ESI]
         MOV    EBP,20[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,24[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,28[ESI]
         MOV    ZW+2,EBX
         MOV    ZW+6,ECX
;
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         SBB    EAX,32[ESI]
         MOV    ECX,8[ESI]
         SBB    EBX,EDX
         MOV    ZW+10,EAX
         SBB    ECX,EDX
         MOV    ZW+14,EBX
         MOV    ZW+18,ECX
;
         MOV    EAX,12[ESI]
         MOV    EBX,16[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+22,EAX
         SBB    EBP,EDX
         MOV    ZW+26,EBX
         SBB    ECX,EDX
         MOV    ZW+30,EBP
         MOV    ZW+34,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^44 X(2d,k,i+d),  [kd/4] = 11
XfftA11: 
         MOV    ECX,20[ESI]
         MOV    EBP,20[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,24[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,28[ESI]
         MOV    ZW+3,EBX
         MOV    ZW+7,ECX
;
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         SBB    EAX,32[ESI]
         MOV    ECX,8[ESI]
         SBB    EBX,EDX
         MOV    ZW+11,EAX
         SBB    ECX,EDX
         MOV    ZW+15,EBX
         MOV    ZW+19,ECX
;
         MOV    EAX,12[ESI]
         MOV    EBX,16[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+23,EAX
         SBB    EBP,EDX
         MOV    ZW+27,EBX
         SBB    ECX,EDX
         MOV    ZW+31,EBP
         MOV    ZW+35,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^48 X(2d,k,i+d),  [kd/4] = 12
XfftA12:
         MOV    EBX,0
         SUB    EAX,20[ESI]
         MOV    ECX,0
         SBB    EBX,24[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,28[ESI]
         MOV    ZW+4,EBX
         MOV    ZW+8,ECX
;
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         SBB    EAX,32[ESI]
         MOV    ECX,8[ESI]
         SBB    EBX,EDX
         MOV    ZW+12,EAX
         SBB    ECX,EDX
         MOV    ZW+16,EBX
         MOV    ZW+20,ECX
;
         MOV    EAX,12[ESI]
         MOV    EBX,16[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+24,EAX
         SBB    ECX,EDX
         MOV    ZW+28,EBX
         MOV    ZW+32,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^52 X(2d,k,i+d),  [kd/4] = 13
XfftA13:
         MOV    ECX,16[ESI]
         MOV    EBP,16[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,20[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,24[ESI]
         MOV    ZW+1,EBX
         MOV    ZW+5,ECX
;
         MOV    EAX,0
         MOV    EBX,0[ESI]
         SBB    EAX,28[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,32[ESI]
         MOV    ZW+9,EAX
         SBB    ECX,EDX
         MOV    ZW+13,EBX
         MOV    ZW+17,ECX
;
         MOV    EAX,8[ESI]
         MOV    EBX,12[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+21,EAX
         SBB    EBP,EDX
         MOV    ZW+25,EBX
         SBB    ECX,EDX
         MOV    ZW+29,EBP
         MOV    ZW+33,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^56 X(2d,k,i+d),  [kd/4] = 14
XfftA14: 
         MOV    ECX,16[ESI]
         MOV    EBP,16[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,20[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,24[ESI]
         MOV    ZW+2,EBX
         MOV    ZW+6,ECX
;
         MOV    EAX,0
         MOV    EBX,0[ESI]
         SBB    EAX,28[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,32[ESI]
         MOV    ZW+10,EAX
         SBB    ECX,EDX
         MOV    ZW+14,EBX
         MOV    ZW+18,ECX
;
         MOV    EAX,8[ESI]
         MOV    EBX,12[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+22,EAX
         SBB    EBP,EDX
         MOV    ZW+26,EBX
         SBB    ECX,EDX
         MOV    ZW+30,EBP
         MOV    ZW+34,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^60 X(2d,k,i+d),  [kd/4] = 15
XfftA15: 
         MOV    ECX,16[ESI]
         MOV    EBP,16[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,20[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,24[ESI]
         MOV    ZW+3,EBX
         MOV    ZW+7,ECX
;
         MOV    EAX,0
         MOV    EBX,0[ESI]
         SBB    EAX,28[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,32[ESI]
         MOV    ZW+11,EAX
         SBB    ECX,EDX
         MOV    ZW+15,EBX
         MOV    ZW+19,ECX
;
         MOV    EAX,8[ESI]
         MOV    EBX,12[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+23,EAX
         SBB    EBP,EDX
         MOV    ZW+27,EBX
         SBB    ECX,EDX
         MOV    ZW+31,EBP
         MOV    ZW+35,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^66 X(2d,k,i+d),  [kd/4] = 16
XfftA16:
         MOV    EBX,0
         SUB    EAX,16[ESI]
         MOV    ECX,0
         SBB    EBX,20[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,24[ESI]
         MOV    ZW+4,EBX
         MOV    ZW+8,ECX
;
         MOV    EAX,0
         MOV    EBX,0[ESI]
         SBB    EAX,28[ESI]
         MOV    ECX,4[ESI]
         SBB    EBX,32[ESI]
         MOV    ZW+12,EAX
         SBB    ECX,EDX
         MOV    ZW+16,EBX
         MOV    ZW+20,ECX
;
         MOV    EAX,8[ESI]
         MOV    EBX,12[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+24,EAX
         SBB    ECX,EDX
         MOV    ZW+28,EBX
         MOV    ZW+32,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^68 X(2d,k,i+d),  [kd/4] = 17
XfftA17:
         MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,16[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,20[ESI]
         MOV    ZW+1,EBX
         MOV    ZW+5,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,24[ESI]
         MOV    ECX,0[ESI]
         SBB    EBX,28[ESI]
         MOV    ZW+9,EAX
         SBB    ECX,32[ESI]
         MOV    ZW+13,EBX
         MOV    ZW+17,ECX
;
         MOV    EAX,4[ESI]
         MOV    EBX,8[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+21,EAX
         SBB    EBP,EDX
         MOV    ZW+25,EBX
         SBB    ECX,EDX
         MOV    ZW+29,EBP
         MOV    ZW+33,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^72 X(2d,k,i+d),  [kd/4] = 18
XfftA18: 
         MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,16[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,20[ESI]
         MOV    ZW+2,EBX
         MOV    ZW+6,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,24[ESI]
         MOV    ECX,0[ESI]
         SBB    EBX,28[ESI]
         MOV    ZW+10,EAX
         SBB    ECX,32[ESI]
         MOV    ZW+14,EBX
         MOV    ZW+18,ECX
;
         MOV    EAX,4[ESI]
         MOV    EBX,8[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+22,EAX
         SBB    EBP,EDX
         MOV    ZW+26,EBX
         SBB    ECX,EDX
         MOV    ZW+30,EBP
         MOV    ZW+34,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^76 X(2d,k,i+d),  [kd/4] = 19
XfftA19: 
         MOV    ECX,12[ESI]
         MOV    EBP,12[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,16[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,20[ESI]
         MOV    ZW+3,EBX
         MOV    ZW+7,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,24[ESI]
         MOV    ECX,0[ESI]
         SBB    EBX,28[ESI]
         MOV    ZW+11,EAX
         SBB    ECX,32[ESI]
         MOV    ZW+15,EBX
         MOV    ZW+19,ECX
;
         MOV    EAX,4[ESI]
         MOV    EBX,8[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+23,EAX
         SBB    EBP,EDX
         MOV    ZW+27,EBX
         SBB    ECX,EDX
         MOV    ZW+31,EBP
         MOV    ZW+35,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^80 X(2d,k,i+d),  [kd/4] = 20
XfftA20:
         MOV    EBX,0
         SUB    EAX,12[ESI]
         MOV    ECX,0
         SBB    EBX,16[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,20[ESI]
         MOV    ZW+4,EBX
         MOV    ZW+8,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,24[ESI]
         MOV    ECX,0[ESI]
         SBB    EBX,28[ESI]
         MOV    ZW+12,EAX
         SBB    ECX,32[ESI]
         MOV    ZW+16,EBX
         MOV    ZW+20,ECX
;
         MOV    EAX,4[ESI]
         MOV    EBX,8[ESI]
         SBB    EAX,EDX
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+24,EAX
         SBB    ECX,EDX
         MOV    ZW+28,EBX
         MOV    ZW+32,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^84 X(2d,k,i+d),  [kd/4] = 21
XfftA21:
         MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FF000000h
         AND    EBP,000FFFFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,12[ESI]
         MOV    ZW-3,EAX
         SBB    ECX,16[ESI]
         MOV    ZW+1,EBX
         MOV    ZW+5,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,20[ESI]
         MOV    ECX,0
         SBB    EBX,24[ESI]
         MOV    ZW+9,EAX
         SBB    ECX,28[ESI]
         MOV    ZW+13,EBX
         MOV    ZW+17,ECX
;
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         SBB    EAX,32[ESI]
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+21,EAX
         SBB    EBP,EDX
         MOV    ZW+25,EBX
         SBB    ECX,EDX
         MOV    ZW+29,EBP
         MOV    ZW+33,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^88 X(2d,k,i+d),  [kd/4] = 22
XfftA22: 
         MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FFFF0000h
         AND    EBP,00000FFFFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,12[ESI]
         MOV    ZW-2,EAX
         SBB    ECX,16[ESI]
         MOV    ZW+2,EBX
         MOV    ZW+6,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,20[ESI]
         MOV    ECX,0
         SBB    EBX,24[ESI]
         MOV    ZW+10,EAX
         SBB    ECX,28[ESI]
         MOV    ZW+14,EBX
         MOV    ZW+18,ECX
;
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         SBB    EAX,32[ESI]
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+22,EAX
         SBB    EBP,EDX
         MOV    ZW+26,EBX
         SBB    ECX,EDX
         MOV    ZW+30,EBP
         MOV    ZW+34,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^92 X(2d,k,i+d),  [kd/4] = 23
XfftA23: 
         MOV    ECX,8[ESI]
         MOV    EBP,8[ESI]
         AND    ECX,0FFFFFF00h
         AND    EBP,0000000FFh
         SUB    EAX,ECX
         MOV    EBX,0
         MOV    ECX,0
         SBB    EBX,12[ESI]
         MOV    ZW-1,EAX
         SBB    ECX,16[ESI]
         MOV    ZW+3,EBX
         MOV    ZW+7,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,20[ESI]
         MOV    ECX,0
         SBB    EBX,24[ESI]
         MOV    ZW+11,EAX
         SBB    ECX,28[ESI]
         MOV    ZW+15,EBX
         MOV    ZW+19,ECX
;
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         SBB    EAX,32[ESI]
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+23,EAX
         SBB    EBP,EDX
         MOV    ZW+27,EBX
         SBB    ECX,EDX
         MOV    ZW+31,EBP
         MOV    ZW+35,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^96 X(2d,k,i+d),  [kd/4] = 24
XfftA24:
         MOV    EBX,0
         SUB    EAX,8[ESI]
         MOV    ECX,0
         SBB    EBX,12[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,16[ESI]
         MOV    ZW+4,EBX
         MOV    ZW+8,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,20[ESI]
         MOV    ECX,0
         SBB    EBX,24[ESI]
         MOV    ZW+12,EAX
         SBB    ECX,28[ESI]
         MOV    ZW+16,EBX
         MOV    ZW+20,ECX
;
         MOV    EAX,0[ESI]
         MOV    EBX,4[ESI]
         SBB    EAX,32[ESI]
         MOV    ECX,0
         SBB    EBX,EDX
         MOV    ZW+24,EAX
         SBB    ECX,EDX
         MOV    ZW+28,EBX
         MOV    ZW+32,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^100 X(2d,k,i+d),  [kd/4] = 25
XfftA25:
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
         MOV    ZW+1,EBX
         MOV    ZW+5,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,16[ESI]
         MOV    ECX,0
         SBB    EBX,20[ESI]
         MOV    ZW+9,EAX
         SBB    ECX,24[ESI]
         MOV    ZW+13,EBX
         MOV    ZW+17,ECX
;
         MOV    EAX,0
         MOV    EBX,0[ESI]
         SBB    EAX,28[ESI]
         MOV    ECX,0
         SBB    EBX,32[ESI]
         MOV    ZW+21,EAX
         SBB    EBP,EDX
         MOV    ZW+25,EBX
         SBB    ECX,EDX
         MOV    ZW+29,EBP
         MOV    ZW+33,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^104 X(2d,k,i+d),  [kd/4] = 26
XfftA26: 
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
         MOV    ZW+2,EBX
         MOV    ZW+6,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,16[ESI]
         MOV    ECX,0
         SBB    EBX,20[ESI]
         MOV    ZW+10,EAX
         SBB    ECX,24[ESI]
         MOV    ZW+14,EBX
         MOV    ZW+18,ECX
;
         MOV    EAX,0
         MOV    EBX,0[ESI]
         SBB    EAX,28[ESI]
         MOV    ECX,0
         SBB    EBX,32[ESI]
         MOV    ZW+22,EAX
         SBB    EBP,EDX
         MOV    ZW+26,EBX
         SBB    ECX,EDX
         MOV    ZW+30,EBP
         MOV    ZW+34,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^108 X(2d,k,i+d),  [kd/4] = 27
XfftA27: 
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
         MOV    ZW+3,EBX
         MOV    ZW+7,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,16[ESI]
         MOV    ECX,0
         SBB    EBX,20[ESI]
         MOV    ZW+11,EAX
         SBB    ECX,24[ESI]
         MOV    ZW+15,EBX
         MOV    ZW+19,ECX
;
         MOV    EAX,0
         MOV    EBX,0[ESI]
         SBB    EAX,28[ESI]
         MOV    ECX,0
         SBB    EBX,32[ESI]
         MOV    ZW+23,EAX
         SBB    EBP,EDX
         MOV    ZW+27,EBX
         SBB    ECX,EDX
         MOV    ZW+31,EBP
         MOV    ZW+35,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^112 X(2d,k,i+d),  [kd/4] = 28
XfftA28:
         MOV    EBX,0
         SUB    EAX,4[ESI]
         MOV    ECX,0
         SBB    EBX,8[ESI]
         MOV    ZW+0,EAX
         SBB    ECX,12[ESI]
         MOV    ZW+4,EBX
         MOV    ZW+8,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,16[ESI]
         MOV    ECX,0
         SBB    EBX,20[ESI]
         MOV    ZW+12,EAX
         SBB    ECX,24[ESI]
         MOV    ZW+16,EBX
         MOV    ZW+20,ECX
;
         MOV    EAX,0
         MOV    EBX,0[ESI]
         SBB    EAX,28[ESI]
         MOV    ECX,0
         SBB    EBX,32[ESI]
         MOV    ZW+24,EAX
         SBB    ECX,EDX
         MOV    ZW+28,EBX
         MOV    ZW+32,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^116 X(2d,k,i+d),  [kd/4] = 29
XfftA29:
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
         MOV    ZW+1,EBX
         MOV    ZW+5,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,12[ESI]
         MOV    ECX,0
         SBB    EBX,16[ESI]
         MOV    ZW+9,EAX
         SBB    ECX,20[ESI]
         MOV    ZW+13,EBX
         MOV    ZW+17,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,24[ESI]
         MOV    ECX,0
         SBB    EBX,28[ESI]
         MOV    ZW+21,EAX
         SBB    EBP,32[ESI]
         MOV    ZW+25,EBX
         SBB    ECX,EDX
         MOV    ZW+29,EBP
         MOV    ZW+33,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^120 X(2d,k,i+d),  [kd/4] = 30
XfftA30: 
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
         MOV    ZW+2,EBX
         MOV    ZW+6,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,12[ESI]
         MOV    ECX,0
         SBB    EBX,16[ESI]
         MOV    ZW+10,EAX
         SBB    ECX,20[ESI]
         MOV    ZW+14,EBX
         MOV    ZW+18,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,24[ESI]
         MOV    ECX,0
         SBB    EBX,28[ESI]
         MOV    ZW+22,EAX
         SBB    EBP,32[ESI]
         MOV    ZW+26,EBX
         SBB    ECX,EDX
         MOV    ZW+30,EBP
         MOV    ZW+34,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^124 X(2d,k,i+d),  [kd/4] = 31
XfftA31: 
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
         MOV    ZW+3,EBX
         MOV    ZW+7,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,12[ESI]
         MOV    ECX,0
         SBB    EBX,16[ESI]
         MOV    ZW+11,EAX
         SBB    ECX,20[ESI]
         MOV    ZW+15,EBX
         MOV    ZW+19,ECX
;
         MOV    EAX,0
         MOV    EBX,0
         SBB    EAX,24[ESI]
         MOV    ECX,0
         SBB    EBX,28[ESI]
         MOV    ZW+23,EAX
         SBB    EBP,32[ESI]
         MOV    ZW+27,EBX
         SBB    ECX,EDX
         MOV    ZW+31,EBP
         MOV    ZW+35,ECX
;
         TEST   ZFkd,3
         JZ     XfftAsub
         JMP    XfftAshl
;====================================================
; Reduce X(i) modulo F, i = 0, 1, ...
;
; Before:  -2^287 < X(i) < 2^287
; After:   0 <= X(i) <= 2^256  
;
; Let X = X(i) = A + 2^256 B
;
; If X >= 0 then X = A - B (mod F)
;           If A - B < 0 then add 2^256 + 1
;
; If X <  0 then X = A + 2^256 B - 2^288
;                  = A + 2^32 - B (mod F)
;
XfftR:   
XfftRb:  MOV    EAX,32[EDI]
         MOV    dword ptr 32+[EDI],0
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
         SBB    dword ptr 16[EDI],0
         JNC    XfftRx
         SBB    dword ptr 20[EDI],0
         JNC    XfftRx
         SBB    dword ptr 24[EDI],0
         JNC    XfftRx
         SBB    dword ptr 28[EDI],0
         JNC    XfftRx
         ADC    dword ptr 0[EDI],0
         JNC    XfftRx
         ADC    dword ptr 4[EDI],0
         JNC    XfftRx
XfftRg:  ADC    dword ptr 8[EDI],0
         JNC    XfftRx
         ADC    dword ptr 12[EDI],0
         JNC    XfftRx
         ADC    dword ptr 16[EDI],0
         JNC    XfftRx
         ADC    dword ptr 20[EDI],0
         JNC    XfftRx
         ADC    dword ptr 24[EDI],0
         JNC    XfftRx
         ADC    dword ptr 28[EDI],0
         JNC    XfftRx
; Must be  a + 2^256,  a < 2^32 
         ADC    dword ptr 32[EDI],0
         TEST   dword ptr 0[EDI],0FFFFFFFFh
         JZ     XfftRx
         DEC    dword ptr 0[EDI]
         MOV    dword ptr 32[EDI],0
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
; X(i) => X(i)^2
;
; 0 <= X(i) <= 2^256
; ZX[ESI] -> X(i),  ZX[EDI] -> X(i)^2
;
XfftQ:   MOV    ZSc,2*(256+9)
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
         TEST   BYTE PTR[32+ESI],1
         JNZ    XfftQu
;-1  0  7 
  MOV  EAX,0[ESI]
  MOV  EDX,28[ESI]
  MOV  ECX,0
  MOV  EBX,0
  MUL  EDX
  MOV  EBP,EAX
  SUB  ECX,EDX
  SBB  EBX,0
  ADD  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
;-1  1  6 
  MOV  EAX,4[ESI]
  MOV  EDX,24[ESI]
  MUL  EDX
  ADD  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  ADD  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
;-1  2  5 
  MOV  EAX,8[ESI]
  MOV  EDX,20[ESI]
  MUL  EDX
  ADD  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  ADD  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
;-1  3  4 
  MOV  EAX,12[ESI]
  MOV  EDX,16[ESI]
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
  MUL  EAX
  SAR  EBP,31
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
; 0  7  1 
  MOV  EAX,28[ESI]
  MOV  EDX,4[ESI]
  MUL  EDX
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
; 0  6  2 
  MOV  EAX,24[ESI]
  MOV  EDX,8[ESI]
  MUL  EDX
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
; 0  5  3 
  MOV  EAX,20[ESI]
  MOV  EDX,12[ESI]
  MUL  EDX
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
; 0  4  4 
  MOV  EAX,16[ESI]
  MUL  EAX
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  MOV  0 [EDI],ECX
; 1  0  1 
  MOV  EAX,0[ESI]
  MOV  EDX,4[ESI]
  MOV  ECX,EBP
  MUL  EDX
  SAR  ECX,31
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
; 1  7  2 
  MOV  EAX,28[ESI]
  MOV  EDX,8[ESI]
  MUL  EDX
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
; 1  6  3 
  MOV  EAX,24[ESI]
  MOV  EDX,12[ESI]
  MUL  EDX
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
; 1  5  4 
  MOV  EAX,20[ESI]
  MOV  EDX,16[ESI]
  MUL  EDX
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
  MOV  4 [EDI],EBX
; 2  0  2 
  MOV  EAX,0[ESI]
  MOV  EDX,8[ESI]
  MOV  EBX,ECX
  MUL  EDX
  SAR  EBX,31
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
; 2  7  3 
  MOV  EAX,28[ESI]
  MOV  EDX,12[ESI]
  MUL  EDX
  SUB  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  SUB  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
; 2  6  4 
  MOV  EAX,24[ESI]
  MOV  EDX,16[ESI]
  MUL  EDX
  SUB  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  SUB  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
; 2  5  5 
  MOV  EAX,20[ESI]
  MUL  EAX
  SUB  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  MOV  8 [EDI],EBP
; 3  0  3 
  MOV  EAX,0[ESI]
  MOV  EDX,12[ESI]
  MOV  EBP,EBX
  MUL  EDX
  SAR  EBP,31
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
; 3  1  2 
  MOV  EAX,4[ESI]
  MOV  EDX,8[ESI]
  MUL  EDX
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
; 3  7  4 
  MOV  EAX,28[ESI]
  MOV  EDX,16[ESI]
  MUL  EDX
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
; 3  6  5 
  MOV  EAX,24[ESI]
  MOV  EDX,20[ESI]
  MUL  EDX
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  SUB  ECX,EAX
  SBB  EBX,EDX
  SBB  EBP,0
  MOV  12 [EDI],ECX
; 4  0  4 
  MOV  EAX,0[ESI]
  MOV  EDX,16[ESI]
  MOV  ECX,EBP
  MUL  EDX
  SAR  ECX,31
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
; 4  1  3 
  MOV  EAX,4[ESI]
  MOV  EDX,12[ESI]
  MUL  EDX
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
; 4  2  2 
  MOV  EAX,8[ESI]
  MUL  EAX
  ADD  EBX,EAX
  ADC  EBP,EDX
  ADC  ECX,0
; 4  7  5 
  MOV  EAX,28[ESI]
  MOV  EDX,20[ESI]
  MUL  EDX
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
; 4  6  6 
  MOV  EAX,24[ESI]
  MUL  EAX
  SUB  EBX,EAX
  SBB  EBP,EDX
  SBB  ECX,0
  MOV  16 [EDI],EBX
; 5  0  5 
  MOV  EAX,0[ESI]
  MOV  EDX,20[ESI]
  MOV  EBX,ECX
  MUL  EDX
  SAR  EBX,31
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
; 5  1  4 
  MOV  EAX,4[ESI]
  MOV  EDX,16[ESI]
  MUL  EDX
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
; 5  2  3 
  MOV  EAX,8[ESI]
  MOV  EDX,12[ESI]
  MUL  EDX
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
  ADD  EBP,EAX
  ADC  ECX,EDX
  ADC  EBX,0
; 5  7  6 
  MOV  EAX,28[ESI]
  MOV  EDX,24[ESI]
  MUL  EDX
  SUB  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  SUB  EBP,EAX
  SBB  ECX,EDX
  SBB  EBX,0
  MOV  20 [EDI],EBP
; 6  0  6 
  MOV  EAX,0[ESI]
  MOV  EBP,EBX
  MOV  EDX,24[ESI]
  SAR  EBP,31
  ADD  EBX,ZW
  ADC  EBP,0
  MUL  EDX
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
; 6  1  5 
  MOV  EAX,4[ESI]
  MOV  EDX,20[ESI]
  MUL  EDX
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
; 6  2  4 
  MOV  EAX,8[ESI]
  MOV  EDX,16[ESI]
  MUL  EDX
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
; 6  3  3 
  MOV  EAX,12[ESI]
  MUL  EAX
  ADD  ECX,EAX
  ADC  EBX,EDX
  ADC  EBP,0
; 6  7  7 
  MOV  EAX,28[ESI]
  MUL  EAX
  SUB  ECX,EAX
  SBB  EBX,EDX
  MOV  24[EDI],ECX
  SBB  EBP,0
  MOV  28[EDI],EBX
  MOV  32[EDI],EBP
; 486 clock ticks: 760 
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
         MOV    ECX,20[ESI]
         MOV    12[EDI],EAX
         MOV    16[EDI],EBX
         MOV    20[EDI],ECX
         MOV    EAX,24[ESI]
         MOV    EBX,28[ESI]
         MOV    ECX,32[ESI]
         MOV    24[EDI],EAX
         MOV    28[EDI],EBX
         MOV    32[EDI],ECX
         JMP    XfftQk
;
; (2^256)^2 = 1 (mod F)
XfftQu:  MOV    dword ptr 0[EDI],1
         MOV    dword ptr 4[EDI],0
         MOV    dword ptr 8[EDI],0
         MOV    dword ptr 12[EDI],0
         MOV    dword ptr 16[EDI],0
         MOV    dword ptr 20[EDI],0
         MOV    dword ptr 24[EDI],0
         MOV    dword ptr 28[EDI],0
         MOV    dword ptr 32[EDI],0
         JMP    XfftQk
;====================================================
;
_AR2F88A endp
_TEXT    ends
         end
