/* *		    sys_call.c includes 1,2 previliges routines	  	  *
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
static int make_signal(unsigned long proc_to,unsigned long signal);
static int set_timer(unsigned long proc,unsigned long tfs);
static int proc_status();
static void reboot_system();
static int pr_time();
#include "system.h"
/*===========================================================================*/
/*                                 SYS_LOOP                                  */
/*===========================================================================*/
/*The main loop of the 0. (sys_call) task*/
static int f;

void sys_loop()
{
 extern unsigned long p_head[200];
 unsigned long *get,str,back,com;
 unsigned long rc[99];
 char * hex = "xxxx\0";
 f=1;
     while(TRUE) { 
	get = &rc[1];back = 0;
	PUSH(get);PUSHN(24);PUSHN(0x405);CALL(105);ADDESP(12);
/*DO THE SYSCALL THAT'S DEFINED IN <RC>*/
	switch (rc[3]) {
	    case 1 : back=proc_status(); 		break;
	    case 2 : back=pr_time();			break;
	    case 3 : 
		if (mem_vll(p_head[rc[1]]+proc_TYPE)==2) {
		back=set_timer(mem_vll(p_head[rc[1]]+THR_MOTHER),rc[4]);
		} else {
		back=set_timer(rc[1],rc[4]);
		}
	    break;
	    case 4 : reboot_system(); 			break;
	    default :
		switch (rc[3]) {
		case 5 : make_signal(rc[4],rc[5]);		break;
		}
	}
/*FINALLY SEND BACK AN INFORMATION OF THE RESULT OF THE SYS_CALL*/
	com = (rc[1]*0x100)+4;
	PUSH(back);PUSHN(1);PUSH(com);CALL(105);ADDESP(12);
    }
}
/*===========================================================================*/
/*                               PROC_STATUS                                 */
/*===========================================================================*/
static int proc_status()
{
extern unsigned long proc_1[100];
extern unsigned long p_head[200],max_proc;
extern struct dwq dma_wq[120];
char * hex = "xxxx\0";
unsigned long i,dat,pid;
LOCK;
    printd("Some DEBUG information\0");
    printd("   pid   | status\0");i=0;
    for (i=1;i<max_proc;i++) {
	printd("\0");pid = proc_1[i];
	dat = mem_vll(p_head[pid]+proc_STATUS);
	PR_XY_HEXL(pid,0,24);PR_XY_HEXL(dat,10,24);
	dat = mem_vll(p_head[pid]+PS_start);PR_XY_HEXL(dat,25,24);
	dat = mem_vll(p_head[pid]+PS_size);PR_XY_HEXL(dat,35,24);
	dat = mem_vll(p_head[pid]+VT_ID);PR_XY_HEXL(dat,50,24);
    }
    pr_time();
UNLOCK;
return 1;
}
/*===========================================================================*/
/*                                SET_TIMER                                  */
/*===========================================================================*/
static int reset_tab()
{
 extern unsigned long tit_p,tit_max;
 extern struct time_tab titab[200];
 unsigned long i;
     if (tit_p > 1) {
         for (i=0;i<(tit_max-tit_p);i++)
	 {
	 titab[i].start = titab[i+tit_p].start;
	 titab[i].tout = titab[i+tit_p].tout;
	 titab[i].to = titab[i+tit_p].to;
	 }
	 tit_max = tit_max - tit_p;
	 tit_p = 0;
	return(1);
    }
     printd("<<resetd>> !!!!! fail");
 return(0);
}

