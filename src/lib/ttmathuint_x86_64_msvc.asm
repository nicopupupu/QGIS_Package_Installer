;
; This file is a part of TTMath Bignum Library
; and is distributed under the (new) BSD licence.
; Author: Christian Kaiser <chk@online.de>
;

; 
; Copyright (c) 2009, Christian Kaiser
; All rights reserved.
; 
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
; 
;  * Redistributions of source code must retain the above copyright notice,
;    this list of conditions and the following disclaimer.
;    
;  * Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.
;    
;  * Neither the name Christian Kaiser nor the names of contributors to this
;    project may be used to endorse or promote products derived
;    from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
; THE POSSIBILITY OF SUCH DAMAGE.
;

;
; compile with debug info:    ml64.exe /c /Zd /Zi ttmathuint_x86_64_msvc.asm
; compile without debug info: ml64.exe /c ttmathuint_x86_64_msvc.asm
; this creates ttmathuint_x86_64_msvc.obj file which can be linked with your program
;

PUBLIC	ttmath_adc_x64
PUBLIC	ttmath_addindexed_x64
PUBLIC	ttmath_addindexed2_x64
PUBLIC	ttmath_addvector_x64

PUBLIC	ttmath_sbb_x64
PUBLIC	ttmath_subindexed_x64
PUBLIC	ttmath_subvector_x64

PUBLIC	ttmath_rcl_x64
PUBLIC	ttmath_rcr_x64

PUBLIC	ttmath_rcl2_x64
PUBLIC	ttmath_rcr2_x64

PUBLIC	ttmath_div_x64

;
; Microsoft x86_64 convention: http://msdn.microsoft.com/en-us/library/9b372w95.aspx
;
;	"rax, rcx, rdx, r8-r11 are volatile."
;	"rbx, rbp, rdi, rsi, r12-r15 are nonvolatile."
;


.CODE


        ALIGN       8

;----------------------------------------

ttmath_adc_x64				PROC
        ; rcx = p1
        ; rdx = p2
        ; r8 = nSize
        ; r9 = nCarry

        xor		rax, rax
        xor		r11, r11
        sub		rax, r9		; sets CARRY if r9 != 0

		ALIGN 16
 loop1:
		mov		rax,qword ptr [rdx + r11 * 8]
		adc		qword ptr [rcx + r11 * 8], rax
		lea		r11, [r11+1]
		dec		r8
		jnz		loop1

		setc	al
		movzx	rax, al

		ret

ttmath_adc_x64				ENDP

;----------------------------------------

        ALIGN       8

;----------------------------------------

ttmath_addindexed_x64	PROC

        ; rcx = p1
        ; rdx = nSize
        ; r8 = nPos
        ; r9 = nValue

		xor		rax, rax			; rax = result
		sub		rdx, r8				; rdx = remaining count of uints

		add		qword ptr [rcx + r8 * 8], r9
		jc		next1

		ret

next1:
		mov		r9, 1

		ALIGN 16
loop1:
		dec		rdx
		jz		done_with_cy
		lea		r8, [r8+1]
		add		qword ptr [rcx + r8 * 8], r9
		jc		loop1

		ret

done_with_cy:
		lea		rax, [rax+1]		; rax = 1

		ret

ttmath_addindexed_x64	ENDP

;----------------------------------------

        ALIGN       8

;----------------------------------------

ttmath_addindexed2_x64	PROC

        ; rcx = p1 (pointer)
        ; rdx = b  (value size)
        ; r8 = nPos
        ; r9 = nValue1
        ; [esp+0x28] = nValue2

		xor		rax, rax			; return value
		mov		r11, rcx			; table
		sub		rdx, r8				; rdx = remaining count of uints
		mov		r10, [esp+028h]		; r10 = nValue2

		add		qword ptr [r11 + r8 * 8], r9
		lea		r8, [r8+1]
		lea		rdx, [rdx-1]
		adc		qword ptr [r11 + r8 * 8], r10
		jc		next
		ret

		ALIGN 16
loop1:
		lea		r8, [r8+1]
		add		qword ptr [r11 + r8 * 8], 1
		jc		next
		ret

next:
		dec		rdx					; does not modify CY too...
		jnz		loop1
		lea		rax, [rax+1]
		ret

ttmath_addindexed2_x64	ENDP



;----------------------------------------

        ALIGN       8

;----------------------------------------


ttmath_addvector_x64				PROC
     