/* *   	   keyboard.c includes the kernels keyboard driver routines	  *
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

#define MAX_KB_ACK_RET	0x1000
#define MAX_KB_BUSY_RET	0x1000
#define KB_ACK		0xFA
#define KB_BUSY		0x02
#define LED_CODE 	0xED
#define KEYBD		0x60
#define KB_COMMAND	0x64
#define KB_STATUS	0x64
#define KB_RESET	0x01
#define KBIT		0x80

#define CTR_ALT_DEL	0xC000C
#define NUM_L		16
#define CAPS_L		18
#define SCROLL_L	17
#define F1		1
#define ALT_F1		0x80001
#define ALT_F2		0x80002
/*===========================================================================*/
/*                                KBD_TRANS                                  */
/*===========================================================================*/
static void mem_str(unsigned long offset,unsigned long data);
static unsigned long case_mod(unsigned long key,unsigned long key_m);
static void pout_byte(unsigned long port,unsigned long data);
static unsigned char pin_byte(unsigned long port);
static unsigned long scan_keyboard();
static void init_keyb();
static void load_keymap();
static int kb_ack();
static int kb_wait();
static void set_leds();
static int func_key(unsigned long code);
static void writeln(unsigned long * ch);
#include "keyboard.h"
ENTRY(kbd);
static unsigned long key_buff[100];
static unsigned long pp,key_str,con;
static int cl,sl,nl,setld,sleep;
static unsigned long tmp,key_tmp,key_tmp2;

