/* *		sysproc.c includes the execution & kill routines 	  *
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
#include "../include/dtypes.h"
#include "../include/prtcm.h"

#include "./sysproc.h"

ENTRY(sysproc);
static char dir[300];
static char *xp;
/*
    The SysProc is the GOD in the kernel. This makes processes born and die.
*/
/*===========================================================================*/
/*                                 GET_REC                                   */
/*===========================================================================*/
static void who_w()
{
 unsigned long *get,back,to,proc;
 unsigned long rc[99];
    init_sysproc();
    while(TRUE) {
    get = &rc[1]; back = 0;
    PUSH(get);PUSHN(24);PUSHN(0x405);CALL(105);ADDESP(12);
/*DO THE SYSCALL THAT'S DEFINED IN <RC>*/
	switch (rc[3]) {
	case 1:	back = do_exec(rc[1]);			break;
	case 2:	back = kill_proc(rc[4]);		break;
	case 3:	back = kill_proc(rc[1]);		break;
	}
/*FINALLY SEND BACK AN INFORMATION OF THE RESULT OF THE SYS_CALL*/
	to = (rc[1]*0x100)+4;
	if (back != 0xFFFFFFFF) {    
	    PUSH(back);PUSHN(1);PUSH(to);CALL(105);ADDESP(12);//send result
	} else {
	    PUSH(back);PUSHN(0);PUSH(to);CALL(105);ADDESP(12);//send result
	}
    }/*while forever*/
}
/*===========================================================================*/
/*                               INIT_SYSPROC                                */
/*===========================================================================*/
static void init_sysproc()
{
extern unsigned long p_head[200];
extern unsigned long max_proc,max_pid;
extern unsigned long proc_1[100];
unsigned long i;
 for (i=max_pid;i<MAX_PROC;i++) {
     p_head[i] = 0xFFFFFFFF;
 }
}
/*===========================================================================*/
/*                                 EXECUTE                                   */
/*===========================================================================*/
/******************************   search_pid    ******************************/
static unsigned long search_pid()
{
extern unsigned long p_head[200];
extern unsigned long max_proc,max_pid;
unsigned long i;
    for (i=40;i<max_pid;i++) {
	if (p_head[i] == 0xFFFFFFFF) {return(i);}
    }
    max_pid++;
    return(max_pid-1);
}
/*******************************    mk_proc    *******************************/
static void mk_proc(unsigned long tnm,unsigned long tss,unsigned long pri)
{
 extern unsigned long p_head[200];

 pmk_gdt32(((tnm * 2)+7-1),tss+LDT_SPACE,0x50,LDT_SYS(1,0,0));
 p_head[tnm] = tss;
 mem_str(p_head[tnm]+proc_TYPE,1);
 mem_str(p_head[tnm]+THR_NEXT,tnm);
 mem_str(p_head[tnm]+LDT_off,p_head[tnm]+LDT_SPACE);
 pmk_gdt32((tnm * 2)+7,tss+U_SPACE,104,TSS_DESC(1,pri,0));
}
/******************************    load_proc    ******************************/
static int load_proc(unsigned long fp,unsigned long start,unsigned long size
,unsigned long add)
{
unsigned long i;
unsigned char b;
 for (i=0;i<size;i++) {
    b = fread(fp,i+start);
    mem_bstr(add+i,b);
 }
 return(1);
}
/*******************************    get_type    ******************************/
static int get_type(unsigned long fp)
{
static char type[20];
    type[0] = fread(fp,4); // check for executable type
    type[1] = fread(fp,5);
    type[2] = fread(fp,6);
    type[3] = 0;
    if (strncmp(type,"SEF",3)==0) {
    return(1);
    } else {
    writeln("Unknown executable \n\0");return(0);
    }
}
/*********************************    exec    ********************************/
static int do_exec(unsigned long proc)
{
 extern unsigned long proc_1[100];
 extern unsigned long p_head[200];
 extern unsigned long max_proc,max_pid;
 unsigned long fp,i;
 unsigned long size,mem_place,mem_size,sef_size,head_size,seg_size;
 unsigned long pid,stack_size;
 unsigned long slen,ts;
 mem_str(p_head[19]+VT_ID,mem_load(p_head[proc]+VT_ID)); 	
    xp = &dir[0];
    PUSH(proc);PUSH(xp);PUSHN(70);PUSHN(0x109);CALL(105);ADDESP(16);
    PUSH(proc);PUSHN(0x06);CALL(105);ADDESP(8);//con dma
    dir[255] = 0;ts = 0;
    xp = strchr(dir,' ');
    if (xp != 0) {ts=1;xp[0]=0;}
    fp = fopen(dir,1);
    if (ts) {xp[0]=32;ts=0;}
	if (fp == 0) {writeln("Error opening file : \0");writeln(dir);writeln("\n\0");return(0);}
	if (get_type(fp) == 0) {return(0);}
	size = fsize(fp);
	sef_size = frlong(fp,0);
	stack_size = frlong(fp,8);
	head_size = frlong(fp,sef_size);
	size = size - sef_size - head_size;
	mem_size = size+1000+stack_size+50;
	mem_place = get_mem(mem_size);
	load_proc(fp,sef_size+4,size+head_size,mem_place);	
    fclose(fp);
 pid = search_pid();	//look for a pid thats not used 
 mk_proc(pid,mem_place,0);
 pmk_ldt32(pid,2,(mem_place+head_size),(size),CODE_SEG(1,1,0,1));
 pmk_ldt32(pid,3,(mem_place+head_size+size+1000),(stack_size),DATA_SEG(1,1,0,1));
 pmk_ldt32(pid,4,(mem_place+head_size),(size+1000+stack_size),DATA_SEG(1,1,0,1));
 load_mreg(pid,_CS,SEL(2,1,1));
 load_mreg(pid,_EIP,0);
 load_mreg(pid,_SS,SEL(3,1,1));
 load_mreg(pid,_DS,SEL(4,1,1));
 load_mreg(pid,_ES,SEL(4,1,1));
 load_mreg(pid,_EFLAGS,512);
 load_mreg(pid,_LDT_select,SEL((((/**/pid/**/ *2)+7)-1),0,0));
 mem_str(p_head[pid]+PS_start,mem_place);
 mem_str(p_head[pid]+PS_size,mem_size);
 mem_str(p_head[pid]+THR_MOTHER,proc);
 mem_str(p_head[pid]+VT_ID,mem_load(p_head[proc]+VT_ID)); 	
    xp = strchr(dir,' ');
	if (xp != 0) {
    	    slen = strlen(xp);
	    for (i=0;i<slen;i++) {
		mem_bstr(mem_place+head_size+size+1000+stack_size-(slen+1)+i,xp[i]);
	    }
	    mem_bstr(mem_place+head_size+size+1000+stack_size-1,0);
	    mem_str(mem_place+head_size+size+1000+stack_size-(slen+8),size+1000+stack_size-(slen+1));// this is argv[1]
    	    mem_str(mem_place+head_size+size+1000+stack_size-(slen+12),1);// this is argc
	}
    load_mreg(pid,_ESP,stack_size-(slen+16));
    LOCK;
	proc_1[max_proc] = pid;
	max_proc++;
	proc_1[max_proc] = 0xFFFFFFFF;
    UNLOCK;
 return(0xFFFFFFFF);
}

