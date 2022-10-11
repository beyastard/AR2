;
; AR2F89A  2.1.0m  TONY FORBES  December 1994
;
; a = a^2  (using FFT modulo F_8, dimension 2^9)
;
; xsqufft89 (&a, d, &f);
;
; Registers:   EAX = &a, EDX = d = d(a)
;              EBX = &f, a 4617-digit work area
; Assumptions: 0 <= a < 2^30720, d(a) > 0 
;              a has sufficient room for 1922 digits
;
; Finite Fourier Transform for a => a^2
;
; F = the Fourier base, the Fermat number F_8 = 2^256 + 1
; D = 512, the dimension of the Fourier transform
; G = 2, a primitive 512th root of unity modulo F
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
         public _AR2F89data
_AR2F89data dd 0
         org    _AR2F89data
Zcx      dd     0        ; Save ECX
Zbp      dd     0        ; Save EBP
Zsi      dd     0        ; Save ESI
Zdi      dd     0        ; Save EDI
ZYptr    dd     0        ; EAX = &a
ZXptr    dd     0        ; EBX = &f
;
LnF      =      9*4      ; length of F
LnM      =      15       ; length of M
Lyh      =      128*15   ; offset to high order y
Lk0      =      0        ; offset to 128-block 0
Lk1      =      128*9*4  ; offset to 128-block 1
Lk2      =      256*9*4  ; offset to 128-block 2
Lk3      =      384*9*4  ; offset to 128-block 3
Lk4      =      512*9*4  ; offset to end of FFT array
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
; where b(i) = i bit-reversed modulo 512
Tb  DW   0,9216,4608,13824,2304,11520,6912,16128 
    DW   1152,10368,5760,14976,3456,12672,8064,17280 
    DW   576,9792,5184,14400,2880,12096,7488,16704 
    DW   1728,10944,6336,15552,4032,13248,8640,17856 
    DW   288,9504,4896,14112,2592,11808,7200,16416 
    DW   1440,10656,6048,15264,3744,12960,8352,17568 
    DW   864,10080,5472,14688,3168,12384,7776,16992 
    DW   2016,11232,6624,15840,4320,13536,8928,18144 
    DW   144,9360,4752,13968,2448,11664,7056,16272 
    DW   1296,10512,5904,15120,3600,12816,8208,17424 
    DW   720,9936,5328,14544,3024,12240,7632,16848 
    DW   1872,11088,6480,15696,4176,13392,8784,18000 
    DW   432,9648,5040,14256,2736,11952,7344,16560 
    DW   1584,10800,6192,15408,3888,13104,8496,17712 
    DW   1008,10224,5616,14832,3312,12528,7920,17136 
    DW   2160,11376,6768,15984,4464,13680,9072,18288 
    DW   72,9288,4680,13896,2376,11592,6984,16200 
    DW   1224,10440,5832,15048,3528,12744,8136,17352 
    DW   648,9864,5256,14472,2952,12168,7560,16776 
    DW   1800,11016,6408,15624,4104,13320,8712,17928 
    DW   360,9576,4968,14184,2664,11880,7272,16488 
    DW   1512,10728,6120,15336,3816,13032,8424,17640 
    DW   936,10152,5544,14760,3240,12456,7848,17064 
    DW   2088,11304,6696,15912,4392,13608,9000,18216 
    DW   216,9432,4824,14040,2520,11736,7128,16344 
    DW   1368,10584,5976,15192,3672,12888,8280,17496 
    DW   792,10008,5400,14616,3096,12312,7704,16920 
    DW   1944,11160,6552,15768,4248,13464,8856,18072 
    DW   504,9720,5112,14328,2808,12024,7416,16632 
    DW   1656,10872,6264,15480,3960,13176,8568,17784 
    DW   1080,10296,5688,14904,3384,12600,7992,17208 
    DW   2232,11448,6840,16056,4536,13752,9144,18360 
    DW   36,9252,4644,13860,2340,11556,6948,16164 
    DW   1188,10404,5796,15012,3492,12708,8100,17316 
    DW   612,9828,5220,14436,2916,12132,7524,16740 
    DW   1764,10980,6372,15588,4068,13284,8676,17892 
    DW   324,9540,4932,14148,2628,11844,7236,16452 
    DW   1476,10692,6084,15300,3780,12996,8388,17604 
    DW   900,10116,5508,14724,3204,12420,7812,17028 
    DW   2052,11268,6660,15876,4356,13572,8964,18180 
    DW   180,9396,4788,14004,2484,11700,7092,16308 
    DW   1332,10548,5940,15156,3636,12852,8244,17460 
    DW   756,9972,5364,14580,3060,12276,7668,16884 
    DW   1908,11124,6516,15732,4212,13428,8820,18036 
    DW   468,9684,5076,14292,2772,11988,7380,16596 
    DW   1620,10836,6228,15444,3924,13140,8532,17748 
    DW   1044,10260,5652,14868,3348,12564,7956,17172 
    DW   2196,11412,6804,16020,4500,13716,9108,18324 
    DW   108,9324,4716,13932,2412,11628,7020,16236 
    DW   1260,10476,5868,15084,3564,12780,8172,17388 
    DW   684,9900,5292,14508,2988,12204,7596,16812 
    DW   1836,11052,6444,15660,4140,13356,8748,17964 
    DW   396,9612,5004,14220,2700,11916,7308,16524 
    DW   1548,10764,6156,15372,3852,13068,8460,17676 
    DW   972,10188,5580,14796,3276,12492,7884,17100 
    DW   2124,11340,6732,15948,4428,13644,9036,18252 
    DW   252,9468,4860,14076,2556,11772,7164,16380 
    DW   1404,10620,6012,15228,3708,12924,8316,17532 
    DW   828,10044,5436,14652,3132,12348,7740,16956 
    DW   1980,11196,6588,15804,4284,13500,8892,18108 
    DW   540,9756,5148,14364,2844,12060,7452,16668 
    DW   1692,10908,6300,15516,3996,13212,8604,17820 
    DW   1116,10332,5724,14940,3420,12636,8028,17244 
    DW   2268,11484,6876,16092,4572,13788,9180,18396 
