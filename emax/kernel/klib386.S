/* *   klib386.s includes the routines for the C language kernel          *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
/*  These routines only preserve the registers, witch are importent for the
 *  C compiler ( ebx, esi, edi, ebp, esp, ds, es, fs, gs, and the direction
 *  bit in the flag register )
 */

#include "../include/prtcm.h"
/*===========================================================================*/
/*                                 DEC_HEX                                   */
/*===========================================================================*/
/* void dec_hex(unsignedint dec,char * hex) */
DEC_HEX_ARG	=	4 + 4 + 4 + 4	/*  dec hex  */
/*			ds edi ebx eip       4 + 4   */
.globl dec_hex
dec_hex:
	pushl	%ebx
	pushl	%edi
	pushl	%es
	
	movl	$(__KERNEL_DS),%eax
	movl	%ax,%ds
	movl	DEC_HEX_ARG(%esp),%ebx		/*Load dec*/
	movl	DEC_HEX_ARG+4(%esp),%esi	/*Load hex*/
	movb	%bh,%al
	call	calc_num
	movw	%cx,(%esi)
	addl	$2,%esi
	movb	%bl,%al	
	call	calc_num
	movw	%cx,(%esi)
	addl	$2,%esi
	xorl	%eax,%eax
	movb	%al,(%esi)
	
	popl	%es
	popl	%edi
	popl	%ebx
	ret
calc_num:		/*IN: al = byte */
	pushl	%ebx
	
	xorb	%ah,%ah	
	movb	$16,%bl
	divb	%bl
	movb	%al,%cl
	addb	$48,%cl
	movb	%ah,%ch
	addb	$48,%ch
	cmpb	$57,%ch
	jna	_pr_1		
	addb	$7,%ch
_pr_1:	
	cmpb	$57,%cl
	jna	_pr_2		
	addb	$7,%cl
_pr_2:
	popl	%ebx
	ret		/*OUT: cx = string */

/*===========================================================================*/
/*                                  PR_XY                                    */
/*===========================================================================*/
/* void pr_xy(unsignedchar x,unsignedchar y,unsignedchar color,char * str) */
PR_XY_ARG	=	4 + 4 + 4 + 4 + 4 + 4	/* x   y  color str */
/*		        ds  es esi edi ebx eip     4 + 4 +  4 +  4*/	
.globl pr_xy
pr_xy:
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	pushl	%es
	pushl	%ds

	movl	$(__K_VIDMEM),%eax
	movl	%ax,%es
	movl	$(__KERNEL_DS),%eax
	movl	%ax,%ds
	
	movl	PR_XY_ARG+4(%esp),%eax		/*Load y*/	
	movl	$160,%ebx
	mulb	%bl
	movl	PR_XY_ARG(%esp),%ebx		/*Load x*/
	addl	%ebx,%eax
	addl	%ebx,%eax
	movl	%eax,%edi
	
	movl	PR_XY_ARG+4+4(%esp),%eax	/*Load color*/	
	movb	%al,%ah
	movl	PR_XY_ARG+4+4+4(%esp),%esi	/*Load str*/
lp_pr_xy:
	movb	(%esi),%al	
	movw	%ax,%es:(%edi)
	inc	%esi
	addl	$2,%edi
	cmpb	$0,%al
	jnz	lp_pr_xy
	
	popl	%ds
	popl	%es
	popl	%esi
	popl	%edi
	popl	%ebx
	ret
/*===========================================================================*/
/*                                  HALT                                     */
/*===========================================================================*/
/* void halt() */
.globl halt
halt:
	cli
	movl	$(__KERNEL_DS),%eax
	movl	%eax,%ds
	movl	$(kmsg_halt),%eax          /* SYSTEM HALTED */
	pushl	%eax
	call	printk	
	addl	$4,%esp
lp1:	jmp	lp1
/*===========================================================================*/
/*                                 RESET                                     */
/*===========================================================================*/
/* void reset() */
.globl reset
reset:
	movl	$(__KERNEL_DS),%eax
	movl	%eax,%ds
	movl	$(kmsg_reboot),%eax          /* SYSTEM REBOOTING.. */
	pushl	%eax
	call	printk	
	addl	$4,%esp
	lidt	idt_zero
	.byte	0xCC		/*as can't compile 'int 3'*/
