
    BOOT_SEG	=	0x07C0	! BOOT_SEG:0x0000 is where this code is loaded
    PROG_SEG	=	0x9000	! This is where the BOOT_SEG will be copyed
    D_BUFF	=	0x8000	! The data buffer for the floppy
    SYS_SEG	=	0x1000	! this is where the system will be loaded
    CYL_RD	=	20	! number of cylinders to read

.globl _main
_main:

	mov	ax,#BOOT_SEG
	mov	ds,ax
	mov	ax,#PROG_SEG
	mov	es,ax
	mov	cx,#256
	sub	si,si
	sub	di,di
	cld
	rep
	movsw
	jmpi	go,PROG_SEG

	    
go:	mov	di,#0xF000	! loader stack size

	mov	ax,#PROG_SEG
	mov	es,ax
	mov	ds,ax
	mov	ss,ax		! put stack at PROG_SEG:0xF000.
	mov	sp,di

	mov 	bp,# msg1		! Print the message	
	mov     cx,# msg1_end - msg1
	call	print	
	
	xor	ah,ah			! reset FDC 
	xor	dl,dl
	int 	0x13	

_loop:	
	call	msegs
	call	read0
	call	m_away			! Mov data buffer to memory
	mov	ah,# 0x0E		! Read head 0 and write a . to screen
	mov	al,#46
	int	0x10

	call	msegs
	call	read1
	call	m_away			! Mov data buffer to memory
	mov	ah,# 0x0E		! Read head 0 and write a . to screen
	mov	al,#46
	int	0x10
	
	call	msegs
	inc	mv_cyl
	
	mov	ch,mv_cyl
	mov	al,# CYL_RD		! check if we are ready, and if
	cmp	al,ch			! we finished, jump to system
	jz	gbye
	
	jmp	_loop	
	
rerr:	call	msegs
	mov 	bp,# msg2		! Print the message	
	mov     cx,# msg2_end - msg2
	call	print	
	jmp	_hng

gbye:	jmpi	512,SYS_SEG		! go to the loaded system	

_hng:	jmp	_hng			! Hang for waiting for CTR+ALT+DEL

m_away: 
	call	msegs

	mov	ax,mv_seg
	mov	es,ax
	mov	ax,# D_BUFF
	mov	ds,ax
	mov	cx,# 0xFFFF
	xor	di,di
	xor	si,si
_2lp:	
	seg	ds
	mov	al,(si)
	seg	es
	mov	(di),al
	inc	di
	inc	si
	loop	_2lp

	call	msegs
	add	mv_seg,# 0x240
	ret

read0:	mov	ch,mv_cyl
	mov	ax,# D_BUFF
	mov	es,ax
	xor	bx,bx
	mov	ah,#0x02                ! service 2, number of sectors   }
	mov	dh,#0
	mov     dl,#0			! drive number                   }
	mov	cl,#1                   ! sector number                  }
	mov	al,#18                  ! Read 18 sectors at one time	 }
	int	0x13			! read it			 }

	jc	rerr
	ret

read1:	mov	ch,mv_cyl
	mov	ax,# D_BUFF
	mov	es,ax
	xor	bx,bx
	mov	ah,#0x02                ! service 2, number of sectors   }
	mov	dh,#1
	mov     dl,#0			! drive number                   }
	mov	cl,#1                   ! sector number                  }
	mov	al,#18                  ! Read 18 sectors at one time	 }
	int	0x13			! read it			 }

	jc	rerr
	ret


msegs:	mov	ax,# PROG_SEG		! Mov datasegment to boot  
	mov 	ds,ax
	mov	es,ax
	ret

print:	xor	dl,dl
	mov	dh,mv_line
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
	
	inc	mv_line			! put a new line
		
	ret

msg1:
 .byte 13,10
 .ascii "Starting to load"
msg1_end:

msg2:
 .byte 13,10
 .ascii "Floppy error"
msg2_end:

mv_seg:
 .word SYS_SEG

mv_cyl:
 .byte 0
mv_line:
 .byte 23


  