;
; Table for controlling X => X^2
;
; X(0) = X(0)^2, 
; X(512-b(k)) = X(k)^2,  k = 1, 2, ..., 511
;
; k => 512-b(k) cycle structure
;
; (0) c=1 
; (1,256,511) c=3 
; (2,384,509,129,254,258,383,3,128,510,257,255) c=12 
; (4,448,505,193,250,322,379,67,124,388,445,133,
;  190,262,319,7,64,508,385,253,130,382,259,127) c=24 
; (5,192,506,321,251,66,380,387,125,132,446,261,
;  191,6,320,507,65,252,386,381,131,126,260,447) c=24 
; (8,480,497,225,242,354,371,99,116,420,437,165,
;  182,294,311,39,56,456,473,201,218,330,347,75,
;  92,396,413,141,158,270,287,15,32,504,449,249,
;  194,378,323,123,68,444,389,189,134,318,263,63) c=48 
; (9,224,498,353,243,98,372,419,117,164,438,293,
;  183,38,312,455,57,200,474,329,219,74,348,395,
;  93,140,414,269,159,14,288,503,33,248,450,377,
;  195,122,324,443,69,188,390,317,135,62,264,479) c=48 
; (10,352,499,97,244,418,373,163,118,292,439,37,
;  184,454,313,199,58,328,475,73,220,394,349,139,
;  94,268,415,13,160,502,289,247,34,376,451,121,
;  196,442,325,187,70,316,391,61,136,478,265,223) c=48 
; (11,96,500,417,245,162,374,291,119,36,440,453,
;  185,198,314,327,59,72,476,393,221,138,350,267,
;  95,12,416,501,161,246,290,375,35,120,452,441,
;  197,186,326,315,71,60,392,477,137,222,266,351) c=48 
; (16,496,481,241,226,370,355,115,100,436,421,181,166,310,295,55,
;  40,472,457,217,202,346,331,91,76,412,397,157,142,286,271,31) c=32 
; (17,240,482,369,227,114,356,435,101,180,422,309,167,54,296,471,
;  41,216,458,345,203,90,332,411,77,156,398,285,143,30,272,495) c=32 
; (18,368,483,113,228,434,357,179,102,308,423,53,168,470,297,215,
;  42,344,459,89,204,410,333,155,78,284,399,29,144,494,273,239) c=32 
; (19,112,484,433,229,178,358,307,103,52,424,469,169,214,298,343,
;  43,88,460,409,205,154,334,283,79,28,400,493,145,238,274,367) c=32 
; (20,432,485,177,230,306,359,51,104,468,425,213,170,342,299,87,
;  44,408,461,153,206,282,335,27,80,492,401,237,146,366,275,111) c=32 
; (21,176,486,305,231,50,360,467,105,212,426,341,171,86,300,407,
;  45,152,462,281,207,26,336,491,81,236,402,365,147,110,276,431) c=32 
; (22,304,487,49,232,466,361,211,106,340,427,85,172,406,301,151,
;  46,280,463,25,208,490,337,235,82,364,403,109,148,430,277,175) c=32 
; (23,48,488,465,233,210,362,339,107,84,428,405,173,150,302,279,
;  47,24,464,489,209,234,338,363,83,108,404,429,149,174,278,303) c=32 
;
Tq  DW   Lk4
    DW   0,Lk4 ; c=1 
    DW   36,9216,18396,Lk4 ; c=3 
    DW   72,13824,18324,4644,9144,9288,13788,108,4608,18360,9252,9180,Lk4 ; c=12 
    DW   144,16128,18180,6948,9000,11592,13644,2412,4464,13968,16020,4788  
    DW   6840,9432,11484,252,2304,18288,13860,9108,4680,13752,9324,4572,Lk4 ; c=24 
    DW   180,6912,18216,11556,9036,2376,13680,13932,4500,4752,16056,9396  
    DW   6876,216,11520,18252,2340,9072,13896,13716,4716,4536,9360,16092,Lk4 ; c=24 
    DW   288,17280,17892,8100,8712,12744,13356,3564,4176,15120,15732,5940  
    DW   6552,10584,11196,1404,2016,16416,17028,7236,7848,11880,12492,2700  
    DW   3312,14256,14868,5076,5688,9720,10332,540,1152,18144,16164,8964  
    DW   6984,13608,11628,4428,2448,15984,14004,6804,4824,11448,9468,2268,Lk4 ; c=48 
    DW   324,8064,17928,12708,8748,3528,13392,15084,4212,5904,15768,10548  
    DW   6588,1368,11232,16380,2052,7200,17064,11844,7884,2664,12528,14220  
    DW   3348,5040,14904,9684,5724,504,10368,18108,1188,8928,16200,13572  
    DW   7020,4392,11664,15948,2484,6768,14040,11412,4860,2232,9504,17244,Lk4 ; c=48 
    DW   360,12672,17964,3492,8784,15048,13428,5868,4248,10512,15804,1332  
    DW   6624,16344,11268,7164,2088,11808,17100,2628,7920,14184,12564,5004  
    DW   3384,9648,14940,468,5760,18072,10404,8892,1224,13536,16236,4356  
    DW   7056,15912,11700,6732,2520,11376,14076,2196,4896,17208,9540,8028,Lk4 ; c=48 
    DW   396,3456,18000,15012,8820,5832,13464,10476,4284,1296,15840,16308  
    DW   6660,7128,11304,11772,2124,2592,17136,14148,7956,4968,12600,9612  
    DW   3420,432,14976,18036,5796,8856,10440,13500,1260,4320,16272,15876  
    DW   7092,6696,11736,11340,2556,2160,14112,17172,4932,7992,9576,12636,Lk4 ; c=48 
    DW   576,17856,17316,8676,8136,13320,12780,4140,3600,15696,15156,6516  
    DW   5976,11160,10620,1980,1440,16992,16452,7812,7272,12456,11916,3276  
    DW   2736,14832,14292,5652,5112,10296,9756,1116,Lk4 ; c=32 
    DW   612,8640,17352,13284,8172,4104,12816,15660,3636,6480,15192,11124  
    DW   6012,1944,10656,16956,1476,7776,16488,12420,7308,3240,11952,14796  
    DW   2772,5616,14328,10260,5148,1080,9792,17820,Lk4 ; c=32 
    DW   648,13248,17388,4068,8208,15624,12852,6444,3672,11088,15228,1908  
    DW   6048,16920,10692,7740,1512,12384,16524,3204,7344,14760,11988,5580  
    DW   2808,10224,14364,1044,5184,17784,9828,8604,Lk4 ; c=32 
    DW   684,4032,17424,15588,8244,6408,12888,11052,3708,1872,15264,16884  
    DW   6084,7704,10728,12348,1548,3168,16560,14724,7380,5544,12024,10188  
    DW   2844,1008,14400,17748,5220,8568,9864,13212,Lk4 ; c=32 
    DW   720,15552,17460,6372,8280,11016,12924,1836,3744,16848,15300,7668  
    DW   6120,12312,10764,3132,1584,14688,16596,5508,7416,10152,12060,972  
    DW   2880,17712,14436,8532,5256,13176,9900,3996,Lk4 ; c=32 
    DW   756,6336,17496,10980,8316,1800,12960,16812,3780,7632,15336,12276  
    DW   6156,3096,10800,14652,1620,5472,16632,10116,7452,936,12096,17676  
    DW   2916,8496,14472,13140,5292,3960,9936,15516,Lk4 ; c=32 
    DW   792,10944,17532,1764,8352,16776,12996,7596,3816,12240,15372,3060  
    DW   6192,14616,10836,5436,1656,10080,16668,900,7488,17640,12132,8460  
    DW   2952,13104,14508,3924,5328,15480,9972,6300,Lk4 ; c=32 
    DW   828,1728,17568,16740,8388,7560,13032,12204,3852,3024,15408,14580  
    DW   6228,5400,10872,10044,1692,864,16704,17604,7524,8424,12168,13068  
    DW   2988,3888,14544,15444,5364,6264,10008,10908,Lk4 ; c=32