/*===========================================================================*/
/*                              MEM_COPY                                     */
/*===========================================================================*/
/* void mem_copy(unsignedlong len,unsignedlong src,unsignedlong dest) */
MEM_COPY_ARG	= 4 + 4 + 4 + 4 + 4  /*         len src dest    */
/*		 esi edi  es  ds eip             4  + 4 + 4     */
.globl mem_copy					/* len = [dword] */
mem_copy:
	pushl	%ds
	pushl	%es
	pushl	%edi
	pushl	%esi

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%ds
	movl	%ax,%es
	movl	MEM_COPY_ARG(%esp),%ecx		/*Load len*/
	movl	MEM_COPY_ARG+4(%esp),%esi	/*Load src*/
	movl	MEM_COPY_ARG+4+4(%esp),%edi	/*Load dest*/
	cld
	rep
	movsl

	popl	%esi
	popl	%edi
	popl	%es
	popl	%ds
	ret
/*===========================================================================*/
/*                              MEM_STORE                                    */
/*===========================================================================*/
/* void mem_store(unsignedlong len,unsignedlong dest,unsignedlong chr) */
MEM_STORE_ARG	= 4 + 4 + 4  /*         len  dest chr    */
/*		 edi  es eip             4  + 4 +  4     */
.globl mem_store				/* len = [dword] */
mem_store:
	pushl	%es
	pushl	%edi

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	MEM_STORE_ARG(%esp),%ecx	/*Load len*/
	movl	MEM_STORE_ARG+4(%esp),%edi	/*Load dest*/
	movl	MEM_STORE_ARG+4+4(%esp),%eax	/*Load chr*/
	cld
	rep
	stosl

	popl	%edi
	popl	%es
	ret
/*===========================================================================*/
/*                               MEM_VSL                                     */
/*===========================================================================*/
/* void mem_vsl(unsignedlong dest,unsignedlong chr) */
MEM_VSL_ARG	= 4 + 4 + 4  /*         dest chr   */
/*		 edi  es eip             4  + 4    */
.globl mem_vsl				
mem_vsl:
	pushl	%edi
	pushl	%es

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	MEM_VSL_ARG(%esp),%edi		/*Load dest*/
	movl	MEM_VSL_ARG+4(%esp),%eax	/*Load chr*/
	movl	%eax,%es:(%edi)
	
	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                                MEM_VLL                                    */
/*===========================================================================*/
/* unsignedlong mem_vll(unsignedlong dest) */
MEM_VLL_ARG	= 4 + 4 + 4  /*    dest  */
/*		 edi  es eip        4    */
.globl mem_vll				
mem_vll:
	pushl	%edi
	pushl	%es

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	MEM_VLL_ARG(%esp),%edi		/*Load dest*/
	xorl	%eax,%eax
	movl	%es:(%edi),%eax
	
	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                                MEM_VS                                     */
/*===========================================================================*/
/* void mem_vs(unsignedlong dest,unsignedint chr) */
MEM_VS_ARG	= 4 + 4 + 4  /*         dest chr   */
/*		 edi  es eip             4  + 4    */
.globl mem_vs				
mem_vs:
	pushl	%edi
	pushl	%es

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	MEM_VS_ARG(%esp),%edi		/*Load dest*/
	movl	MEM_VS_ARG+4(%esp),%eax		/*Load chr*/
	movw	%ax,%es:(%edi)
	
	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                                MEM_VL                                     */
/*===========================================================================*/
/* unsignedint mem_vl(unsignedlong dest) */
MEM_VL_ARG	= 4 + 4 + 4  /*    dest  */
/*		 edi  es eip        4    */
.globl mem_vl				
mem_vl:
	pushl	%edi
	pushl	%es

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	MEM_VL_ARG(%esp),%edi		/*Load dest*/
	xorl	%eax,%eax
	movw	%es:(%edi),%ax
	
	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                                MEM_VSB                                    */
/*===========================================================================*/
/* void mem_vsb(unsignedlong dest,unsignedint chr) */
MEM_VSB_ARG	= 4 + 4 + 4  /*         dest chr   */
/*		 edi  es eip             4  + 4    */
.globl mem_vsb				
mem_vsb:
	pushl	%edi
	pushl	%es

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	MEM_VSB_ARG(%esp),%edi		/*Load dest*/
	movl	MEM_VSB_ARG+4(%esp),%eax	/*Load chr*/
	movb	%al,%es:(%edi)
	
	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                                MEM_VLB                                    */
