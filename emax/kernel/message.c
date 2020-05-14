/* * 	      message.c includes the routines to handle messages  	  *
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
#include "../include/pis.h"
#include "../include/tss.h"
/*
    This is the most important part of the kernel. The message server
    handles the Inter Process Communication. It does some for the DMA
    and it does some job for the signal server too. 
*/
 
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*                              MESSAGE_SERVER                               */
/*'''''''''''''''''''''''''''''''''''''``````````````````````````````````````*/
static void switch_proc(unsigned proc, unsigned rpl);
static unsigned long mem_load(unsigned long offset);
static void mem_str(unsigned long offset,unsigned long data);
static unsigned char m_al(unsigned long eax);
static unsigned long msel_off(unsigned long ss,unsigned long proc);
static unsigned char m_ah(unsigned long eax);
static unsigned long get_mreg(unsigned long pn,unsigned long reg);
static void load_mreg(unsigned long pn,unsigned long reg,unsigned long data);
static unsigned int mem_wload(unsigned long offset);
static unsigned long msel_hat(unsigned long ss,unsigned long proc);
static void send(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long to);
static void rec(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long from);
static void msg_copy(unsigned long src,unsigned long dest,unsigned long size);
static void sys_call(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long to);
static void call(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long to);
static void do_sys(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long to);
static void rec_sys(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long from);
static unsigned long sfs(unsigned long call_num);
static unsigned long sfc(unsigned long call_num);
static unsigned long mk_dma(unsigned long off,unsigned long size,
unsigned long proc,char mode);
static unsigned long put_dma(unsigned long num,unsigned long to);
static unsigned long con_dma(unsigned long num);
static unsigned long con_dma2(unsigned long num);
static void sleep(unsigned long stat);
static int check_mem(unsigned long pid,unsigned long off,unsigned long size);