;
_DATA   ends
;
_TEXT    segment dword public 'CODE'
         assume  CS:_TEXT
         assume  DS:DGROUP
_AR2F89A proc    near
;========================================
;
; y => y^2 by Finite Fourier Transform
;
         public xsqufft89_
xsqufft89_:
         mov    Zcx,ECX
         mov    Zbp,EBP
         mov    Zsi,ESI
         mov    Zdi,EDI
         mov    ZYptr,EAX            ; &a
         mov    ZXptr,EBX            ; &f (FFT array)
;
; Clear high order digits, if necessary
         lea    EDI,[EAX+EDX*4]      ; First high order digit
         lea    ESI,3840[EAX]        ; Limit
Xfft2:   cmp    EDI,ESI
         jae    Xfft3
         mov    dword ptr [EDI],0
         lea    EDI,4[EDI]
         jmp    Xfft2
Xfft3:
;
; 9-stage Fast Fourier Transform 
         CALL   XfftI      ; d = 256, 128
         MOV    ZFd,64
         MOV    ZFe,4
         MOV    ZFj,64*LnF
         CALL   XfftA
         CALL   XfftR
;
; X^2
         CALL   XfftQ
;
; 9-stage Fast Fourier Transform 
         MOV    ZFd,256
         MOV    ZFe,1
         MOV    ZFj,256*LnF
         CALL   XfftA
         CALL   XfftS