/*===========================================================================*/
/* unsignedint mem_vlb(unsignedlong dest)*/
MEM_VLB_ARG	= 4 + 4 + 4  /*    dest  */
/*		 edi  es eip        4    */
.globl mem_vlb				
mem_vlb:
	pushl	%edi
	pushl	%es

	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	MEM_VLB_ARG(%esp),%edi		/*Load dest*/
	xorl	%eax,%eax
	movb	%es:(%edi),%al
	
	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                              OUT_BYTE                                     */
/*===========================================================================*/
/* void out_byte(unsignedint port,unsignedchar chr) */
OUT_BYTE_ARG	=  4  /*         port chr    */
/*		  eip             4  + 4     */
.globl out_byte
out_byte:
	movl	OUT_BYTE_ARG(%esp),%edx
	movl	OUT_BYTE_ARG+4(%esp),%eax
	outb	%al,%dx
	ret
/*===========================================================================*/
/*                               IN_BYTE                                     */
/*===========================================================================*/
/* unsignedchar in_byte(unsignedint port) */
IN_BYTE_ARG	=  4  /*         port     */
/*		  eip             4       */
.globl in_byte
in_byte:
	movl	IN_BYTE_ARG(%esp),%edx
	xorl	%eax,%eax
	inb	%dx,%al
	ret
/*===========================================================================*/
/*                              OUT_WORD                                     */
/*===========================================================================*/
/* void out_word(unsignedint port,unsignedint chr) */
OUT_WORD_ARG	=  4  /*         port chr    */
/*		  eip             4  + 4     */
.globl out_word
out_word:
	movl	OUT_WORD_ARG(%esp),%edx
	movl	OUT_WORD_ARG+4(%esp),%eax
	outw	%ax,%dx
	ret
/*===========================================================================*/
/*                               IN_WORD                                     */
/*===========================================================================*/
/* unsignedint in_byte(unsignedint port) */
IN_WORD_ARG	=  4  /*         port     */
/*		  eip             4       */
.globl in_word
in_word:
	movl	IN_WORD_ARG(%esp),%edx
	xorl	%eax,%eax
	inw	%dx,%ax
	ret
/*===========================================================================*/
/*                               IN_LONG                                     */
/*===========================================================================*/
/* unsignedint in_long(unsignedint port) */
IN_LONG_ARG	=  4  /*         port     */
/*		  eip             4       */
.globl in_long
in_long:
	movl	IN_LONG_ARG(%esp),%edx
	xorl	%eax,%eax
	inl	%dx,%eax
	ret
/*===========================================================================*/
/*                               PORT_READ                                   */
/*===========================================================================*/
/* void port_read(unsignedint port,unsignedlong dest,unsignedlong count) */
PORT_READ_ARG	= 4 + 4 + 4  /*         port dest count   */
/*		  es edi eip             4  +  4 +  4     */
.globl port_read				/* count = [word] */
port_read:
	cld
	pushl	%edi
	pushl	%es
	
	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	PORT_READ_ARG(%esp),%edx	/*Load port*/
	movl	PORT_READ_ARG+4(%esp),%edi	/*Load dest*/
	movl	PORT_READ_ARG+4+4(%esp),%ecx	/*Load count*/
	rep
	insw

	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                            PORT_READ_BYTE                                 */
/*===========================================================================*/
/* void port_read_byte(unsignedint port,unsignedlong dest,unsignedlong count)*/
PORT_READ_BYTE_ARG	= 4 + 4 + 4  /*         port dest count   */
/*			  es edi eip             4  +  4 +  4     */
.globl port_read_byte				/* count = [byte] */
port_read_byte:
	cld
	pushl	%edi
	pushl	%es
	
	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	PORT_READ_BYTE_ARG(%esp),%edx		/*Load port*/
	movl	PORT_READ_BYTE_ARG+4(%esp),%edi		/*Load dest*/
	movl	PORT_READ_BYTE_ARG+4+4(%esp),%ecx	/*Load count*/
	rep
	insb

	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                               PORT_WRITE                                  */
/*===========================================================================*/
/* void port_write(unsignedint port,unsignedlong dest,unsignedlong count) */
PORT_WRITE_ARG	= 4 + 4 + 4  /*         port dest count   */
/*		  es edi eip             4  +  4 +  4     */
.globl port_write				/* count = [word] */
port_write:
	cld
	pushl	%edi
	pushl	%es
	
	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	PORT_WRITE_ARG(%esp),%edx	/*Load port*/
	movl	PORT_WRITE_ARG+4(%esp),%edi	/*Load dest*/
	movl	PORT_WRITE_ARG+4+4(%esp),%ecx	/*Load count*/
	rep
	outsw

	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                             PORT_WRITE_BYTE                               */