#include "../lib/string.c"
#include "./memutils.c"
#include "../lib/stdio.c"
#include "../lib/stdlib.c"
#include "../lib/emax.c"
/*===========================================================================*/
/*                                KILL_PROC                                  */
/*===========================================================================*/
/********************************    getpos    *******************************/
static unsigned long getpos(unsigned long pid)
{
 extern unsigned long proc_1[100];
 extern unsigned long max_proc,max_pid;
 unsigned long i;
 for (i=0;i<max_proc;i++) { if (proc_1[i] == pid) {return(i);}}
 writeln("No such pid\n\0");
 return(0);
}
/*********************************    getq    ********************************/
static int getq(unsigned long pid)
{
 extern unsigned long proc_1[100];
 extern unsigned long max_proc,max_pid;
 unsigned long i,pos;
    LOCK;
	pos = getpos(pid);
	if (pos == 0) {return(0);}
	for (i=pos;i<=max_proc;i++) {
	    proc_1[i] = proc_1[i+1];
	}
	max_proc--;
    UNLOCK;
    return(1);
}
/*******************************    kill_proc    *****************************/
static int kill_proc(unsigned long pid)
{
 extern unsigned long p_head[200];
 unsigned long mother;
 mem_str(p_head[19]+VT_ID,mem_load(p_head[pid]+VT_ID)); 	
    
     if (pid == 0) {return(0);} 
     if (getq(pid) == 0) {return(0);}
     mother = mem_load(p_head[pid]+THR_MOTHER);
     free_region(mem_load(p_head[pid]+PS_start),mem_load(p_head[pid]+PS_start)+mem_load(p_head[pid]+PS_size));
     /* free the memory of the proc */     
     p_head[pid] = 0xFFFFFFFF;/* free this pid */
     mem_str(p_head[mother]+proc_STATUS,0);/* we just don't want mummy to sleep over the whole day */
     return(1);
}
/*===========================================================================*/
/*                                 WRITELN                                   */
/*===========================================================================*/
static void writeln(char * ch)
{
unsigned long buff[255];
unsigned long i,*cp,size;
cp=&buff[1];
i=1;    
    while (ch[i-1] != 0) {
    buff[i] = ch[i-1];
    i++;}buff[i]=0;
    size = i+3;
PUSHN(6);PUSH(cp);PUSH(size);PUSHN(0x08);CALL(105);ADDESP(16);
PUSHN(1);PUSHN(0);PUSHN(4);PUSHESP;PUSHN(3);PUSHN(0x603);CALL(105);ADDESP(24);
}
/*===========================================================================*/
/*                                 READLN                                    */
/*===========================================================================*/
static void readln(char * ch)
{
unsigned long buff[267];
unsigned long i,*cp,dma,com;
cp=&buff[1];
PUSHN(6);PUSH(cp);PUSHN(255);PUSHN(0x108);CALL(105);ADDESP(16);
PUSHN(0);PUSHN(0);PUSHN(3);PUSHESP;PUSHN(3);PUSHN(0x603);CALL(105);ADDESP(24);
    i=1;    
    while (buff[i] != 0) {
    ch[i-1] = buff[i];
    i++;
    if (i == 255) {buff[i] = 0;}
    }ch[i-1] = 0;ch[i] = 0;
}
/*===========================================================================*/
/*                               MEM_BLOAD                                    */
/*===========================================================================*/
static unsigned char mem_bload(unsigned long offset)
{
unsigned long lng;
	asm("pushl	%edi");
	asm("pushl	%0"
		: 
		: "r" (offset));
	asm("popl	%edi\n\t
	     xorl	%eax,%eax\n\t
	     movb	%es:(%edi),%al\n\t
	     popl	%edi\n\t
	     pushl	%eax	    	
	    ");
	asm("popl	%0"
		: "=r" (lng)
		: );
return lng;
}
/*===========================================================================*/
/*                                MEM_BSTR                                    */
/*===========================================================================*/
static void mem_bstr(unsigned long offset,unsigned long data)
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
	     movb	%al,%es:(%edi)\n\t
	     popl	%edi\n\t	    	
	    ");
}
/*===========================================================================*/
/*                                MK_LDT32                                   */
/*===========================================================================*/
static void pmk_ldt32(unsigned int pn,unsigned int num,unsigned long offset,
unsigned long hat,unsigned int type)
 {
 extern unsigned long p_head[200];
 unsigned long foff;

 foff = (num * 8)+mem_load(p_head[pn]+LDT_off);
 mem_wstr(foff,m_axl(hat));
 foff = foff + 2;
 mem_wstr(foff,m_axl(offset));
 foff = foff + 2;
 mem_bstr(foff,m_al(m_axh(offset)));
 foff = foff + 1;
 mem_bstr(foff,m_al(type));
 foff = foff + 1;
 mem_bstr(foff,((m_ah(type)*0x10)+m_axh(hat)) );
 foff = foff + 1;
 mem_bstr(foff,m_ah(m_axh(offset)));
 }    
