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

    