/*===========================================================================*/
/*void port_write_byte(unsignedint port,unsignedlong dest,unsignedlong count)*/
PORT_WRITE_BYTE_ARG	= 4 + 4 + 4  /*         port dest count   */
/*			  es edi eip             4  +  4 +  4     */
.globl port_write_byte				/* count = [byte] */
port_write_byte:
	cld
	pushl	%edi
	pushl	%es
	
	movl	$(__KERNEL_DS),%eax
	movl	%ax,%es
	movl	PORT_WRITE_BYTE_ARG(%esp),%edx		/*Load port*/
	movl	PORT_WRITE_BYTE_ARG+4(%esp),%edi	/*Load dest*/
	movl	PORT_WRITE_BYTE_ARG+4+4(%esp),%ecx	/*Load count*/
	rep
	outsb

	popl	%es
	popl	%edi
	ret
/*===========================================================================*/
/*                                 AXH                                       */
/*===========================================================================*/
/*unsignedint axh(unsignedlong eax)*/
AXH_ARG =	4    /*        eax */
/*             eip              4  */
.globl axh
axh:
	movl	AXH_ARG(%esp),%eax
	movl	$16,%ecx
lp_axh:
	cld
	rcr	%eax
	loop	lp_axh
	ret
/*===========================================================================*/
/*                                 AXL                                       */
/*===========================================================================*/
/*unsignedint axl(unsigned long eax)*/
AXL_ARG =	4    /*        eax  */
/*             eip              4   */
.globl axl
axl:
	movl	AXL_ARG(%esp),%edx
	movl	$0xFFFF,%eax
	andl	%edx,%eax
	ret
/*===========================================================================*/
/*                                  AH                                       */
/*===========================================================================*/
/*unsignedchar ah(unsignedlong eax)*/
AH_ARG =	4    /*        eax */
/*             eip              4  */
.globl ah
ah:
	movl	AH_ARG(%esp),%edx
	xorl	%eax,%eax
	movb	%dh,%al
	ret
/*===========================================================================*/
/*                                  AL                                       */
/*===========================================================================*/
/*unsignedchar al(unsigned long eax)*/
AL_ARG =	4    /*        eax  */
/*             eip              4   */
.globl al
al:
	movl	AL_ARG(%esp),%edx
	xorl	%eax,%eax
	movb	%dl,%al
	ret
/*===========================================================================*/
/*                                P_INT                                      */
/*===========================================================================*/
/*unsignedlong P_INT(unsigned long *eax)*/
AL_ARG =	4    /*        eax  */
/*             eip              4   */
.globl p_int
p_int:
	movl	AL_ARG(%esp),%eax
	ret
/*===========================================================================*/
/*                                NCT+                                       */
/*===========================================================================*/
/*void nct()*/
.globl nct
nct:	
	push	%es
	push	%edi
	
	movl	$(__KERNEL_CS),%eax
	movl	%ax,%es
	movl	$0x1FFFC,%edi	
	incl	%es:(%edi)	
	
	popl	%edi
	popl	%es
	ret
/*===========================================================================*/
/*                               ENABLE_IRQ                                  */
/*===========================================================================*/
/*void enable_irq(unsignedchar irq)*/
.globl enable_irq
enable_irq:
	movl	4(%esp),%ecx
	pushf
	cli
	movb	$~1,%ah
	rolb	%cl,%ah
	cmpb	$8,%cl
	jae	enable_8
enable_0:
	inb	$0x21
	andb	%ah,%al
	outb	$0x21
	popf
	ret
enable_8:	
	inb	$0xA1
	andb	%ah,%al
	outb	$0xA1
	popf
	ret
/*===========================================================================*/
/*                                  LTR                                     */
/*===========================================================================*/
/*void ltr(unsignedint task)*/
.globl ltr
ltr:
	movl	4(%esp),%eax
	ltr	%ax
	ret
/*===========================================================================*/
/*                                  DATA                                     */
/*===========================================================================*/

kmsg_halt:
 .ascii "                             -- System HALTED --"
 .byte 0

kmsg_reboot:
 .ascii "                              -- Rebooting... --"
 .byte 0

idt_zero:
 .word 0,0,0