;
; y = x/512
; X(i)   => y,  i = 0, 1, ..., 510
; X(511) is always zero
;
; HHHHGGGGFFFFEEEEDDDDCCCCBBBBAAAA000ihhhhggggffffeeeeddddccccbbbbaaaa
; MOV Y,X+1                       0000ihhhhggggffffeeeeddddccccbbbbaaa
;                                   eeeeddddccccbbbbaaaa    ADD/ADC
;                   000ihhhhggggffff                        ADC 0, MOV
;                    eeeeddddccccbbbbaaaa                   ADD/ADC
;    000ihhhhggggffff                                       ADC 0, MOV
;
         mov    ESI,ZXptr
         mov    EDI,ZYptr
         MOV    EAX,0[ESI]
         MOV    EDX,4[ESI]
;
         SHRD   EAX,EDX,9
         MOV    EBX,8[ESI]
         MOV    0[EDI],EAX
         SHRD   EDX,EBX,9
         MOV    EAX,12[ESI]
         MOV    4[EDI],EDX
         SHRD   EBX,EAX,9
         MOV    EDX,16[ESI]
         MOV    8[EDI],EBX
;
         SHRD   EAX,EDX,9
         MOV    EBX,20[ESI]
         MOV    12[EDI],EAX
         SHRD   EDX,EBX,9
         MOV    EAX,24[ESI]
         MOV    16[EDI],EDX
         SHRD   EBX,EAX,9
         MOV    EDX,28[ESI]
         MOV    20[EDI],EBX
