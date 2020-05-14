/* *   		clock.c includes the C language stuff for clock 	  *
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
#include "../include/dtypes.h"
#include "../include/clock.h"

static void inc_time();
/*===========================================================================*/
/*             ||TASK 72||         CLOCK                                     */
/*===========================================================================*/
#include "clk.h"
ENTRY(clock);
static void tick()
{
 extern unsigned long mintime,g_sig,sig_num,sig_to,tit_p,tit_max,doing_proc;
 extern unsigned long uct_time[3];
 extern struct time_tab titab[200];

    while(TRUE) { 
	if (0xFFFFFFFF - uct_time[0] > mintime) {
	 uct_time[0] = uct_time[0] + mintime;
	} else { /* Hmmm... another tick for the clock (this is boring)*/
	 uct_time[0] = uct_time[0] + mintime;
	 uct_time[1]++;
	}
	if (tit_p < tit_max) { /* wake up a process if it want's it*/
	    if (uct_time[0] - titab[tit_p].start > titab[tit_p].tout) {
	    g_sig = 1;
	    sig_num = 16;
	    sig_to = titab[tit_p].to;
	    tit_p++; /* set table pointer to the next proc or the end*/
	    doing_proc = 13;
	    asm( "movb	$0x20,%al\n\t
        	  outb	%al,$0x20\n\t"); /* Enable the interrupt */
	    CALL(264);
	    } else {
		asm( "movb	$0x20,%al\n\t
                      outb	%al,$0x20\n\t"); /* Enable the interrupt */
		CALL(88); /* Call the scheduler */
	    }
	} else {
	asm( "movb	$0x20,%al\n\t
              outb	%al,$0x20\n\t"); /* Enable the interrupt */
	CALL(88); /* Call the scheduler */
	}
    }
}
ENTRY(clock_end);
	//======================================*
	// END OF CLOCK TASK			*
	//======================================*

/*===========================================================================*/
/*			THE STUP FOR THE CLOCK TASK			     */
/*===========================================================================*/
/* This will be called from main.c (not from the task) */

void init_clock()
{
 extern unsigned long mintime;
 extern unsigned long uct_time[3];

 mintime = 1000 / HZ; /* Calculate how many milisecond's have been since last tick*/
 out_byte(TIMER_MODE,SQUARE_WAVE); /* 	\	*/
 out_byte(TIMER0,TIMER_COUNT); /*	| set the time to the pre-defined values  */
 out_byte(TIMER0,TIMER_COUNT >> 8); /*	/	*/
 mk_idt32(0x20,0,72,(TASK_GATE(1,0))*0x100);
 uct_time[0] = 0;/*\							*/
 uct_time[1] = 0;/* | Fill the whole 128bit of the timer with '0' 	*/
 uct_time[2] = 0;/* |	( in version 0.1 only 64-bit is used )		*/
 uct_time[3] = 0;/*/							*/
}

