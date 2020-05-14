/* *   			pinit.c includes the init jobs  		  *
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
//0x26 38

static unsigned long mem_load(unsigned long offset);
static void mem_str(unsigned long offset,unsigned long data);
static unsigned char m_al(unsigned long eax);
static unsigned long msel_off(unsigned long ss,unsigned long proc);
static unsigned char m_ah(unsigned long eax);
static unsigned long get_mreg(unsigned long pn,unsigned long reg);
static void load_mreg(unsigned long pn,unsigned long reg,unsigned long data);
static unsigned int mem_wload(unsigned long offset);
static unsigned long msel_hat(unsigned long ss,unsigned long proc);
static void writeln(char * ch);
static void readln(char * ch);

#include "pinit.h"	/* The U-space of the process*/
ENTRY(pinit);
extern unsigned long p_head[200]; /* the process management main tab */
/*  this is the INIT process of the OS. This proc is runned when the OS 
    is loaded from the floppy, and the 32-bit is executed. This will 
    start the main servers, and finally it'll allocate the VT's      */
void proc1_loop()	/* This is the init script, wich loads the 2nd level */
{			/* servers fs , mem etc.. */
/* Let's start the memory server */
 mem_str(p_head[12]+proc_STATUS,0);
 PUSHN(0x10A);CALL(105);ADDESP(4);/* go to sleep */
/* Now it's time to mount the fs */
 mem_str(p_head[14]+proc_STATUS,0);
 PUSHN(0x10A);CALL(105);ADDESP(4);/* go to sleep */
 /* Make the console to work with more than one VTs */
 PUSHN(1);PUSHESP;PUSHN(1);PUSHN(0x603);CALL(105);ADDESP(16);
 mem_str(p_head[21]+proc_STATUS,0); /* In this version the OS only suppports */
 mem_str(p_head[22]+proc_STATUS,0); /* two VTs, so let's start them 	     */
 PUSHN(0x10A);CALL(105);ADDESP(4);/* go to sleep */
}
ENTRY(pinit_sig3); /* the thread that handles the answer of the servers */
static void wake_up()
{
 while (TRUE) {
     PUSHN(0x20A);CALL(105);ADDESP(4);/* wake up mummy */
     PUSHN(0x10A);CALL(105);ADDESP(4);/* go to sleep */
 }
}
#include "./memutils.c"
ENTRY(pinit_end);