;
         SHRD   EAX,EDX,9
         MOV    EBX,32[ESI]
         MOV    24[EDI],EAX
         SHRD   EDX,EBX,9
         MOV    dword ptr 32[EDI],0
         MOV    28[EDI],EDX
;
         MOV    EBP,2
         add    EDI,LnM
Xfft6:   MOV    SI,Tb[EBP]
         AND    ESI,0FFFFh
         add    ESI,ZXptr
         MOV    EAX,0[ESI]
         MOV    EDX,4[ESI]
;
         SHRD   EAX,EDX,9
         MOV    EBX,8[ESI]
         MOV    ZU+0,EAX
         SHRD   EDX,EBX,9
         MOV    EAX,12[ESI]
         MOV    ZU+4,EDX
         SHRD   EBX,EAX,9
         MOV    EDX,16[ESI]
         MOV    ZU+8,EBX
;
         SHRD   EAX,EDX,9
         MOV    EBX,20[ESI]
         MOV    ZU+12,EAX
         SHRD   EDX,EBX,9
         MOV    EAX,24[ESI]
         MOV    ZU+16,EDX
         SHRD   EBX,EAX,9
         MOV    EDX,28[ESI]
         MOV    ZU+20,EBX
;
         SHRD   EAX,EDX,9
         MOV    EBX,32[ESI]
         MOV    ZU+24,EAX
         SHRD   EDX,EBX,9
         MOV    EAX,ZU+0
         MOV    ZU+28,EDX
;
         MOV    EBX,ZU+4
         ADD    0[EDI],EAX
         MOV    ECX,ZU+8
         ADC    4[EDI],EBX
         MOV    EDX,ZU+12
         ADC    8[EDI],ECX
         MOV    EAX,ZU+16
         ADC    12[EDI],EDX
         MOV    EBX,ZU+20
         ADC    16[EDI],EAX
         MOV    ECX,ZU+24
         ADC    EBX,0
         MOV    EDX,ZU+28
         ADC    ECX,0
         MOV    20[EDI],EBX
         ADC    EDX,0
         MOV    24[EDI],ECX
         MOV    28[EDI],EDX
;
         MOV    dword ptr 32[EDI],0
         ADD    EBP,2
         ADD    EDI,LnM
         CMP    EBP,2*510
         JLE    Xfft6
;
         mov    EDI,Zdi
         mov    ESI,Zsi
         mov    EBP,Zbp
         mov    ECX,Zcx
         retn   
;========================================
; FFT special first step: y => X, d = 128
;
; X(128,0,i) = y(i) +       y(i+128)
; X(128,1,i) = y(i) + G^128 y(i+128)
; X(128,2,i) = y(i) -       y(i+128)
; X(128,3,i) = y(i) - G^128 y(i+128)
;
; i = 0, 1, ..., 127
;
; 0 <= y(i) < 2^120
;
XfftI:   mov    ESI,ZYptr               ; ESI -> y
         mov    EDI,ZXptr               ; EDI -> f
         add    ESI,127*LnM
         add    EDI,127*LnF