static int move(unsigned long br_point,int mov)
{
 extern unsigned long tit_p,tit_max;
 extern struct time_tab titab[200];
 unsigned long i;
    if (mov < 0) {
	if ((br_point + mov)<0) {return(0);}
    	    for (i=br_point;i!=(tit_max-br_point);i++) {
	    titab[i]=titab[i-mov];
	    }
    } else {
	if ((tit_max + mov)>199) {return(0);}
    	for (i=tit_max;i!=br_point;i--) {
	titab[i]=titab[i-mov];
	}
    }
    tit_max = tit_max + mov;
    return(1);
}
static void insert(unsigned long pos,unsigned long start,
unsigned long tout,unsigned long to,unsigned long sig)
{
 extern unsigned long tit_p,tit_max;
 extern struct time_tab titab[200];
    if (move(pos,1)) {
    } else {
	pos -= tit_p;
	reset_tab();
	if (move(pos,1)!=1) {printd("<<error>>");return;}
    }
    titab[pos].start = start;
    titab[pos].tout = tout;
    titab[pos].to = to;
    titab[pos].sig = sig;
}

static unsigned long find(unsigned long tout) {
 extern unsigned long tit_p,tit_max;
 extern struct time_tab titab[200];
 unsigned long i;
    for (i=tit_p;i<tit_max;i++) {
	if (titab[i].tout > tout) {
	 return(i);
	}
    }
    return(tit_max);
}

static int set_timer(unsigned long proc,unsigned long tfs)
{
 extern unsigned long tit_p,tit_max;
 extern unsigned long uct_time[3];
 extern struct time_tab titab[200];
 unsigned long i;
LOCK;
 if (tit_max < 200) {
     insert(find(tfs),uct_time[0],tfs,proc,16);
 } else {
     if (tit_p > 1) {
     reset_tab();
     insert(find(tfs),uct_time[0],tfs,proc,16);
     } else { return 0;printd("<<system full>>");}
 }
 titab[tit_max].tout = 0;
UNLOCK;
return 1;
}
/*===========================================================================*/
/*                               REBOOT_SYSTEM                               */
/*===========================================================================*/
static void reboot_system()
{
 extern unsigned long p_head[200];
    printd("Preparing for reset...\0");
    mem_vsl(p_head[0]+mess_STK,0);
    mem_vsl(p_head[0]+mess_DEST,0);
    mem_vsl(p_head[0]+mess_SIZE,0);
    mem_vsl(p_head[0]+mesr_STK,0);
    mem_vsl(p_head[0]+mesr_DEST,0);
    mem_vsl(p_head[0]+mesr_SIZE,0);
    put_done();
    printd("Umountig filesystems...\0");
    PUSHN(1);PUSHESP;PUSHN(1);PUSHN(0xE03);CALL(105);ADDESP(16);
    put_done();
    out_byte(0x3F2,0);// stop floppy motor
    printd("The system will be reseted NOW\0");
       asm("xorl	%eax,%eax
    	    xorl	%edi,%edi
	    movl	$64,%ecx
	    cld
	    rep
	    stosl
	    divl	%eax,%eax
	    divl	%eax,%eax");
}
/*===========================================================================*/
/*                               MAKE_SIGNAL                                 */
/*===========================================================================*/
static int make_signal(unsigned long proc_to,unsigned long signal)
{
 extern unsigned long sig_num,sig_to,doing_proc;
    LOCK;
    sig_num = signal;
    sig_to = proc_to;
    doing_proc = 13;
    CALL(264);
    UNLOCK;
 return(1);
}
/*===========================================================================*/
/*                                  PR_TIME                                  */
/*===========================================================================*/
static int pr_time()
{
 extern unsigned long str_con,g_sig,tit_max,tit_p,dm1,dma_p,dma_w;
 extern unsigned long uct_time[3];
 unsigned long stack,in;
 char * hex = "xxxx\0";
		PR_XY_HEXL(uct_time[0],70,0);
/*		in = tit_p;
		PR_XY_HEXL(in,70,1);
		in = tit_max;
		PR_XY_HEXL(in,70,2);
		in = dma_p;
		PR_XY_HEXL(in,70,3);
		in = dma_w;
		PR_XY_HEXL(in,70,4);
		in = dm1;
		PR_XY_HEXL(in,70,5);*/
 return(1);
}