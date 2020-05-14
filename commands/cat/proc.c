/* *   		     proc2.h includes the header for proc2		  *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
#include "./pcall.h"
LONG(proc_head_end-SEF); /* The size */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
ENTRY(SEF); /* The strustur of the executable => Sharable & Executable Format*/
LONG(SEF_end-SEF); /* The size */
STRING("SEF");BYTE(1);/* The exec type and the version of it */
LONG(10000);// the size of the stack segment
ENTRY(SEF_end);
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
LONG(proc_head_end-proc_head);
ENTRY(proc_head);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
LONG(0x00000000);//the number of the proc's selector in GDT	
LONG(0x00000000);//LDT selector in GDT 
LONG(0x00000000);//the offset of the LDT
LONG(0x00000000);//the offset of the TSS
LONG(0x00000000);//the start and the 			INFO
LONG(0x00000000);//end of the proc's segments
LONG(0x00000000);//the type of the header (proc oder thread)
LONG(0x00000000);//the the next thread to schedule (if there aren't threads the procs PID)
/* misc info */
LONG(0x00000000);//the number of the vt witch is connected to the proc
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
	/*dma 2*/
LONG(0x00000000);//dma 2 offset 
LONG(0x00000000);//dma 2 size				DMA
LONG(0x00000000);//dma 2 dest proc
LONG(0x00000000);//dma 2 mode (rw)
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
/*The information of the threads */
LONG(0x00000000);// the number of the last thread in the process
LONG(0x00000000);// the mother process which the thread is in
LONG(0x00000000);// the number of the thread
/* this is the space where the processes LDT is saved (only in thread [0])*/
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
ENTRY(proc_head_end);
