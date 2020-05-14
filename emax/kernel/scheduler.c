/* *   		scheduler.c includes the scheduling algorithm 		  *
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
#include "../include/dtypes.h"
#include "../include/tss.h"
#include "../include/pis.h"

/* This routine handles the switches between the processes */
/*===========================================================================*/
/*               ||TASK 88||      SCHEDULER                                  */
/*===========================================================================*/
    void reset_pri();
    unsigned long get_max();
    static void free_proc(unsigned proc);
    void switch_proc(unsigned proc, unsigned rpl);
    static void mem_dec(unsigned long offset);
    static void mem_str(unsigned long offset,unsigned long data);
    static unsigned long mem_load(unsigned long offset);
#include "scheduler.h"
/*
    The scheduler is the hearth of the kernel. This supports the CPU
    time for the user level procs. It does priority scheduling, but
    this won't bee long. We'll change this ancient routine to a newer
    one. ;-) 
*/
ENTRY(scheduler);
void scheduler_loop()
 {
 extern unsigned long p_head[200];
 extern unsigned long doing_proc,sys_proc;
 extern unsigned long max_proc,max_pid;
 extern unsigned long proc_1[100];
 unsigned long proc_nr;

    proc_1[1] = 0;
    proc_1[2] = 6;
    proc_1[3] = 7;
    proc_1[4] = 8;
    proc_1[5] = 19;
    proc_1[6] = 38;
    proc_1[7] = 9;
    proc_1[8] = 15;
    proc_1[9] = 10;
    proc_1[10] = 16;
    proc_1[11] = 14;
    proc_1[12] = 17;
    proc_1[13] = 12;
    proc_1[14] = 18;
    proc_1[15] = 21;
    proc_1[16] = 22;
    proc_1[17] = 0xFFFFFFFF;
    proc_1[18] = 0xFFFFFFFF;
    max_proc = 17;max_pid=40;
    
    while(TRUE) { 
    sys_proc = 1;
	    proc_nr = get_max();
		    if (proc_nr != 0xFFFFFFFF)
		    {
		    mem_dec(p_head[proc_nr]+p_PRI_data);
		    switch_proc(proc_nr,0);
		    } else
		    {
		    reset_pri();
	    	    proc_nr = get_max();
			if (proc_nr == 0xFFFFFFFF)
			{
			switch_proc(NULL_PROC,0);
			} else {}
		    }
	   
    }
 }

void switch_proc(unsigned proc, unsigned rpl)
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
 
unsigned long get_max()
 {
 extern unsigned long p_head[200];
 extern unsigned long proc_1[100];
 unsigned long max,pt,con,pn;
    max = 0;
    pn = 0xFFFFFFFF;
    con = 1;
    pt = proc_1[con];
      while (pt != 0xFFFFFFFF) {
	 if (mem_load(p_head[pt]+p_PRI_data) > max) 
	  {
	    if (mem_load(p_head[pt]+proc_STATUS) != 1)
	     {  
	      max = mem_load(p_head[pt]+p_PRI_data);
	      pn = pt;
	     }
	  } 
         con++;
	 pt = proc_1[con];
      }
return pn; 
}

void reset_pri()
 {
 extern unsigned long p_head[200];
 extern unsigned long proc_1[100];
 unsigned long prn,con;
    con = 1;
    prn = proc_1[con];
      while (prn != 0xFFFFFFFF) {
	 if (mem_load(p_head[prn]+proc_STATUS) != 1) {
          mem_str(p_head[prn]+p_PRI_data,mem_load(p_head[prn]+proc_PRI));
	  } else {}
         con++;
	 prn = proc_1[con];
      }
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
/*                               MEM_DEC                                    */
/*===========================================================================*/
static void mem_dec(unsigned long offset)
{
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("popl	%edi\n\t
	     decl	%es:(%edi)\n\t
	     popl	%edi\n\t
	    ");
}

ENTRY(scheduler_end);

 