#include "message.h"
ENTRY(message);
static void message_type()
{
 extern unsigned long doing_proc,sys_proc;
 unsigned long tmp;
 unsigned long stack,esp,ss;
 unsigned long size,mode,sw,offset,ext;

 while(TRUE) { 
     sys_proc = 3;
     ss = msel_off(get_mreg(doing_proc,_SS),doing_proc);
     esp = get_mreg(doing_proc,_ESP);
     size = mem_load(ss+esp+4) * 4;
     mode = mem_load(ss+esp);
     offset = mem_load(ss+esp+8);
     ext = mem_load(ss+esp+12);
     sw = m_al(mode);

    if (sw < 3) {
        switch (sw) {
	    case 1:send(ss,esp,size,offset,mode/0x100);break;
	    case 2:rec(ss,esp,size,offset,mode/0x100);break;
	}    
    } else {
     if (sw < 7) {
	switch (sw) {
	    case 3: tmp = mode/0x100;
	     if (tmp == 4) { sys_call(ss,esp,size,offset,/*sfs(mem_load(offset))*/0);
		} else { call(ss,esp,size,offset,mode/0x100); }
	    break;
	    case 4:do_sys(ss,esp,size,offset,mode/0x100);break;
	    case 5:rec_sys(ss,esp,size,offset,mode/0x100);break;
	    case 6:con_dma(size/4);CALL(88);break;
	}
      } else {
	    if (sw < 11) { 
		switch (sw) {
		case 7:put_dma(mode/0x100,size/4);CALL(88);break;
		case 8:
		mk_dma(offset+ss,size,ext,mode/0x100);CALL(88);
		break;
		case 9:
		ss = msel_off(get_mreg(doing_proc,_DS),doing_proc);
		mk_dma(offset+ss,size,ext,mode/0x100);CALL(88);
		break;
		case 10:sleep(mode/0x100);break;
	    }
	    } else {
		switch (sw) {
		case 12:con_dma2(size/4);CALL(88);break;
		default:/*while (TRUE) {}*/ CALL(88);
		}
	    }
      }
    }	
 } //while (TRUE) {}
}
/*===========================================================================*/
/*                                  SEND                                     */
/*===========================================================================*/
static void send(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long to)
{
 extern unsigned long p_head[200];
 extern unsigned long doing_proc;
     if (size < 100) {
        if (check_mem(doing_proc,ss+offset,size)) {
	    if (mem_load(p_head[to]+mesr_DEST) != doing_proc) {
	     mem_str(p_head[doing_proc]+mess_DEST,to);
	     mem_str(p_head[doing_proc]+mess_SIZE,size);
	     mem_str(p_head[doing_proc]+mess_STK,offset+ss);
	     mem_str(p_head[doing_proc]+proc_STATUS,1);	     
	    } else {
	         if (size + 8 <= mem_load(p_head[to]+mesr_SIZE)) {
		    mem_str(mem_load(p_head[to]+mesr_STK),doing_proc);
		    mem_str(mem_load(p_head[to]+mesr_STK)+4,ss);
		    msg_copy(offset+ss,mem_load(p_head[to]+mesr_STK)+8,size/4);
		    mem_str(p_head[to]+mesr_DEST,0xFFFFFFFF);
	    	    mem_str(p_head[to]+proc_STATUS,0);	     
		 } 
	      } 
	}
    }
 CALL(88);
}
/*===========================================================================*/
/*                                  REC                                      */
/*===========================================================================*/
static void rec(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long from)
{
 extern unsigned long p_head[200];
 extern unsigned long doing_proc;
     if (size < 100) {
        if (check_mem(doing_proc,ss+offset,size)) {
	    if (mem_load(p_head[from]+mess_DEST) != doing_proc) {
	     mem_str(p_head[doing_proc]+mesr_DEST,from);
	     mem_str(p_head[doing_proc]+mesr_SIZE,size);
	     mem_str(p_head[doing_proc]+mesr_STK,offset+ss);
	     mem_str(p_head[doing_proc]+proc_STATUS,1);	     
	    } else {
	         if (size >= mem_load(p_head[from]+mess_SIZE) + 8) {
		    mem_str(offset+ss,doing_proc);
		    mem_str(offset+ss+4,ss);
		    msg_copy(mem_load(p_head[from]+mess_STK),offset+ss+8,size/4);
		    mem_str(p_head[from]+mess_DEST,0xFFFFFFFF);
		    mem_str(p_head[from]+proc_STATUS,0);	     
		 } 
	   }
	}
    }
 CALL(88);
}
/*===========================================================================*/
/*                                SYS_CALL                                   */
/*===========================================================================*/
static void sys_call(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long to)
{
 extern unsigned long p_head[200];
 extern unsigned long doing_proc; 
 if (size < 100) {
        if (check_mem(doing_proc,ss+offset,size)) {
	    if (mem_load(p_head[to]+mesr_DEST) == 0xFFFFFFFF) {
	     mem_str(p_head[doing_proc]+mess_DEST,to);
	     mem_str(p_head[doing_proc]+mess_SIZE,size);
	     mem_str(p_head[doing_proc]+mess_STK,offset+ss);
             mem_str(p_head[doing_proc]+mesr_DEST,to);
	     mem_str(p_head[doing_proc]+proc_STATUS,1);	     
	    } else {
	      if (mem_load(p_head[to]+mesr_DEST) == 4) {
	         if (size + 8 <= mem_load(p_head[to]+mesr_SIZE)) {
		    mem_str(mem_load(p_head[to]+mesr_STK),doing_proc);
		    mem_str(mem_load(p_head[to]+mesr_STK)+4,ss);
		    msg_copy(offset+ss,mem_load(p_head[to]+mesr_STK)+8,size/4);
		    mem_str(p_head[to]+mesr_DEST,0xFFFFFFFF);
	            mem_str(p_head[to]+proc_STATUS,0);	     
		    mem_str(p_head[doing_proc]+mesr_DEST,0xFFFFFFFF);
	            mem_str(p_head[doing_proc]+proc_STATUS,1);	     
		 } 
	      } 
	    }	 
        }
    }
/*doing_proc = 3;*/
CALL(88);
}
/*===========================================================================*/
/*                                  CALL                                     */
/*===========================================================================*/
static void call(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long to)
{
 extern unsigned long p_head[200];
 extern unsigned long doing_proc;
     if (size < 100) {
        if (check_mem(doing_proc,ss+offset,size)) {
	    if (mem_load(p_head[to]+mesr_DEST) == 0xFFFFFFFF) {
	     mem_str(p_head[doing_proc]+mess_DEST,to);
	     mem_str(p_head[doing_proc]+mess_SIZE,size);
	     mem_str(p_head[doing_proc]+mess_STK,offset+ss);
	     mem_str(p_head[doing_proc]+proc_STATUS,1);	     
	    } else {
	         if (size + 8 <= mem_load(p_head[to]+mesr_SIZE)) {
		    mem_str(mem_load(p_head[to]+mesr_STK),doing_proc);
		    mem_str(mem_load(p_head[to]+mesr_STK)+4,ss);
		    msg_copy(offset+ss,mem_load(p_head[to]+mesr_STK)+8,size/4);
		    mem_str(p_head[to]+mesr_DEST,0xFFFFFFFF);
		    mem_str(p_head[to]+proc_STATUS,0);	     
	            mem_str(p_head[doing_proc]+proc_STATUS,1);	     
		 }
	    }
	}
    }CALL(88);
}
/*===========================================================================*/
/*                                 DO_SYS                                    */
/*===========================================================================*/
static void do_sys(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long off,unsigned long to)
{
 extern unsigned long p_head[200];
 extern unsigned long doing_proc;

		    mem_str(p_head[to]+U_SPACE+_EAX,off);
		    mem_str(p_head[to]+mesr_DEST,0xFFFFFFFF);
		    if (size == 4) { 
	    	    mem_str(p_head[to]+proc_STATUS,0);}
 CALL(88);
}
/*===========================================================================*/
/*                                REC_SYS                                    */
/*===========================================================================*/
static void rec_sys(unsigned long ss,unsigned long esp,unsigned long size,
unsigned long offset,unsigned long from)
{
 extern unsigned long p_head[200];
 extern unsigned long doing_proc;

     if (size < 100) {
        if (check_mem(doing_proc,ss+offset,size)) {
	 if (from == 4) {
	 from = sfc(doing_proc);
	 }
	    if (mem_load(p_head[from]+mess_DEST) == 0xFFFFFFFF) {
	     mem_str(p_head[doing_proc]+mesr_DEST,from);
	     mem_str(p_head[doing_proc]+mesr_SIZE,size);
	     mem_str(p_head[doing_proc]+mesr_STK,offset+ss);
	     mem_str(p_head[doing_proc]+proc_STATUS,1);	     
	    } else {
	     if (mem_load(p_head[from]+mess_DEST) == doing_proc) {
	         if (size >= mem_load(p_head[from]+mess_SIZE) + 8) {
		  mem_str(offset+ss,from);
		  mem_str(offset+ss+4,msel_off(get_mreg(from,_SS),from));
msg_copy(mem_load(p_head[from]+mess_STK),offset+ss+8,mem_load(p_head[from]+mess_SIZE)/4);
	          mem_str(p_head[from]+mess_DEST,0xFFFFFFFF);
	         } 
	     }	 
	    }
	}
    }
 /*doing_proc = 3;*/
 CALL(88);
}
/*===========================================================================*/
/*                                   SFS                                     */
/*===========================================================================*/
// Search for a sys_call server !!!
static unsigned long sfs(unsigned long call_num)
{
unsigned long to;

to = 0;

return to;
}
/*===========================================================================*/
/*                                   SFC                                     */
/*===========================================================================*/
// Search for a sys_call !!!
static unsigned long sfc(unsigned long call_num)
{
extern unsigned long doing_proc;
extern unsigned long proc_1[100];
extern unsigned long max_proc,max_pid;
extern unsigned long p_head[200];
unsigned long to,i,pn;

to = 4;
i = mem_load(p_head[doing_proc]+SFC_id);
if (i == 0) {i = 1;}
if (i > max_proc) {i = 1;}
pn = proc_1[i];
    while (pn != 0xFFFFFFFF) { 
	if (mem_load(p_head[pn]+mess_DEST) == call_num) {
	to = pn;
	mem_str(p_head[doing_proc]+SFC_id,to+1);
	return(to);
	} 
     i++;
     pn = proc_1[i];
    }
i = 1;pn = proc_1[i];
    while (pn != 0xFFFFFFFF) { 
	if (mem_load(p_head[pn]+mess_DEST) == call_num) {
	to = pn;
	mem_str(p_head[doing_proc]+SFC_id,to+1);
	return(to);
	} 
     i++;
     pn = proc_1[i];
    }
return(to);
}
/*===========================================================================*/
/*                                  MK_DMA                                   */
/*===========================================================================*/
static unsigned long mk_dma(unsigned long off,unsigned long size,
unsigned long proc,char mode)
{
extern unsigned long p_head[200];
extern unsigned long doing_proc;
    if (mode < 2) {
        if (check_mem(doing_proc,off,size)) {
	    mem_str(p_head[doing_proc]+dma_OFF,off);
    	    mem_str(p_head[doing_proc]+dma_SIZE,size);
    	    mem_str(p_head[doing_proc]+dma_MODE,mode);
    	    mem_str(p_head[doing_proc]+dma_PTO,proc);
	    }
    } else {
	mode = mode-2;
        if (check_mem(doing_proc,off,size)) {
    	    mem_str(p_head[doing_proc]+dma2_OFF,off);
    	    mem_str(p_head[doing_proc]+dma2_SIZE,size);
    	    mem_str(p_head[doing_proc]+dma2_MODE,mode);
    	    mem_str(p_head[doing_proc]+dma2_PTO,proc);
	}
    }
return 1;
}
/*===========================================================================*/
/*                               NEXT_DMA_W                                  */
/*===========================================================================*/
static int next_dma_w()
{
extern unsigned long dma_w;
    dma_w++;
    if (dma_w > 100) {dma_w = 0;}
return(1);
}
/*===========================================================================*/
/*                                CON_DMA                                    */
/*===========================================================================*/
static unsigned long con_dma(unsigned long num)
{
extern unsigned long p_head[200],doing_proc,dm1,dma_w;
extern struct dwq dma_wq[120];
unsigned long ret,md;
ret=0;
if (mem_load(p_head[num]+dma_PTO)==doing_proc) { 
    if(mem_load(p_head[doing_proc]+dma_PTO) == num) {
	md=(mem_load(p_head[num]+dma_MODE)*2)+mem_load(p_head[doing_proc]+dma_MODE);
	switch (md) {
             case 1:
		dma_wq[dma_w].usg =1;
		dma_wq[dma_w].slp =1;
		dma_wq[dma_w].s_proc = num;
		dma_wq[dma_w].d_proc = doing_proc;
		mem_str(p_head[doing_proc]+proc_STATUS,1);	     
		mem_str(p_head[doing_proc]+dma_PTO,0xFFFFFFFF);	     
		mem_str(p_head[num]+dma_PTO,0xFFFFFFFF);	     
		next_dma_w();
		mem_str(p_head[8]+proc_STATUS,0);	     
		ret=1;
	     break;
	     case 2:
		dma_wq[dma_w].usg =1;
		dma_wq[dma_w].slp =2;
		dma_wq[dma_w].s_proc = doing_proc;
		dma_wq[dma_w].d_proc = num;
		mem_str(p_head[doing_proc]+proc_STATUS,1);
		mem_str(p_head[doing_proc]+dma_PTO,0xFFFFFFFF);	     
		mem_str(p_head[num]+dma_PTO,0xFFFFFFFF);	     
		next_dma_w();
		mem_str(p_head[8]+proc_STATUS,0);	     
		ret=1;
	     break;
	} 
    }
}
return(ret);
}
/*===========================================================================*/
/*                                CON_DMA2                                   */
/*===========================================================================*/
static unsigned long con_dma2(unsigned long num)
{
extern unsigned long p_head[200],doing_proc,dm1,dma_w;
extern struct dwq dma_wq[120];
unsigned long ret,i,md;
ret=0;
if (mem_load(p_head[num]+dma2_PTO)==doing_proc) { 
    if(mem_load(p_head[doing_proc]+dma2_PTO) == num) {
	md=(mem_load(p_head[num]+dma2_MODE)*2)+mem_load(p_head[doing_proc]+dma2_MODE);
	 switch (md) {
	    case 1:
		dma_wq[dma_w].usg =2;
		dma_wq[dma_w].slp =1;
		dma_wq[dma_w].s_proc = num;
		dma_wq[dma_w].d_proc = doing_proc;
		mem_str(p_head[doing_proc]+proc_STATUS,1);
		mem_str(p_head[doing_proc]+dma2_PTO,0xFFFFFFFF);	     
		mem_str(p_head[num]+dma2_PTO,0xFFFFFFFF);	     
		next_dma_w();
		mem_str(p_head[8]+proc_STATUS,0);	     
		ret=1;
	    break;
	    case 2:
		dma_wq[dma_w].usg =2;
		dma_wq[dma_w].slp =2;
		dma_wq[dma_w].s_proc = doing_proc;
		dma_wq[dma_w].d_proc = num;
		mem_str(p_head[doing_proc]+proc_STATUS,1);
		mem_str(p_head[doing_proc]+dma2_PTO,0xFFFFFFFF);	     
		mem_str(p_head[num]+dma2_PTO,0xFFFFFFFF);	     
		next_dma_w();
		mem_str(p_head[8]+proc_STATUS,0);	     
		ret=1;
	    break;
	 }
    } 
}
return(ret);
}
/*===========================================================================*/
/*                                PUT_DMA                                    */
/*===========================================================================*/
static unsigned long put_dma(unsigned long num,unsigned long to)
{
extern unsigned long p_head[200];
extern unsigned long doing_proc;
int ret;ret=0;
    if (mem_load(p_head[num]+dma_PTO) == doing_proc) {
	mem_str(p_head[num]+dma_PTO,to);ret=1;
    }
return ret;
}
/*===========================================================================*/
/*                                 SLEEP                                     */
/*===========================================================================*/
static void sleep(unsigned long stat)
{
extern unsigned long p_head[200];
extern unsigned long doing_proc;
	if (stat < 2) {
	    mem_str(p_head[doing_proc]+proc_STATUS,stat);
	    CALL(88);
	} else {
	    if (stat < 4) {
		mem_str(p_head[mem_load(p_head[doing_proc]+THR_MOTHER)]+proc_STATUS,stat-2);
		switch_proc(doing_proc,0);
	    } 
	}
}
/*===========================================================================*/
/*                                CHECK_MEM                                  */
/*===========================================================================*/
static int check_mem(unsigned long pid,unsigned long off,unsigned long size)
{
extern unsigned long p_head[200];
    if (off < mem_load(p_head[pid]+PS_start)) {return(0);}
    if (size > mem_load(p_head[pid]+PS_size)) {return(0);}
return(1);
}
/*===========================================================================*/
/*                                  UTILS                                    */
/*===========================================================================*/

static void free_proc(unsigned proc)
 {
 unsigned toff;
 toff = 0x238+(((2*proc))*8)+5;
	asm("
	pushl	%edi
	");
	asm("pushl %0
		"
		: "=r" (toff)
		: "r" (toff));
	asm("
	popl	%edi
	movb	%es:(%edi),%al
	andb	$253,%al
	movb	%al,%es:(%edi)
	popl	%edi
	");
 }
static void switch_proc(unsigned proc, unsigned rpl)
 {
 extern unsigned long doing_proc;
 unsigned long num;
 doing_proc = proc;
 num =	(8 * ((proc*2)+7)) + rpl;
 free_proc(proc);
	PUSH(num);
     asm("
	popl	%eax
	pushl	%edi
	movl	$swch_to,%edi
	addl	$5,%edi
	movb	%al,%ds:(%edi)
	inc 	%edi
	movb	%ah,%ds:(%edi)
	popl	%edi
	jmp	swch_to
	swch_to:
	ljmp	$0xFEBA,$0     
     ");
 }
#include "memutils.c"
ENTRY(message_end);


