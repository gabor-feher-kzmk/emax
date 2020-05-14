/* *   int.c includes the C language interrupt control of the kernel      *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
#include "../include/tss.h"
#include "../include/pcall.h"
#include "../include/dtypes.h"
#include "../include/pis.h"

/*
 * This routine will handle all interrupts (exceptions)
 */

void init_IDT_tasks()
{
    void exc();
    void exc_end();
    void exc_head();
    
 new_task(4,&exc_head,0);
 mk_ldt32(4,2,&exc,&exc_end-&exc,CODE_SEG(1,0,0,1));
 mk_ldt32(4,3,0x80000,0x400,DATA_SEG(1,0,0,1));
 mk_ldt32(4,4,0xB8000,4000,DATA_SEG(1,0,0,1));
 mk_ldt32(4,5,0,0xFFFFF,DATA_SEG(1,0,1,1));
 load_preg(4,_CS,SEL(2,1,0));
 load_preg(4,_EIP,0);
 load_preg(4,_SS,SEL(3,1,0));
 load_preg(4,_ESP,0x400);
 load_preg(4,_ES,SEL(4,1,0));
 load_preg(4,_DS,SEL(5,1,0));
 load_preg(4,_EFLAGS,0);
 load_preg(4,_LDT_select,SEL((((/**/4/**/ *2)+7)-1),0,0));
}

/*
 * This routine loads the IDT
 */ 
void load_IDT() 
 {
 init_IDT_tasks();
 mk_idt32(0x0,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x1,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x2,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x3,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x4,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x5,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x6,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x7,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x8,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x9,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0xA,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0xB,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0xC,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0xD,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0xE,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0xF,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x10,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x11,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x12,0,120,(TASK_GATE(1,0))*0x100);
 mk_idt32(0x13,0,120,(TASK_GATE(1,0))*0x100);
 }

/*===========================================================================*/
/*             These routines will handle all exceptions                     */
/*===========================================================================*/

void printi(unsigned long b,unsigned long type,unsigned long dest);
void pr_str(char *ch,int type,int x,int y);
static unsigned long mem_load(unsigned long offset);
static void ltoa(char *str,unsigned long lnum);

ENTRY(exc_head);
LONG(0x00000000);//the number of the proc's selector in GDT	
LONG(0x00000000);//LDT selector in GDT 
LONG(0x00000000);//the offset of the LDT
LONG(0x00000000);//the offset of the TSS
LONG(0x00000000);//the start and the 			INFO
LONG(0x00000000);//end of the proc's segments
LONG(0x00000000);//the start and the
LONG(0x00000000);//end of the proc's stack
/* misc info */
LONG(0x00000001);//the number of the vt witch is connected to the proc
LONG(0x00000000);//free for future usage
LONG(0x00000000);//free for future usage
LONG(0x00000000);//free for future usage
/* seched data */
LONG(0x00000001);//priority of proc
LONG(0x00000001);//data for priority count		SCHED
LONG(0x00000000);//the status of the proc
/* message data */
LONG(0x00000000);//send stack 
LONG(0xFFFFFFFF);//send dest proc			
LONG(0x00000000);//size of send message			MESSAGE
LONG(0x00000000);//recieve stack 
LONG(0xFFFFFFFF);//recieve dest proc			
LONG(0x00000000);//size of recieve message
/* dma data */
LONG(0x00000000);//dma offset 
LONG(0x00000000);//dma size				DMA
LONG(0x00000000);//dma dest proc
LONG(0x00000000);//dma mode (rw)
/* status */
LONG(0x00000000);/*the kill status of process (0=never kill it) the more the
proc will run if (dangerous slownes)*/
/*This will be the space where the CPU's registers are pushed*/
LONG(0x00000000); //	_Back Link	0
LONG(0x00000000); //	_ESP0		4
LONG(0x00000000); //	_SS0		8
LONG(0x00000000); //	_ESP1		12
LONG(0x00000000); //	_SS1		16
LONG(0x00000000); //	_ESP2		20
LONG(0x00000000); //	_SS2		24
LONG(0x00000000); //	_CR3		28
LONG(0x00000000); //	_EIP		32
LONG(0x00000000); //	_EFLAGS		36
LONG(0x00000000); //	_EAX		40
LONG(0x00000000); //	_ECX		44
LONG(0x00000000); //	_EDX		48
LONG(0x00000000); //	_EBX		52
LONG(0x00000000); //	_ESP		56
LONG(0x00000000); //	_EBP		60
LONG(0x00000000); //	_ESI		64
LONG(0x00000000); //	_EDI		68
LONG(0x00000000); //	_ES		72
LONG(0x00000000); //	_CS		76
LONG(0x00000000); //	_SS		80
LONG(0x00000000); //	_DS		84
LONG(0x00000000); //	_FS		88
LONG(0x00000000); //	_GS		92
LONG(0x00000000); //	_LDT_select	96
LONG(0x00000000); //	_IO_map		100
/* this is the space where the processes LDT is saved*/
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT0
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT1
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT2
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT3
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT4
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT5
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT6
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT7
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT8
LONG(0x00000000);//|
LONG(0x00000000);//|> LDT9
/*This is the space where the processes signals are defined */
LONG(0xFFFFFFFF);

