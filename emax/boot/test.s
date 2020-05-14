
    TEST_ADDR 		=	0x7c
    PROGSEG		= 	0x1020	! the program is loaded to here
    VIDMEM		=	0xB800	! the address of video memory	
    __KERNEL_CS		=	0x10	! the address of kernel segment in GDT

.globl _main
_main:
	mov	ax,#PROGSEG
	mov	ds,ax
	mov	es,ax

	push dx
	mov dx,#0x3f2
	xor al, al
	outb
	pop dx

	call	prdone		! Print that loading is done

! The next part of assembly program will check that the code is running on 
! an i386 compatible processor	     	

	pushf
	pushf
	pop	ax		! Test the NT (nested task) bit
	xor	ah,# 0x40
	push	ax
	popf
	pushf
	pop	bx
	popf
	cmp	ax,bx
	jne	NT_err

!&&&&&&&&&&&&&&&&&&&&&&&&&&&__ROUTINES__&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&	
	jmpi	bjmp,PROGSEG

prdone:  
	xor	bh,bh
	mov	dl,#75
	mov	dh,#24
	mov	ah,#0x02
	int	0x10
	
	mov	cx,#1
	mov	bl,#2
	
	mov	al,#100		!d
	mov	ah,#0x09		! write string, move cursor
	int	0x10

	xor	bh,bh
	mov	dl,#76
	mov	dh,#24
	mov	ah,#0x02
	int	0x10
	
	mov	al,#111		!o
	mov	ah,#0x09
	int	0x10

	xor	bh,bh
	mov	dl,#77
	mov	dh,#24
	mov	ah,#0x02
	int	0x10
	
	mov	al,#110		!n
	mov	ah,#0x09
	int	0x10
	
	xor	bh,bh
	mov	dl,#78
	mov	dh,#24
	mov	ah,#0x02
	int	0x10

	mov	al,#101		!e
	mov	ah,#0x09
	int	0x10
	ret

NT_err:					! if the processor isn't i386 
					! compatible
	mov	dl,#0
	mov	dh,#23
	mov	cx,# msg4_end - msg4
	mov	bx,#0x0004		! page 0, attribute 2 (green)
	mov	bp,#msg4
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
_hng3:	jmp	_hng3

print:	mov	ax,#PROGSEG
	mov	es,ax
	
	push	cx
	call	scrup
	pop	cx
	
	mov	dl,#0
	mov	dh,#23
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

	ret	

scrup:
	xor	si,si
	xor	di,di
	mov	ax,#VIDMEM
	add	ax,#0xA
	mov	ds,ax
	mov	ax,#0x8000
	mov	es,ax
	mov	cx,#1920
	sub	si,si
	sub	di,di
	cld
	rep
	movsw

	xor	si,si
	xor	di,di
	mov	ax,#0x8000
	mov	ds,ax
	mov	ax,#VIDMEM
	mov	es,ax
	mov	cx,#1920
	sub	si,si
	sub	di,di
	cld
	rep
	movsw
	mov	ax,#VIDMEM
	add	ax,#0x0F0
	mov	es,ax
	xor	di,di
	xor	ax,ax
	mov	cx,#80
lp_3:
	seg	es
	mov	(di),ax
	add	di,#2
	loop	lp_3
	
	mov	ax,#PROGSEG
	mov	es,ax
	ret

!&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
bjmp:
dtc_end:
	mov	cx,# msg9_end - msg9
	mov	bp,#msg9
	call	print

	jmp	protectm	
!<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<__ROUTINES__>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
pchar:	xor	bh,bh
	mov	ah,# 0x0E		!  write a num to screen
	add 	al,# 48
	int	0x10
	ret

numki:	xor	dx,dx
	mov	bx,# 10000
	div	bx
	call	pchar
		
	mov	ax,dx
	xor	dx,dx
	mov	bx,# 1000
	div	bx
	call	pchar

	mov	ax,dx
	xor	dx,dx
	mov	bx,# 100
	div	bx
	call	pchar
	
	mov	ax,dx
	xor	dx,dx
	mov	bx,# 10
	div	bx
	call	pchar
	
	mov	al,dl
	call	pchar				
	ret

!$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

protectm:

        push	cs
	call	default_switch

	cli			! disable interrupts

	mov	ax,#PROGSEG
	mov	ds,ax

	lidt	idt_48		! load idt with 0,0
	lgdt	gdt_48		! load gdt with whatever appropriate

! that was painless, now we enable A20

	call	empty_8042
	mov	al,#0xD1		! command write
	out	#0x64,al
	call	empty_8042
	mov	al,#0xDF		! A20 on
	out	#0x60,al
	call	empty_8042

! wait until a20 really *is* enabled; it can take a fair amount of
! time on certain systems; Toshiba Tecras are known to have this
! problem.  The memory location used here is the int 0x1f vector,
! which should be safe to use; any *unused* memory location < 0xfff0
! should work here.  

	push	ds
	xor	ax,ax			! segment 0x0000
	mov	ds,ax
	dec	ax			! segment 0xffff (HMA)
	mov	gs,ax
	mov	bx,[TEST_ADDR]		! we want to restore the value later
a20_wait:
	inc	ax
	mov	[TEST_ADDR],ax
	seg	gs
	cmp	ax,[TEST_ADDR+0x10]
	je	a20_wait		! loop until no longer aliased
	mov	[TEST_ADDR],bx		! restore original value
	pop	ds
		
! make sure any possible coprocessor is properly reset..

	xor	ax,ax
	out	#0xf0,al
	call	delay
	out	#0xf1,al
	call	delay