/*===========================================================================*/
/*                                MK_GDT32                                   */
/*===========================================================================*/
static void pmk_gdt32(unsigned int num,unsigned long offset,unsigned long hat,
 unsigned int type)
 {
 unsigned long foff;

 foff = (num * 8)+__GDT_ADDR;
 mem_wstr(foff,m_axl(hat));
 foff = foff + 2;
 mem_wstr(foff,m_axl(offset));
 foff = foff + 2;
 mem_bstr(foff,m_al(m_axh(offset)));
 foff = foff + 1;
 mem_bstr(foff,m_al(type));
 foff = foff + 1;
 mem_bstr(foff,((m_ah(type)*0x10)+m_axh(hat)) );
 foff = foff + 1;
 mem_bstr(foff,m_ah(m_axh(offset)));
 }    
/*===========================================================================*/
/*                                   AXH                                      */
/*===========================================================================*/
static unsigned int m_axh(unsigned long eax)
{
 unsigned long ret;
    ret = eax >> 16;
return ret; 
}
/*===========================================================================*/
/*                                   AXL                                      */
/*===========================================================================*/
static unsigned int m_axl(unsigned long eax)
{
 unsigned long ret;
	asm("pushl	%0"
		: 
		: "r" (eax));
	asm("popl	%eax\n\t
	     xorl	%edx,%edx\n\t
	     movw	%ax,%dx\n\t
	     pushl	%edx\n\t
	    ");
	asm("popl	%0"
		: "=r" (ret)
		: );
return ret; 
}
ENTRY(sysproc_end);
