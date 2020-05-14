/* *   		signal.c includes the signal management for the kernel	  *
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

#include "signal.h"

/*-----------------------------memutils.c------------------------------------*/
static void free_proc(unsigned proc);
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
static void msg_copy(unsigned long src,unsigned long dest,unsigned long size);
/*---------------------------------------------------------------------------*/
static void init_signal();
static int do_signal(unsigned long proc,unsigned long signal);
static unsigned long get_t(unsigned long proc,unsigned long signal);
ENTRY(signal);
/*
    The Signal Server handels the signals witch are made by the interrups or 
    processes
*/
/*===========================================================================*/
/*                                 GET_REC                                   */
/*===========================================================================*/
static void who_w()
{
 extern unsigned long sig_num,sig_to;
 LOCK;
 init_signal();
    while(TRUE) {
		if (do_signal(sig_to,sig_num)) {
		} else { CALL(88); }
    }/*while forever*/
}
/*===========================================================================*/
/*                                INIT_SIG                                   */
/*===========================================================================*/
static void init_signal()
{
}
/*===========================================================================*/
/*                                 DO_SIGNAL                                 */
/*===========================================================================*/
static int do_signal(unsigned long proc,unsigned long signal)
{
 extern unsigned long p_head[200];
 unsigned long t,h_proc;
 
 if (mem_load(p_head[proc]+SIGNAL) != 0xFFFFFFFF) {
  t=get_t(proc,signal);if (t!=0xFFFFFFFF) {
/*    if (mem_load(p_head[proc]+(t*12)+DEF_SIG+SIG_MASK)==0) {
        mem_str(p_head[proc]+(t*12)+DEF_SIG+SIG_MASK,1);*/
	h_proc = mem_load(p_head[proc]+(t*12)+DEF_SIG+SIG_THREAD);
	mem_str(p_head[mem_load(p_head[proc]+(t*12)+DEF_SIG+SIG_THREAD)]+proc_STATUS,0);
	switch_proc(h_proc,0);
	return(1);
/*    }*/
 }}
 return(0);
}
/*===========================================================================*/
/*                                   GET_T                                   */
/*===========================================================================*/
static unsigned long get_t(unsigned long proc,unsigned long signal)
{
extern unsigned long p_head[200];
unsigned long c,t,i;
 t=mem_load(p_head[proc]+SIGNAL)+1;
    for (i=0;i<t;i++) {
    if (mem_load(p_head[proc]+DEF_SIG+SIG_NUM+(i*12))==signal) {return(i);}
    }
 return(0xFFFFFFFF);
}
/*===========================================================================*/
/*                                 PSW_UTILS                                 */
/*===========================================================================*/
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

#include "memutils.c"
ENTRY(signal_end);