! well, that went ok, I hope. Now we have to reprogram the interrupts :-(
! we put them right after the intel-reserved hardware interrupts, at
! int 0x20-0x2F. There they won't mess up anything. Sadly IBM really
! messed this up with the original PC, and they haven't been able to
! rectify it afterwards. Thus the bios puts interrupts at 0x08-0x0f,
! which is used for the internal hardware interrupts as well. We just
! have to reprogram the 8259's, and it isn't fun.

	mov	al,#0x11		! initialization sequence
	out	#0x20,al		! send it to 8259A-1
	call	delay
	out	#0xA0,al		! and to 8259A-2
	call	delay
	mov	al,#0x20		! start of hardware int's (0x20)
	out	#0x21,al
	call	delay
	mov	al,#0x28		! start of hardware int's 2 (0x28)
	out	#0xA1,al
	call	delay
	mov	al,#0x04		! 8259-1 is master
	out	#0x21,al
	call	delay
	mov	al,#0x02		! 8259-2 is slave
	out	#0xA1,al
	call	delay
	mov	al,#0x01		! 8086 mode for both
	out	#0x21,al
	call	delay
	out	#0xA1,al
	call	delay
	mov	al,#0xFF		! mask off all interrupts for now
	out	#0xA1,al
	call	delay
	mov	al,#0xFB		! mask all irq's but irq2 which
	out	#0x21,al		! is cascaded

! Well, that certainly wasn't fun :-(. Hopefully it works, and we don't
! need no steenking BIOS anyway (except for the initial loading :-).
! The BIOS routine wants lots of unnecessary data, and it's less
! "interesting" anyway. This is how REAL programmers do it.
!
! Well, now's the time to actually move into protected mode. To make
! things as simple as possible, we do no register set-up or anything,
! we let the GNU-compiled 32-bit programs do that. We just jump to
! absolute address 0x1000 (or the loader supplied one),
! in 32-bit protected mode.
!
! Note that the short jump isn't strictly needed, although there are
! reasons why it might be a good idea. It won't hurt in any case.
!
	mov	ax,#1		! protected mode (PE) bit
	lmsw	ax		! This is it!
	jmp	flush_instr
flush_instr:
	xor	bx,bx		! Flag to indicate a boot
	cli

! NOTE: For high loaded big kernels we need a
!
!	but we yet haven't reloaded the CS register, so the default size 
!	of the target offset still is 16 bit.
!       However, using an operant prefix (0x66), the CPU will properly
!	take our 48 bit far pointer. (INTeL 80386 Programmer's Reference
!	Manual, Mixing 16-bit and 32-bit code, page 16-6)
	db	0x66,0xea	! prefix + jmpi-opcode
	dd	0x11000		! will be set to 0x100000 for big kernels
	dw	__KERNEL_CS


!>>>>>>>>>>>>>>>>>>>>>>>>____SWITCH_TO_PROTECTED MODE____<<<<<<<<<<<<<<<<<<<<<<		



!$$$$$$$$$$$$$$$$$___LAST_ROUTINE_OF_SYSTEM____$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

_hng2:	jmp	_hng2		! End of the system when its halted

!<<<<<<<<<<<<<<<<_____WAITING_FOR__CTRL+ALT+DEL_____>>>>>>>>>>>>>>>>>>>>>>>>>>	

delay:
	.word	0x00eb			! jmp $+2
	ret

empty_8042:
       push    ecx
       mov     ecx,#0xFFFFFF

empty_8042_loop:
       dec     ecx
       jz      empty_8042_end_loop

	call	delay
	in	al,#0x64	! 8042 status port
	test	al,#1		! output buffer?
	jz	no_output
	call	delay
	in	al,#0x60	! read it
	jmp	empty_8042_loop
no_output:
	test	al,#2		! is input buffer full?
	jnz	empty_8042_loop	! yes - loop
empty_8042_end_loop:
        pop     ecx
	ret

! This is the default real mode switch routine.
! to be called just before protected mode transition

default_switch:
	cli			! no interrupts allowed !
	mov	al,#0x80	! disable NMI for the bootup sequence
	out	#0x70,al
	retf

mGDT:
	.word	0,0,0,0		! dummy

	.word	0,0,0,0		! unused

	.word	0xFFFF		! 4Gb - (0x100000*0x1000 = 4Gb)
	.word	0x0000		! base address=0
	.word	0x9A00		! code read/exec
	.word	0x00CF		! granularity=4096, 386 (+5th nibble of limit)

	.word	0xFFFF		! 4Gb - (0x100000*0x1000 = 4Gb)
	.word	0x0000		! base address=0
	.word	0x9200		! data read/write
	.word	0x00CF		! granularity=4096, 386 (+5th nibble of limit)

	.word	0xFFFF		! 4Gb - (0x100000*0x1000 = 4Gb)
	.word	0x8000		! base address=0
	.word	0x920B		! data read/write
	.word	0x00CF		! granularity=4096, 386 (+5th nibble of limit)

idt_48:
	.word	0			! idt limit=0
	.word	0,0			! idt base=0L

gdt_48:
	.word	0x40		! gdt limit=64, 5 GDT entries
	.word	512+mGDT,0x1	! gdt base = 0x1xxxx

msg3:
 .byte 13,10    
 .ascii "done"
msg3_end:

msg4:
 .byte 13,10    
 .ascii "KERNEL PANIC:            the CPU isn't i386 compatible"
msg4_end:

msg9:
 .byte 13,10    
 .ascii "Executeing 32-bit protected mode..."
msg9_end:

 