/* *   Head.S includes the startup code for the protected mode kernel part  *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   **************************************************************************/
#include "../include/prtcm.h"
/* 	
    This part of the kernel is run after the protected mode is executed.
    These routines will make the environment for the C language part of
    the kernel.  
*/
	.globl	st_32
st_32:
	cli
	cld
	movl	$(__KERNEL_DS),%eax
	movl	%ax,%ds
	movl	%ax,%es
	movl	%ax,%fs
	movl	%ax,%gs
	movl	%ax,%ss
	movl	$(__K_STACK),%esp
	movl	%esp,%ebp
	call	put_done
	
	call	idt_32			/* make the 32 bit IDT */
	call	gdt_32			/* make the 32 bit GDT */

	ljmp	$(__KERNEL_CS),$1f
1:	movl	$(__KERNEL_DS),%eax	/* reload all the segment registers */
	movl	%ax,%ds			/* after changing gdt. */
	movl	%ax,%es
	movl	%ax,%fs
	movl	%ax,%gs
	movl	$(__KERNEL_SS),%eax
	movl	%ax,%ss
	movl	$(__K_STACK),%esp
	movl	%esp,%ebp
	call	put_done

	jmp	main

hng4:	jmp	hng4		/*The code should never return here, but if it
				 *does, we'll wait for it */
/*==========================================================================*
 *-------------------------------ROUTINES-----------------------------------*
 *==========================================================================*/
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$__PRINT_PANIC__$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
/*  void print_panic(unsigned char, unsignedlong stroff)  */
PRINT_PANIC_ARG	=	4 + 4 + 4
/*                     edx esi eip  */
.globl print_panic
print_panic:
	pushl	%esi
	pushl	%edx
	
	call	scroll_screen_up
	movl	PRINT_PANIC_ARG(%esp),%esi
	movl	PRINT_PANIC_ARG+4(%esp),%edx
	movb	%dl,%dh
	call	print_msg
	
	popl	%edx
	popl	%esi
	ret
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$__PRINTK__$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
/*  void printk(unsignedlong stroff)  */
PRINTK_ARG	=	4 + 4 + 4
/*                     edx esi eip  */
.globl	printk
printk:
	pushl	%esi
	pushl	%edx
	
	call	scroll_screen_up
	movl	PRINTK_ARG(%esp),%esi
	movb	$7,%dh
	call	print_msg
	
	popl	%edx
	popl	%esi
	ret
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$__PUT_DONE__$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
/* void put_done(void) */
.globl put_done
put_done:
	push	%eax
	push	%edi
	push	%es	

	movl	$(__K_VIDMEM),%eax
	movl	%ax,%es

	movl	$0xF96,%edi     
	movl	$0x026F0264,%eax	
	movl	%eax,%es:(%edi)	
					/* Write the 'done' string */
	addl	$4,%edi		
	movl	$0x0265026E,%eax	
	movl	%eax,%es:(%edi)
	
	pop	%es
	pop	%edi
	pop	%eax	
	ret
	
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$__SCROLL_SCREEL_UP__$$$$$$$$$$$$$$$$$$$$$$$$$$*/
.globl scroll_screen_up
scroll_screen_up:
	pushl	%eax
	pushl	%ecx
	pushl	%esi
	pushl	%edi
	pushl	%ds
	pushl	%es

	cld
	movl	$(__K_VIDMEM),%eax
	movl	%ax,%ds
	movl	%ax,%es
	movl	$160,%esi
	xorl	%edi,%edi
	movl	$960,%ecx
	rep	
	movsl
	
	movl	$40,%ecx
	xorl	%eax,%eax
	rep	
	stosl
	
	popl	%es
	popl	%ds 
	popl	%edi 
	popl	%esi 
	popl	%ecx 
	popl	%eax 
	ret

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$__PRINT_MSG__$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
/* esi = msg offset , dh = color */
    .globl print_msg
print_msg:
	pushl	%eax
	pushl	%edi
	pushl	%es
	
	movl	$(__K_VIDMEM),%eax
	movl	%ax,%es
	movl	$3840,%edi
pk_lp1:	
	movb	(%esi),%al
	cmpb	$0,%al
	jz	pk_lp1_end
	movb	%dh,%ah
	movw	%ax,%es:(%edi)
	inc	%esi
	add	$2,%edi
	cmpl	$4000,%edi
	jnz	pk_lp1
pk_lp1_end:
	popl	%es
	popl	%edi
	popl	%eax
	ret	

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$__MK_IDT__$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
/* void mk_idt(unsignedchar num, unsignedlong offset, unsignedint selector) */
IDT_TYPE	=	0x8E00
MK_IDT_ARG	= 4 + 4 + 4 + 4  /*       num   offset selector        */
/*		 edi ebx eax eip           4  +   4   +   4            */
.globl	mk_idt
mk_idt:
	pushl	%eax
	pushl	%ebx
	pushl	%edi
	
	movl	MK_IDT_ARG(%esp),%eax		/*Load num*/
	movl	$8,%ebx
	mull	%ebx
	movl	%eax,%edi
	
	movl	MK_IDT_ARG+4(%esp),%eax		/*Load offset*/
	movl	%eax,(%edi)
	addl	$2,%edi
	movw	(%edi),%ax
		
	movl	MK_IDT_ARG+4+4(%esp),%ebx	/*Load	selector*/
	movw	%bx,(%edi)
	addl	$2,%edi
	
	movw	$IDT_TYPE,(%edi)
	addl	$2,%edi
	
	movw	%ax,(%edi)
	
	popl	%edi	
	popl	%ebx
	popl	%eax
	ret	
