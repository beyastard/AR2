#include "ar2w.h"

unsigned long sv_u;
unsigned long sv_y;

unsigned long Wbx;
unsigned long Wdx;

/*
; a (mod b)
;
; Registers:   eax = a, edx = b.
; Assumptions: 0 <= a < 2^32, 0 < b < 2^32.
; Return:      eax = a (mod b).
*/
long wmod(long a, long b)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		eax,a
		mov		edx,b

		push	ebx
		mov		ebx,edx
		xor		edx,edx
		div		ebx
		mov		eax,edx
		pop		ebx
		
		mov		edx,Wdx
		mov		result,eax
	}

	return result;
}

/*
; a (mod b)
;
; Registers:   eax = a, edx = b.
; Assumptions: -2^31 <= a < 2^31, 1 < b < 2^31.
; Return:      eax = a (mod b).
*/
long wmods(long a, long b)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		eax,a
		mov		edx,b

		push	ebx
		mov 	ebx,edx
		mov 	edx,eax      ; Extend sgn(a) to edx 
		sar 	edx,31 
		idiv	ebx          ; Signed integer division 
		cmp 	edx,0
		jnl 	wmods5
		add 	edx,ebx      ; Add b if negative
wmods5:
		mov		eax,edx
		pop		ebx
		
		mov		edx,Wdx
		mov		result,eax
	}

	return result;
}

/*
; a*d (mod b)
;
; Registers:   eax = a, edx = d, ebx = c.
; Assumptions: 0 <= a < 2^32, 0 < b < 2^32, a*d < b*2^32.
; Return:      eax = a*d (mod b).
*/
long wmulmod(long a, long d, long c)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		Wbx,ebx
		mov		eax,a
		mov		edx,d
		mov		ebx,c
		
		mul		edx
        div		ebx
        mov		eax,edx
        
		mov		edx,Wdx
		mov		ebx,Wbx
		mov		result,eax
	}

	return result;
}

/*
; a^d (mod b)
;
; Registers:   eax = a, edx = d, ebx = b.
; Assumptions: 0 <= a, d < 2^32, 0 < b < 2^32.
; Return:      eax = a^d (mod b).
*/
long wexpmod(long a, long d, long b)
{
	long result;

	__asm	
	{
		mov		Wdx,edx
		mov		Wbx,ebx
		mov		eax,a
		mov		edx,d
		mov		ebx,b
		
		push	ecx
        push	esi
        push	edi
        mov 	ecx,eax                ; ecx = a
        mov 	esi,edx                ; esi = d
        mov 	edi,32
        mov 	eax,1
wemod2:
		cmp 	eax,1
        jna 	wemod4
        mul 	eax
        div 	ebx
        mov 	eax,edx
wemod4:
		shl 	esi,1
        jnc 	wemod6
        mul 	ecx
        div 	ebx
        mov 	eax,edx
wemod6:
		dec 	edi
        jnz 	wemod2
        pop 	edi
        pop 	esi
        pop 	ecx
        
		mov		edx,Wdx
		mov		ebx,Wbx
		mov		result,eax
	}

	return result;
}

/*
; u = 1/x (mod p), y = gcd(x, p)
;
; Registers:   eax = x, edx = p.
; Assumptions: 0 <= x < p < 2^31, p > 0.
; Return:      eax = 1/x (mod p) if gcd(p, x) = 1,
;              eax = 0 otherwise.
*/
long winvmod(long a, long b)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		eax,a
		mov		edx,b
		
		push	edi
        push	esi
        push	ebp
        push	ecx
        push	ebx
        mov 	ebp,edx         ; ebp = p
;
; See Henri Cohen, "A Course in Computational Algebraic 
; Number Theory", 1.3.2, for details of this algorithm. 
; See AR2I.C for C version.
;
; Numbers can be negative, but never outside range [-p, p], 
; temp2 and temp4 are always >= 0,
; result is always in [-(p-1)/2, (p-1)/2].
;
        mov		sv_u,1          ; u = 1
        mov		sv_y,eax        ; y = a
        xor		ebx,ebx         ; ebx = temp1 = 0
        mov		ecx,ebp         ; ecx = temp2 = p
wimod2:
		mov 	eax,sv_y 
        xor 	edx,edx         ; edx:eax = temp5 = y
        idiv	ecx             ; eax = [temp5/temp2]
        mov 	esi,edx         ; esi = temp4 = temp5%temp2
        imul	eax,ebx         ; eax = temp5 * temp1
        mov 	edi,sv_u        ; edi = temp3 = u
        sub 	edi,eax         ; edi = temp3 - temp5
        mov 	sv_u,ebx        ; u = temp1
        mov 	sv_y,ecx        ; y = temp2
        mov 	ebx,edi         ; temp1 = temp3
        mov 	ecx,esi         ; temp2 = temp4
        and 	esi,esi
        jnz 	wimod2
;
        cmp		sv_y,1
        jne		wimod8          ; gcd(x, p) != 1
;
        mov		eax,sv_u        ; eax = 1/x (mod p)
        cmp		eax,0
        jnl		wimod6
        add		eax,ebp         ; Add p if < 0
wimod6:
		pop		ebx
        pop		ecx
        pop		ebp
        pop		esi
        pop		edi
        
		mov		edx,Wdx
		jmp		wimod9
wimod8:
		xor		eax,eax
        jmp		wimod6
wimod9:
		mov		result,eax
	}

	return result;
}

/*
; a^d
;
; Registers:   eax = a, edx = d.
; Assumptions: 0 <= a, d < 2^32.
; Return:      eax = a^d.
*/
long wexp(long a, long b)
{
	long result;

	__asm
	{
		mov		Wdx,edx
		mov		eax,a
		mov		edx,b
		
		push	ecx
        push	esi
        push	edi
        mov 	ecx,eax                ; ecx = a
        mov 	esi,edx                ; esi = d
        mov 	edi,32
        mov 	eax,1
wexp2:
		cmp 	eax,1
        jna 	wexp4
        mul 	eax
wexp4:
		shl 	esi,1
        jnc 	wexp6
        mul 	ecx
wexp6:
		dec 	edi
        jnz 	wexp2
        pop 	edi
        pop 	esi
        pop 	ecx
		
		mov		edx,Wdx
		mov		result,eax
	}

	return result;
}

/*
; (log a)/(log 2) + 1
;
; Registers:   eax = a.
; Assumptions: 0 <= a < 2^32.
; Return:      eax = length of a.
*/
long wlen(long a)
{
	long result;

	__asm
	{
		mov		eax,a
		
		push	edx
        mov 	edx,eax
        xor 	eax,eax
        and 	edx,edx
        jz  	wlen4
wlen2:
		shr 	edx,1
        inc 	eax
        jnz 	wlen2
wlen4:
		pop 	edx
        
		mov		result,eax
	}

	return result;
}