void kbd_task() 
{ 
unsigned long tr_buff[100];
unsigned long i,*bp,size,sc;
LOCK;
init_keyb();
 while (TRUE) {
bp = &tr_buff[1];
    if (setld) {set_leds(); setld=0;}  
    if (pp != 1) {con=150;
	key_buff[pp]=0;i=1;
	    while (key_buff[i] != 0) {	
    		tr_buff[i] = key_buff[i];
		i++;
	    }
       tr_buff[i]=0;pp=1;
        PUSHN(6);PUSH(bp);PUSH(i);PUSHN(8);CALL(105);ADDESP(16);//mk_dma
	PUSHN(4);PUSHN(2);PUSHESP;PUSHN(2);PUSHN(0x603);CALL(105);ADDESP(20);//sys_call
   }     
    if (con == 0) {
	PUSHN(0x10A);CALL(105);ADDESP(4);
	con=150;
    } else {
	con--;
    }
 } // while (forever)
}
ENTRY(kbd_sig1);
static void kbd_int()
{
 extern unsigned long doing_proc,int1;
 LOCK;
 while(TRUE) { 
	    key_tmp = key_str;
	    key_tmp &= 0xFFFF0000;
	    key_tmp2 = int1;
	    key_str = case_mod(key_tmp2,key_tmp);
	    func_key(key_str);
	if (pp < 100) {
	    key_buff[pp]  = key_str;
	    pp++;
	}
    if (con == 0) {
    PUSHN(0x20A);CALL(105);ADDESP(4); /* wake up mum */
    }
    PUSHN(0x10A);CALL(105);ADDESP(4);/* end of signal */
 } // the while (forever) loop
}
/*===========================================================================*/
/*                                  CASE_MOD                                 */
/*===========================================================================*/
static unsigned long case_mod(unsigned long key,unsigned long key_m)
{
extern unsigned long utfmap[128];
extern unsigned long utfsmap[128];
extern unsigned long utfcmap[128];
unsigned long mod;
unsigned long UTF;
mod = key_m;
    if (key <= 0x80) {
     switch (key) {
     case 0x36:	mod |= 0x10000;		break;
     case 0x2A:	mod |= 0x20000;		break;
     case 0x1D:	mod |= 0x40000;		break;
     case 0x38: mod |= 0x80000;		break;
     }
    } else {
     switch (key) {
     case 0xB6:	mod &= 0xFFFEFFFF;	break;
     case 0xAA:	mod &= 0xFFFDFFFF;	break;
     case 0x9D:	mod &= 0xFFFBFFFF;	break;
     case 0xB8: mod &= 0xFFF7FFFF;	break;
     }
    } 
   UTF = 0;
   if (key < 0x59) {
	if (mod && 0x20000) {
         UTF = utfsmap[key];
	} else {
    	    if (cl == 1) {
		UTF = utfcmap[key];
	    } else {
		UTF = utfmap[key];
	    }
	}
     UTF &= 0x0000FFFF;
    }
 mod = mod + UTF;
 return mod; 
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
/*                                SCAN_KEYBOARD                              */
/*===========================================================================*/
static unsigned long scan_keyboard()
{
int code;
int val;

code = pin_byte(KEYBD);
val = pin_byte(0x61);
pout_byte(0x61, val | KBIT);
pout_byte(0x61, val);
return code;
}
/*===========================================================================*/
/*                                  KBD_WAIT                                 */
/*===========================================================================*/
static int kb_wait()
{
int retries;

retries = MAX_KB_BUSY_RET + 1;
while (--retries != 0 && pin_byte(KB_STATUS) & KB_BUSY);
return(retries);
}
/*===========================================================================*/
/*                                  KBD_ACK                                  */
/*===========================================================================*/
static int kb_ack()
{
int retries;

retries = MAX_KB_ACK_RET + 1;
while (--retries != 0 && pin_byte(KEYBD) & KB_ACK);
return(retries);
}
/*===========================================================================*/
/*                                  SET_LEDS                                 */
/*===========================================================================*/
static void set_leds()
{
unsigned leds;

leds = (sl << 0) | (nl << 1) | (cl << 2);
kb_wait();
pout_byte(KEYBD, LED_CODE);  
kb_ack();

kb_wait();
pout_byte(KEYBD, leds);  
kb_ack();
}
/*===========================================================================*/
/*                                  FUNC_KEY                                 */
/*===========================================================================*/
static int func_key(unsigned long code)
{
extern unsigned long doing_proc;
extern unsigned long p_head[200];
int l;
    switch (code) {
    case NUM_L:		if (nl == 1) {nl = 0;} else {nl = 1;} setld=1;	break;
    case SCROLL_L:	if (sl == 1) {sl = 0;} else {sl = 1;} setld=1;	break;
    case CAPS_L:	if (cl == 1) {cl = 0;} else {cl = 1;} setld=1;	break;
    case CTR_ALT_DEL : // DO a TRIPLE FAULT and reboot...
    PUSHN(0x4);PUSHESP;PUSHN(1);PUSHN(0x403);CALL(105);ADDESP(16);
    case F1 : // some Debug information
    PUSHN(1);PUSHESP;PUSHN(1);PUSHN(0x403);CALL(105);ADDESP(16);
    break;
    }	
    switch (code) {
    case ALT_F1:	PUSHN(1);PUSHN(9);PUSHESP;PUSHN(2);PUSHN(0x603);CALL(105);ADDESP(20);	break;
    case ALT_F2:	PUSHN(2);PUSHN(9);PUSHESP;PUSHN(2);PUSHN(0x603);CALL(105);ADDESP(20);	break;
    case 3 : /*writeln("Console workilg \n\0");*/
    doing_proc = 6; l = 0/0; break;
    }	
}
/*===========================================================================*/
/*                                  PIN_BYTE                                 */
/*===========================================================================*/
static unsigned char pin_byte(unsigned long port)
{
unsigned long ret;
	PUSH(port);
	asm( "	popl	%edx
		xorl	%eax,%eax
		inb	%dx,%al ");
	RES(ret);
return ret;	
}
/*===========================================================================*/
/*                                  POUT_BYTE                                */
/*===========================================================================*/
static void pout_byte(unsigned long port,unsigned long data)
{
PUSH(port);
PUSH(data);
asm("
    popl	%eax
    popl	%edx
    outb	%al,%dx    
");
}
/*===========================================================================*/
/*                                 INIT_KEYB                                 */
/*===========================================================================*/
static void init_keyb()
{
extern unsigned long utfmap[128];
extern unsigned long utfsmap[128];
extern unsigned long utfcmap[128];
key_str=0;pp=1;setld=0;sl = 0;nl = 0;cl = 0;sleep=1;con=15;
set_leds();
#include "../include/keymap/uk.map"
}
ENTRY(kbd_end);

#include "./ints/int1.c"