/*==========================================================================*
 *---------This routine will handle the exceptions for a while--------------*
 *==========================================================================*/

exceptions:
	movl	$140,%eax
	pushl	%eax
	movl	$(kmsg_exc),%eax
	pushl	%eax
	call	print_panic	
	addl	$8,%esp

	movl	$15,%eax
	pushl	%eax
	movl	$(kmsg_exc1),%eax
	pushl	%eax
	call	print_panic	
	addl	$8,%esp

	movl	$15,%eax
	pushl	%eax
	movl	$(kmsg_exc2),%eax
	pushl	%eax
	call	print_panic	
	addl	$8,%esp
	
	movl	$15,%eax
	pushl	%eax
	movl	$(kmsg_exc3),%eax
	pushl	%eax
	call	print_panic	
	addl	$8,%esp

	movl	$15,%eax
	pushl	%eax
	movl	$(kmsg_exc4),%eax
	pushl	%eax
	call	print_panic	
	addl	$8,%esp

	movl	$15,%eax
	pushl	%eax
	movl	$(kmsg_exc5),%eax
	pushl	%eax
	call	print_panic	
	addl	$8,%esp

ex_end:	jmp	ex_end
	iret

/*==========================================================================*
 *------------------------These are main routines---------------------------*
 *------!!!! : They don't save registers-(excepted  segment regs)-----------*
 *==========================================================================*/
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$__IDT_32__$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
idt_32:
	movl	$(kmsg_1),%eax              /* Make the IDT */
	pushl	%eax
	call	printk	
	addl	$4,%esp
	xorl	%ecx,%ecx	
m_lp1:	
	movl	$0x10,%eax
	pushl	%eax	
	movl	$(exceptions),%eax
	pushl	%eax
	pushl	%ecx
	call	mk_idt
	addl	$12,%esp
	inc	%ecx
	cmp	$19,%ecx
	jnz	m_lp1

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%ds
	movl	%ax,%es
	lidt	idt_addr
	call	put_done
	ret
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$__GDT_32__$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
gdt_32:
	movl	$(kmsg_2),%eax              /* Make the GDT */
	pushl	%eax
	call	printk	
	addl	$4,%esp

	movl	$(__GDT_ADDR),%edi
	movl	$gdt_new,%esi	
	cld
	movl	$14,%ecx
	rep	
	movsl

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%ds
	movl	%ax,%es
	lgdt	gdt_addr	
	ret			
/*==========================================================================*
 *----------------------------------DATA------------------------------------*
 *==========================================================================*/
kmsg_1:
 .ascii "Setting up IDT table..."
 .byte 0

kmsg_2:
 .ascii "Setting up GDT table..."
 .byte 0


kmsg_exc:
 .ascii "KERNEL PANIC:                      Low level exception"
 .byte 0

kmsg_exc1:
 .ascii "     Ohh !!   Sorry, but you should never get this error. There is a really  "
 .byte 0
                  
kmsg_exc2:
 .ascii "     big problem  becouse this means that someting isn't OK at the  kernel.  "
 .byte 0
                                                                                   
kmsg_exc3:
 .ascii "     If there is a newer version of the kernel out then see its description "
 .byte 0
                                                                                      
kmsg_exc4:
 .ascii "     and if there is an low level exception  fixed, then get the new kernel "
 .byte 0
                  
kmsg_exc5:
 .ascii "     else please write me the problem to kozmik@email.com .        Thanks ! "
 .byte 0

idt_addr:
	.word 0x180 	/* IDT size */
	.long __IDT_ADDR/* IDT is at 0x00:0x00 */ 

gdt_addr:
	.word 0x5800	/* GDT size */
	.long __GDT_ADDR/* GDT is at 0x00:0x200 */ 


gdt_new:
	.word	0,0,0,0		/* dummy */

	.word	0,0,0,0		/* unused */

	.word	0xFFFF		/* 4Gb - (0x100000*0x1000 = 4Gb 	*/
	.word	0x0000		/* base address=0			*/
	.word	0x9A00		/* code read/exec			*/
	.word	0x00CF	      	/*granularity=4096, 386 		*/
	
	.word	0xFFFF		/* 4Gb - (0x100000*0x1000 = 4Gb)	*/
	.word	0x0000		/* base address=0			*/
	.word	0x9200		/* data read/write			*/
	.word	0x00CF		/* granularity=4096, 386		*/
	
	.word	0xFFFF		/* 4Gb - (0x100000*0x1000 = 4Gb)	*/
	.word	0x8000		/* base address=0			*/
	.word	0x920B		/* data read/write			*/
	.word	0x00CF		/* granularity=4096, 386		*/

	.word	0xFFFF		/* 64K kernel stack			*/
	.word	0x8000		/* base address=0			*/
	.word	0x9209		/* data read/write			*/
	.word	0x0040		/* granularity=byte, 386		*/