;
; Pick up high order words
XfftId:  MOV    ECX,12[ESI]
         MOV    EDX,12+Lyh[ESI]
         AND    ECX,00FFFFFFh           ; ECX = y3(i)
         AND    EDX,00FFFFFFh           ; EDX = y3(i+d)
;
; X(128,2,i) = y(i) - y(i+128)  [at offset Lk1]
; X(128,3,i) = y(i)
; -2^120 < X(128,2,i) < 2^120
         MOV    EAX,0[ESI]              ; EAX = y0(i)
         MOV    EBX,4[ESI]              ; EBX = y1(i)
         MOV    EBP,8[ESI]              ; EBP = y2(i)
         MOV     0+Lk3[EDI],EAX         ; y0(i)
         MOV     4+Lk3[EDI],EBX         ; y1(i)
         SUB    EAX,0+Lyh[ESI]          ; EAX = y0(i) - y0(i+d)
         MOV     8+Lk3[EDI],EBP         ; y2(i)
         SBB    EBX,4+Lyh[ESI]          ; EBX = y1(i) - y1(i+d)
         MOV     0+Lk1[EDI],EAX         ; y0(i) - y0(i+d)
         SBB    EBP,8+Lyh[ESI]          ; EBP = y2(i) - y2(i+d)
         MOV     4+Lk1[EDI],EBX         ; y1(i) - y1(i+d)
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
; X(128,0,i) = y(i) +       y(i+128)
; X(128,1,i) = y(i) + G^128 y(i+128)  [at offset Lk2]
; 0 <= X(128,0,i) < 2^121
; 0 <= X(128,1,i) < 2^248
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
; Subtract  G^128 y(i+128)  from  X(128,3,i)
; -2^248 < X(128,3,i) < 2^120
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
         SHR    EBP,3          ; EBP = [kd/8]
         AND    EDI,0FFFFh     
         MOV    EAX,ZFd        ; d
         add    EDI,ZXptr      ; EDI -> X(2d,k,i)
         MOV    EBP,XfftAj[EBP*4]
         MOV    ZFi,EAX        ; counter for i
         MOV    ZFX,EBP        ; EBP = code offset for [kd/8]
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
; W = X(2d,k,i+d),  [kd/8] = 0
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
         TEST   ZFkd,7
         JZ     XfftAsub
;
; W = G^(kd-[kd/8]*8) W = G^kd X(2d,k,i+d)
XfftAshl:
         MOV    ECX,ZFkd
         MOV    EAX,ZW+32
         MOV    EDX,ZW+28
         AND    CL,7
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
; W = G^8 X(2d,k,i+d),  [kd/8] = 1
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^16 X(2d,k,i+d),  [kd/8] = 2
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^24 X(2d,k,i+d),  [kd/8] = 3
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^32 X(2d,k,i+d),  [kd/8] = 4
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^40 X(2d,k,i+d),  [kd/8] = 5
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^48 X(2d,k,i+d),  [kd/8] = 6
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^56 X(2d,k,i+d),  [kd/8] = 7
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^64 X(2d,k,i+d),  [kd/8] = 8
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^72 X(2d,k,i+d),  [kd/8] = 9
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^80 X(2d,k,i+d),  [kd/8] = 10
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^88 X(2d,k,i+d),  [kd/8] = 11
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^96 X(2d,k,i+d),  [kd/8] = 12
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^104 X(2d,k,i+d),  [kd/8] = 13
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^112 X(2d,k,i+d),  [kd/8] = 14
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^120 X(2d,k,i+d),  [kd/8] = 15
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^128 X(2d,k,i+d),  [kd/8] = 16
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^136 X(2d,k,i+d),  [kd/8] = 17
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^144 X(2d,k,i+d),  [kd/8] = 18
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^152 X(2d,k,i+d),  [kd/8] = 19
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^160 X(2d,k,i+d),  [kd/8] = 20
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^168 X(2d,k,i+d),  [kd/8] = 21
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^176 X(2d,k,i+d),  [kd/8] = 22
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^184 X(2d,k,i+d),  [kd/8] = 23
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^192 X(2d,k,i+d),  [kd/8] = 24
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^200 X(2d,k,i+d),  [kd/8] = 25
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^208 X(2d,k,i+d),  [kd/8] = 26
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^216 X(2d,k,i+d),  [kd/8] = 27
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^224 X(2d,k,i+d),  [kd/8] = 28
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^232 X(2d,k,i+d),  [kd/8] = 29
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^240 X(2d,k,i+d),  [kd/8] = 30
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;
; W = G^248 X(2d,k,i+d),  [kd/8] = 31
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
         TEST   ZFkd,7
         JZ     XfftAsub
         JMP    XfftAshl
