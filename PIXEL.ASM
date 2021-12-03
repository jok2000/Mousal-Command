;
; 640x400 graphics mode -- asm routines
; Generalized to PPERLINE
;

PPERLINE	equ	320
	.MODEL	small
EXTENDED_REG	equ	01ceh
PLANE_SELECT	equ	0b2h
PLANE_MASK	equ	0e1h

	.DATA
	PUBLIC	_image_cont
_image_cont	dd	0


	.CODE
	.386

	PUBLIC _char_cor
_char_cor proc near
; Correct character background on last line
; void char_cor(int c)
	push	bp
	mov		bp,sp
	mov		cx,word ptr [bp + 4]
	push	si
	push	di
	mov		ax,0a000h
	mov		es,ax
	mov		si,PPERLINE*192
	add 	si,cx
	mov		bl,8
cc1:
	mov		di,si
	mov 	bh,8
cc2:
	mov		al,es:[si]
	inc		si
	and		al,al
	jnz		ccok
	mov 	al,2
ccok:
	stosb
	dec		bh
	jnz		cc2
	add 	si,PPERLINE-8
	dec		bl
	jnz		cc1
	pop		di
	pop		si
	pop		bp
	ret
_char_cor endp

;
; given X in CX, Y in DX
;
;	void	pixel(int x, int y, int val);
;
	PUBLIC	_pixel

_pixel	proc	near
	push	bp
	mov	bp,sp
	push	di
	movzx	edx,word ptr [bp + 6]
	mov	eax,PPERLINE
	mul	edx
	movzx	edx,word ptr [bp + 4]
	add	eax,edx
	mov	di,ax
	shr	eax,16
	mov	bh,al
;	cli
;	mov	dx,EXTENDED_REG
;	mov	al,PLANE_SELECT
;	out	dx,al
;	inc	dl
;	in	al,dx
;	mov	ah,al
;	and	ah,PLANE_MASK
;	shl	bh,1
;	or	ah,bh
;	mov	al,PLANE_SELECT
;	dec	dl
;	out	dx,ax
;	sti
	mov	ax,0a000h
	mov	es,ax
	mov	ax,word ptr [bp + 8]
	cmp	ax,-1
	jnz	pixs
	mov	al,byte ptr es:[di]
	mov	ah,0
pixx:
	pop	di
	pop	bp
	ret
pixs:
	mov	ah,byte ptr es:[di]
	stosb
	mov	al,ah
	mov ah,0
	jmp pixx

_pixel	endp

; Compute video page
vpage:	push	eax
	push	bx
	push	dx

	mov	eax,edi		; Fetch screen pointer
	shr	eax,16
	mov	bh,al
	cli
	mov	dx,EXTENDED_REG
	mov	al,PLANE_SELECT
	out	dx,al
	inc	dl
	in	al,dx
	mov	ah,al
	and	ah,PLANE_MASK
	shl	bh,1
	or	ah,bh
	mov	al,PLANE_SELECT
	dec	dl
	out	dx,ax
	sti

	pop	dx
	pop	bx
	pop	eax
	ret

;
; set a graphics mode
;
;	int	init_mode(int mode);
;
	PUBLIC	_init_mode

_init_mode	proc	near
	push	bp
	mov	bp,sp
	mov	ah,0fh
	int	10h
	mov	ah,0
	push	ax
	mov	ax,word ptr [bp + 4]
	mov	ah,0
	int	10h
	pop	ax
	pop	bp
	ret
_init_mode	endp

;
; initialize the palette
;
;	void	init_pal(char *pal,int num);
;
	PUBLIC	_init_pal

_init_pal proc	near
	push	bp
	mov		bp,sp
	push	es
	mov		ax,1012h
	mov		bx,word ptr [bp+6]
	mov		cx,word ptr [bp+8]
	mov		dx,word ptr [bp+4]
	push	ds
	pop		es
	int		10h		; set the palette
	pop		es
	pop		bp
	ret

_init_pal	endp

;;
;; read a key
;;
;;	int	scankey();
;;
	PUBLIC	_scankey

_scankey	proc	near
	mov	ah,01h
	int	16h
	jnz	short _waitkey
	xor	ax,ax
	ret
_scankey	endp

;;
;; wait for a key
;;
;;	int	waitkey()
;;
	PUBLIC	_waitkey

_waitkey	proc	near
	mov	ah,00h
	int	16h
	or	al,al
	jz	short wkey1
	xor	ah,ah
wkey1:	ret
_waitkey	endp

	end

