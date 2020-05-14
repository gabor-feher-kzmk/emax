/* *	dma.c includes the utilities for the kernel's Direct Memory Acces *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
#include "../include/pcall.h"
#include "../include/tss.h"
#include "../include/pis.h"

static void proc_stat(unsigned long proc,unsigned long slp);
static void msg_copy(unsigned long src,unsigned long dest,unsigned long size);
static void roll_wq();
static void mem_str(unsigned long offset,unsigned long data);
static unsigned long mem_load(unsigned long offset);
static int next_dma();
#include "dma.h"
ENTRY(dma);
/*
    The DMA task is used, when two proc wants to transfer more then 100
    bytes. They call the Message server and it sets one of the colums of
    the 'dma_wq'. When the DMA wakes up, it'll transfer the data.
*/

/*===========================================================================*/
/*                               CIRC_DMA                                    */
/*===========================================================================*/
void circ_dma()
{
extern unsigned long p_head[200],dma_p,dm1; 
extern struct dwq dma_wq[120];
 LOCK;
 while(TRUE) { 
     if (dma_wq[dma_p].usg == 1) {
	    msg_copy(mem_load(p_head[dma_wq[dma_p].s_proc]+dma_OFF),mem_load(p_head[dma_wq[dma_p].d_proc]+dma_OFF),
                     mem_load(p_head[dma_wq[dma_p].s_proc]+dma_SIZE)/4);
	 switch (dma_wq[dma_p].slp) {	case 1:proc_stat(dma_wq[dma_p].d_proc,0);break;
	                                case 2:proc_stat(dma_wq[dma_p].s_proc,0);break;	}
         next_dma();
     } else {
     if (dma_wq[dma_p].usg == 2) {
             msg_copy(mem_load(p_head[dma_wq[dma_p].s_proc]+dma2_OFF),mem_load(p_head[dma_wq[dma_p].d_proc]+dma2_OFF),
		      mem_load(p_head[dma_wq[dma_p].s_proc]+dma2_SIZE)/4);
	 switch (dma_wq[dma_p].slp) {	case 1:proc_stat(dma_wq[dma_p].d_proc,0);break;
	                                case 2:proc_stat(dma_wq[dma_p].s_proc,0);break;	}
         next_dma();
     } else {
     PUSHN(0x10A);CALL(105);ADDESP(4);/* go to sleep */
     }}
 }     
}
/*===========================================================================*/
/*                               NEXT_DMA                                    */
/*===========================================================================*/
static int next_dma()
{
extern unsigned long dma_p,dma_w,dm1; 
extern struct dwq dma_wq[120];
    dma_wq[dma_p].usg = 0;
    dma_wq[dma_p].d_proc = 0;
    dma_wq[dma_p].s_proc = 0;
    dma_wq[dma_p].slp = 0;
    dma_p++;
    if (dma_p > 100) {
	dma_p = 0;
    }
    if (dma_p ==  dma_w) {PUSHN(0x10A);CALL(105);ADDESP(4);}/* if no job then go to sleep */
}
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
UNLOCK;
asm("popl	%ecx
     popl	%edi
     popl	%esi
     cld
     rep
     movsl
    ");
LOCK;
asm("popl	%esi
     popl	%edi
    ");
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
	     movl	%eax,%ds:(%edi)\n\t
	     popl	%edi\n\t	    	
	    ");
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
	     movl	%ds:(%edi),%eax\n\t
	     popl	%edi\n\t
	     pushl	%eax	    	
	    ");
	asm("popl	%0"
		: "=r" (lng)
		: );
return lng;
}
/*===========================================================================*/
/*                                 PROC_STAT                                 */
/*===========================================================================*/
static void proc_stat(unsigned long proc,unsigned long slp)
{
 extern unsigned long p_head[200];
    mem_str(p_head[proc]+proc_STATUS,slp);
}

ENTRY(dma_end);