static char reg[60];

void exc()
 {
 extern unsigned long doing_proc,sys_proc;
 extern unsigned long uct_time[3];
 extern unsigned long p_head[200];
LOCK; 
    pr_str("EXCEPTION",143,35,0);
    pr_str("Stats of proc \0",15,0,0);ltoa(reg,doing_proc);pr_str(reg,15,14,0);

    pr_str("EIP : \0",15,0,6);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_EIP));pr_str(reg,15,8,6);
    pr_str("EFLAGS: \0",15,0,7);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_EFLAGS));pr_str(reg,15,8,7);
    pr_str("EAX : \0",15,0,8);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_EAX));pr_str(reg,15,8,8);
    pr_str("ECX : \0",15,0,9);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_EBX));pr_str(reg,15,8,9);
    pr_str("EDX : \0",15,0,10);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_ECX));pr_str(reg,15,8,10);
    pr_str("EBX : \0",15,0,11);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_EDX));pr_str(reg,15,8,11);
    pr_str("ESP : \0",15,0,12);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_ESP));pr_str(reg,15,8,12);
    pr_str("EBP : \0",15,0,13);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_EBP));pr_str(reg,15,8,13);
    pr_str("ESI : \0",15,0,14);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_ESI));pr_str(reg,15,8,14);
    pr_str("EDI : \0",15,0,15);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_EDI));pr_str(reg,15,8,15);
    pr_str("ES : \0",15,0,16);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_ES));pr_str(reg,15,8,16);
    pr_str("CS : \0",15,0,17);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_CS));pr_str(reg,15,8,17);
    pr_str("SS : \0",15,0,18);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_SS));pr_str(reg,15,8,18);
    pr_str("DS : \0",15,0,19);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_DS));pr_str(reg,15,8,19);
    pr_str("FS : \0",15,0,20);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_FS));pr_str(reg,15,8,20);
    pr_str("GS : \0",15,0,21);
    ltoa(reg,mem_load(p_head[doing_proc]+U_SPACE+_GS));pr_str(reg,15,8,21);
    pr_str("r_STK: \0",15,0,22);
    ltoa(reg,mem_load(p_head[doing_proc]+mesr_STK));pr_str(reg,15,8,22);
    pr_str("r_DEST: \0",15,0,23);
    ltoa(reg,mem_load(p_head[doing_proc]+mesr_DEST));pr_str(reg,15,8,23);
    pr_str("r_SIZE: \0",15,0,24);
    ltoa(reg,mem_load(p_head[doing_proc]+mesr_SIZE));pr_str(reg,15,8,24);
    pr_str("s_STK: \0",15,0,3);
    ltoa(reg,mem_load(p_head[doing_proc]+mess_STK));pr_str(reg,15,8,3);
    pr_str("s_DEST: \0",15,0,4);
    ltoa(reg,mem_load(p_head[doing_proc]+mess_DEST));pr_str(reg,15,8,4);
    pr_str("s_SIZE: \0",15,0,5);
    ltoa(reg,mem_load(p_head[doing_proc]+mess_SIZE));pr_str(reg,15,8,5);
    pr_str("dmaTO: \0",15,0,1);
    ltoa(reg,mem_load(p_head[doing_proc]+dma_PTO));pr_str(reg,15,8,1);
    pr_str("dm2TO: \0",15,0,2);
    ltoa(reg,mem_load(p_head[doing_proc]+dma2_PTO));pr_str(reg,15,8,2);
 while (TRUE) { /*FOREVER*/ }
}
/*===========================================================================*/
/*                                   PRINT                                   */
/*===========================================================================*/
void pr_str(char *ch,int type,int x,int y)
{
unsigned long pos,i;
 pos =  (y * 160) + (x*2);
 i = 0;
     while (ch[i] != 0) {
     printi(ch[i],type,pos+(i*2));
     i++;
     }
}
void printi(unsigned long b,unsigned long type,unsigned long dest)
{
    unsigned long ch;

    ch = b + ((type)*0x100);	
	asm("pushl	%edi\n\t");
	asm("pushl	%0"	
		: 
		: "r" (ch));
	asm("pushl	%0"	
		: 
		: "r" (dest));
	asm("
	     popl	%edi\n\t
	     popl	%eax\n\t
	     movw	%ax,%es:(%edi)\n\t	
	     popl	%edi\n\t
	    ");	
}
/*===========================================================================*/
/*                                   LTOA                                    */
/*===========================================================================*/
static void ltoa(char *str,unsigned long lnum)
{
unsigned long i,div,num,cnum,digit,set;
if (lnum == 0) {str[0] = 48;str[1] = 0;return;}
 div = 1000000000;
 num = lnum;
 cnum = 0;
 set = 0;
    for (i=0;i<10;i++) {
	digit = (num / div);
	num = num % div;
	div = div / 10;
	    if (digit != 0) { 
		str[cnum] = 48 + digit;
		cnum++;
		set=1;
	    } else {
		if (set != 0) {str[cnum] = 48 + digit;cnum++;}
	    }
    }
    str[cnum] = 0;
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

void exc_end()
{
}


         