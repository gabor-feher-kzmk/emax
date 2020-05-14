/* *   memutils.c includes the low level routines for memory management   *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
#include "../include/pis.h"
/*===========================================================================*/
/*                               MSG_COPY                                    */
/*===========================================================================*/
static void msg_copy(unsigned long src,unsigned long dest,unsigned long size)
{
asm("pushl	%edi
     pushl	%esi
    ");
asm("pushl	%0"
	: 
	: "r" (src));
asm("pushl	%0"
	: 
	: "r" (dest));
asm("pushl	%0"
	: 
	: "r" (size));
asm("popl	%ecx
     popl	%edi
     popl	%esi
     cld
     rep
     movsl
    ");
asm("popl	%esi
     popl	%edi
    ");	
		
}
/*===========================================================================*/
/*                               LOAD_PREG                                   */
/*===========================================================================*/
static void load_mreg(unsigned long pn,unsigned long reg,unsigned long data)
{
 extern unsigned long p_head[200];

 mem_str(reg+p_head[pn]+U_SPACE,data);
} 
/*===========================================================================*/
/*                               GET_PREG                                    */
/*===========================================================================*/
static unsigned long get_mreg(unsigned long pn,unsigned long reg)
{
 extern unsigned long p_head[200];

 return mem_load(reg+p_head[pn]+U_SPACE);
} 
/*===========================================================================*/
/*                               MEM_LOAD                                    */
/*===========================================================================*/
static unsigned long mem_load(unsigned long offset)
{
unsigned long lng;
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("popl	%edi\n\t
	     movl	%es:(%edi),%eax\n\t
	     popl	%edi\n\t
	     pushl	%eax	    	
	    ");
	asm("popl	%0"
		: "=r" (lng)
		: );
return lng;
}
/*===========================================================================*/
/*                                MEM_STR                                    */
/*===========================================================================*/
static void mem_str(unsigned long offset,unsigned long data)
{
unsigned long lng;
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("pushl	%0"
		: 
		: "r" (data));
	asm("popl	%eax
	     popl	%edi\n\t
	     movl	%eax,%es:(%edi)\n\t
	     popl	%edi\n\t	    	
	    ");
}
/*===========================================================================*/
/*                               MEM_WLOAD                                    */
/*===========================================================================*/
static unsigned int mem_wload(unsigned long offset)
{
unsigned int lng;
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("popl	%edi\n\t
	     xorl	%eax,%eax\n\t
	     movw	%es:(%edi),%ax\n\t
	     popl	%edi\n\t
	     pushl	%eax	    	
	    ");
	asm("popl	%0"
		: "=r" (lng)
		: );
return lng;
}
/*===========================================================================*/
/*                                MEM_WSTR                                    */
/*===========================================================================*/
static void mem_wstr(unsigned long offset,unsigned int data)
{
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("pushl	%0"
		: 
		: "r" (data));
	asm("popl	%eax
	     popl	%edi\n\t
	     movw	%ax,%es:(%edi)\n\t
	     popl	%edi\n\t	    	
	    ");
}
/*===========================================================================*/
/*                                SEL_OFF                                    */
/*===========================================================================*/
static unsigned long msel_off(unsigned long ss,unsigned long proc)
 {
 extern unsigned long p_head[200];
 unsigned long foff,ret;

 ret = 0;
 foff = ss / 8;
	if(proc==0) {
	foff=(foff*8)+0x200;
	} else {
	foff=(foff*8)+mem_load(p_head[proc]+LDT_off);
	}
 foff = foff + 2;
 ret = mem_wload(foff);
 foff = foff + 2;
 ret = ret + (m_al(mem_wload(foff))*0x10000);
 foff = foff + 2;
 ret = ret + (m_ah(mem_wload(foff))*0x1000000);
 return ret;
 }
/*===========================================================================*/
/*                                MSEL_HAT                                    */
/*===========================================================================*/
static unsigned long msel_hat(unsigned long ss,unsigned long proc)
 {
 extern unsigned long p_head[200];
 unsigned long foff,ret;
 ret = 0;
 foff = ss / 8;
	if(proc==0) {
	foff=(foff*8)+0x200;
	} else {
	foff=(foff*8)+mem_load(p_head[proc]+LDT_off);
	}
 ret = mem_wload(foff);
 foff = foff + 6;
 ret = ret + (m_al(mem_wload(foff)) & 0xF)*0x10000;
 return ret;
 }
/*===========================================================================*/
/*                                   AH                                      */
/*===========================================================================*/
static unsigned char m_ah(unsigned long eax)
{
 unsigned long ret;
	asm("pushl	%0"
		: 
		: "r" (eax));
	asm("popl	%eax\n\t
	     xorl	%edx,%edx\n\t
	     movb	%ah,%dl\n\t
	     pushl	%edx\n\t
	    ");
	asm("popl	%0"
		: "=r" (ret)
		: );
return ret; 
}
/*===========================================================================*/
/*                                   AL                                      */
/*===========================================================================*/
static unsigned char m_al(unsigned long eax)
{
 unsigned long ret;
	asm("pushl	%0"
		: 
		: "r" (eax));
	asm("popl	%eax\n\t
	     xorl	%edx,%edx\n\t
	     movb	%al,%dl\n\t
	     pushl	%edx\n\t
	    ");
	asm("popl	%0"
		: "=r" (ret)
		: );
return ret; 
}
