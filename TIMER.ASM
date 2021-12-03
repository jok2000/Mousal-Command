
;    Copyright Ad Lib Inc., 1990

;    This file is part of the Ad Lib Programmer's Manual product and is
;    subject to copyright laws.  As such, you may make copies of this file
;    only for the purpose of having backup copies.  This file may not be
;    redistributed in any form whatsoever.

;    If you find yourself in possession of this file without having received
;    it directly from Ad Lib Inc., then you are in violation of copyright
;    laws, which is a form of theft.

; TIMER.ASM
;
; 08h interrupt driver (from BYTE special issue, nov. 86, pp 249-262)
;
; Adaptation: Marc Savary, Ad Lib Inc, 1986/11/03
;

;	This module allows you to change the timer-0 interrupt rate,
;   without affecting the rate at which the currently installed
;   routine is called  (18.2 Hz).
;
;   A user function (TimeOut()) is called after 'n'
;   interrupts, 'n' being reset after each call.


; Originally written for Lattice C Compiler, small model.  Adapted to
; Microsoft by using macros (notably BEGIN, P_END, extrn).  A flag for the
; type of compiler is defined in the file VERSION.INC.  According to this
; flag, the file containing the appropriate equates and macros is then
; included.

; 3-may-89, Dale Glowinski
; Removed stuff for Lattice interface.  Adapted to MASM V1.51


.MODEL SMALL

; @codesize is predefined by MASM: 0 = small, compact; 1 = med, large, huge
IF  @codesize
   CPSIZE       EQU  4
ELSE
   CPSIZE       EQU  2
ENDIF

clk_int equ 08h     ; timer-0 interrupt vector number


; ============= vector structure ==========

vector	struc
regip	dw	?
regcs	dw	?
vector	ends


; ========================= DATA SEGMENT ============================

.DATA

int_stack_size       equ 512

the_stack            db  int_stack_size DUP ('S')
interrupt_stack_top  dw  0


;========================== CODE SEGMENT ============================

.CODE


IF  @codesize
   extrn  _TimeOut: far
ELSE
   extrn  _TimeOut: near
ENDIF


clkdivh    dw  ?       ; actual divisor ... high
clkdivl    dw  ?       ; ... low
clkmod     dw  ?       ; divisor modulus

int08      vector  <>

appl_ds    dw  ?
old_ss     dw  ?       ; interrupted code's SS
old_sp     dw  ?       ; SP .......

soundDelay dw  ?       ; delay counter

user_routine_on db (?) ; flag to avoid reentrance




;	clkrate()
;
;	change timer-0 divider
;	IN : AX count divisor

clkrate proc
	; load counter 0 of 8253:
	push    ax
	mov     al, 00110110b   ; square wave mode
	out     43h, al
	pop     ax
	out     40h, al
	xchg	ah, al
	out     40h, al
	xchg	ah, al
	ret
clkrate endp


;   _SetInt (state)
;
;	enable/disable CPU interrupt.

public SetInt

SetInt proc

Sintframe struc
		dw  (?)
		db  CPSIZE DUP (?)
state	dw	(?)			; interrupt state
Sintframe ends
	push    bp
	mov     bp, sp

	cmp     [bp].state, 0
	jne     s_on
	; off:
	cli
	jmp     s_end
s_on:
	sti
s_end:
	pop     bp
	ret

SetInt endp




;   _SetClkRate (unsigned count)
;
;	Initialize interrupt rate to (1.119 MHz / count) cycles/sec.
;
public _SetClkRate

_SetClkRate proc

scrframe struc
		dw  (?)
		db  CPSIZE DUP (?)
divid	dw	(?)			; timer's divider
scrframe ends
	push	bp
	mov     bp, sp

	mov     ax, [bp].divid
	pushf
	cli	
	mov     CS:clkdivl, ax
	cmp     ax, 1
	mov     CS:clkdivh, 0
	adc     CS:clkdivh, 0
	call	clkrate
	popf
	pop     bp
	ret
_SetClkRate endp



; Install clock-driver (soft int-08, timer-0).
; Save a copy of DS.
;
public _Clk_install

_Clk_install proc

; install clock interrupt handler
	push    ax
	push	dx