;====================================================
; Reduce X(i) modulo F, i = 0, 1, ..., 511
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
XfftR:   MOV    EDI,ZXptr
         add    EDI,511*LnF
;
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
; Determine X(i) from X(i) modulo F and D, 
; i = 0, 1, ..., 510
;
; Before:  -2^287 < X(i) < 2^287
; After:   0 <= X(i) <= 2^265, 512|X(i)  
;
; As XfftR except that we must add  b(2^256 + 1) 
; if  X(i) = -b (mod 512)
;
XfftS:   MOV    EDI,ZXptr
         add    EDI,510*LnF
;
XfftSb:  MOV    EAX,32[EDI]
         MOV    dword ptr 32[EDI],0
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
         SBB    dword ptr 16[EDI],0
         JNC    XfftSt
         SBB    dword ptr 20[EDI],0
         JNC    XfftSt
         SBB    dword ptr 24[EDI],0
         JNC    XfftSt
         SBB    dword ptr 28[EDI],0
         JNC    XfftSt
         ADC    dword ptr 0[EDI],0
         JNC    XfftSt
         ADC    dword ptr 4[EDI],0
         JNC    XfftSt
XfftSg:  ADC    dword ptr 8[EDI],0
         JNC    XfftSt
         ADC    dword ptr 12[EDI],0
         JNC    XfftSt
         ADC    dword ptr 16[EDI],0
         JNC    XfftSt
         ADC    dword ptr 20[EDI],0
         JNC    XfftSt
         ADC    dword ptr 24[EDI],0
         JNC    XfftSt
         ADC    dword ptr 28[EDI],0
         JNC    XfftSt
; Must be  a + 2^256,  a < 2^32 
         ADC    dword ptr 32[EDI],0
         TEST   dword ptr 0[EDI],0FFFFFFFFh
         JZ     XfftSx
         DEC    dword ptr 0[EDI]
         MOV    dword ptr 32[EDI],0
         JMP    XfftSt
; X(i) < 0
XfftSn:  SUB    0[EDI],EAX
         JC     XfftSt
         ADD    dword ptr 4[EDI],1
         JC     XfftSg
;
XfftSt:  MOV    EAX,0[EDI]
         AND    EAX,511
         JZ     XfftSx
         NEG    EAX
         ADD    EAX,512
         ADD    32[EDI],EAX
         ADD    0[EDI],EAX
         JNC    XfftSx
         ADC    dword ptr 4[EDI],0
         JNC    XfftSx
         ADC    dword ptr 8[EDI],0
         JNC    XfftSx
         ADC    dword ptr 12[EDI],0
         JNC    XfftSx
         ADC    dword ptr 16[EDI],0
         JNC    XfftSx
         ADC    dword ptr 20[EDI],0
         JNC    XfftSx
         ADC    dword ptr 24[EDI],0
         ADC    dword ptr 28[EDI],0
         ADC    dword ptr 32[EDI],0
XfftSx:  MOV    EAX,ZXptr
         SUB    EDI,LnF
         cmp    EDI,EAX
         JNL    XfftSb
         RETN
;====================================================
; X(i) => X(i)^2
;
; 0 <= X(i) <= 2^256
; ZX[ESI] -> X(i),  ZX[EDI] -> X(i)^2
;
XfftQ:   MOV    ZSc,2*(512+17)
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
_AR2F89A endp
_TEXT    ends
         end
