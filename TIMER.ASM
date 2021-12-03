
; 0x8 interrupt driver From the BYTE magazine special issue, 11/1986 page 249
;

.MODEL SMALL

IF  @codesize
   CPSIZE       EQU  4
ELSE
   CPSIZE       EQU  2
ENDIF

clk_int equ 08h 


vector	struc
regip	dw	?
regcs	dw	?
vector	ends

.DATA

int_stack_size       equ 512

the_stack            db  int_stack_size DUP ('S')
interrupt_stack_top  dw  0

.CODE


IF  @codesize
   extrn  _TimeOut: far
ELSE
   extrn  _TimeOut: near
ENDIF


clkdivh    dw  ?     
clkdivl    dw  ?     
clkmod     dw  ?      

int08      vector  <>

appl_ds    dw  ?
old_ss     dw  ?     
old_sp     dw  ?      

soundDelay dw  ?     

user_routine_on db (?) 





clkrate proc
	push    ax
	mov     al, 00110110b   ; square wave 
	out     43h, al
	pop     ax
	out     40h, al
	xchg	ah, al
	out     40h, al
	xchg	ah, al
	ret
clkrate endp



public SetInt

SetInt proc

Sintframe struc
		dw  (?)
		db  CPSIZE DUP (?)
state	dw	(?)		
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

public _SetClkRate

_SetClkRate proc

scrframe struc
		dw  (?)
		db  CPSIZE DUP (?)
divid	dw	(?)			
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



public _Clk_install

_Clk_install proc

	push    ax
	push	dx

	xor     ax, ax
	call	clkrate
	mov     CS:clkdivh, 1
	mov     CS:clkdivl, ax
	mov     CS:clkmod, ax

	mov     cs:user_routine_on, 0

	mov     ax, ds
	mov     cs:appl_ds, ax

	push	es
	mov     ah, 35h
	mov     al, clk_int
	int     21h       
	assume  es:nothing
	mov     CS:int08.regip, bx
	mov     CS:int08.regcs, es
	pop     es
	push	ds
	mov     ah, 25h
	mov     al, clk_int
	mov     dx, offset clkint
	mov     bx, CS
	mov     ds, bx
	int     21h      
	pop     ds
	pop     dx
	pop     ax

	ret
_Clk_install endp


public _Clk_uninstall

_Clk_uninstall proc

	xor     ax, ax
	call	clkrate
	push	ds
	mov     ah, 25h
	mov     al, clk_int
	lds     dx, CS:int08
	int     21h        
	pop     ds
	ret
_Clk_uninstall endp




public _StartTimeOut

_StartTimeOut proc

istad	struc
		dw  (?)
		db  CPSIZE DUP (?)
delay	dw	(?)
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


clkint  proc

	push	ax

	mov     ax, CS:clkdivl
	add     CS:clkmod, ax
	mov     ax, CS:clkdivh
	adc     ax, 0
	jnz     clkint8
	mov     al, 00100000b
	out     20h, al    
	jmp     clkint7
clkint8	label	near
	pushf
	call	CS:int08

clkint7	label	near

	dec     CS:soundDelay      
	jnz     clkint_end

	cmp     CS:user_routine_on, 0
	jnz     clkint_end     

	push	bx
	push	cx
	push	dx
	push	ds
	push	es
	push	si
	push	di
	push	bp

	mov     cs:old_ss, ss
	mov     cs:old_sp, sp

	mov     ax, cs:appl_ds
	mov     es, ax
	mov     ds, ax

	mov     ss, ax
	mov     sp, offset DGROUP:interrupt_stack_top


go_user	label near
public go_user

	inc     CS:user_routine_on

	sti
	call    _TimeOut

	cli
	dec     CS:user_routine_on

	mov     bx, CS:soundDelay
	neg     bx
	cmp     bx, ax
	jb      clk_delay_ok 

	mov     CS:soundDelay, 0
	jmp     go_user

clk_delay_ok	label near
	add     CS:soundDelay, ax 

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