; init. clk variables:
	xor     ax, ax
	call	clkrate
	mov     CS:clkdivh, 1
	mov     CS:clkdivl, ax
	mov     CS:clkmod, ax

; init flag:
	mov     cs:user_routine_on, 0

; save application DS:
	mov     ax, ds
	mov     cs:appl_ds, ax

; save current int. vector
	push	es
	mov     ah, 35h
	mov     al, clk_int
	int     21h         ; get old vector
	assume  es:nothing
	mov     CS:int08.regip, bx
	mov     CS:int08.regcs, es
	pop     es
; install interrupt intercept vector:
	push	ds
	mov     ah, 25h
	mov     al, clk_int
	mov     dx, offset clkint
	mov     bx, CS
	mov     ds, bx
	int     21h         ; set int. vector
	pop     ds
	pop     dx
	pop     ax

	ret
_Clk_install endp



;   _clk_uninstall()
;
public _Clk_uninstall

_Clk_uninstall proc

	xor     ax, ax
	call	clkrate
; reset int. vector:
	push	ds
	mov     ah, 25h
	mov     al, clk_int
	lds     dx, CS:int08
	int     21h         ; set vector ...
	pop     ds
	ret
_Clk_uninstall endp




;   _StartTimeOut (delay)
;
;	Initialize count-down delay to 'delay'.
;

public _StartTimeOut

_StartTimeOut proc

istad	struc
		dw  (?)
		db  CPSIZE DUP (?)
delay	dw	(?)			; delay before doing the next call
istad	ends

	push	bp
	mov     bp, sp
	pushf
	cli
	mov     ax, [bp].delay
	mov     cs:soundDelay, ax
	popf
	pop     bp
	ret

_StartTimeOut endp



; 	clkint
;
; int-08 Interrupt Driver routine.
;
; Check for roll-over of 65536 cycles  (18.2 hz ) and call
; old driver if so.
; 
; Count-down delay variable, and if zero, call routine 'TimeOut()'
; & set the new delay.
;
clkint  proc

	push	ax

; check for roll-over of 65536 cycles  (18.2 hz )
	mov     ax, CS:clkdivl
	add     CS:clkmod, ax
	mov     ax, CS:clkdivh
	adc     ax, 0
	jnz     clkint8
; not yet time, skip original interrupt
	mov     al, 00100000b
	out     20h, al         ; 8259 ...
	jmp     clkint7
; do the original interrupt:
clkint8	label	near
	pushf
	call	CS:int08

clkint7	label	near

	dec     CS:soundDelay       ; 16 bits unsigned counter
	jnz     clkint_end

; to avoid a reentrant call
	cmp     CS:user_routine_on, 0
	jnz     clkint_end      ; already active ...

; end of delay. Prepare environment before calling TimeOut()
; (allocate temporary stack, set segment registers).
;

; save all registers...	
	push	bx
	push	cx
	push	dx
	push	ds
	push	es
	push	si
	push	di
	push	bp

; save active stack pointers
	mov     cs:old_ss, ss
	mov     cs:old_sp, sp

; get application's DS
	mov     ax, cs:appl_ds
	mov     es, ax
	mov     ds, ax

; set new stack:
	mov     ss, ax
	mov     sp, offset DGROUP:interrupt_stack_top


go_user	label near
public go_user

; protect call with flag
	inc     CS:user_routine_on

; call the C routine
	sti
	call    _TimeOut         ; time-out driver ... ==> AX: new delay

	cli
; 
	dec     CS:user_routine_on

; compute new delay
	mov     bx, CS:soundDelay
	neg     bx              ; # of interrupt since call to TimeOut()
	cmp     bx, ax              ; time-out ?
	jb      clk_delay_ok            ; no ...

; we must recall TimeOut immediately
	mov     CS:soundDelay, 0
	jmp     go_user

clk_delay_ok	label near
	add     CS:soundDelay, ax       ; leftover delay count

; restore stack
	mov     bx, CS:old_ss
	mov     ss, bx
	mov     sp, CS:old_sp

	sti

	pop     bp
	pop     di
	pop     si
	pop     es
	pop     ds
	pop     dx
	pop     cx
	pop     bx

clkint_end	label near
	pop     ax
	iret
clkint	endp

	end
