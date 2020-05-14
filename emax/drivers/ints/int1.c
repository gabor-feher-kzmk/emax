/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*                                  KBD_INT                                  */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "int1.h"

static void kout_byte(unsigned long port,unsigned long data);
static unsigned char kin_byte(unsigned long port);

ENTRY(keyboard);
static void key_pressed()
{
int val,key;
extern unsigned long sig_num,sig_to,doing_proc,int1;
LOCK;
 while(TRUE) { 
    key = kin_byte(0x60);
    int1 = key;
    if (key == 66) { 1/0; }
    sig_num = 1;
    sig_to = 9;
    doing_proc = 13;
asm( "	movb	$0x21,%al\n\t
	outb	%al,$0x20\n\t");
    CALL(264);
 } // the while (forever) loop
}
/*===========================================================================*/
/*                                  PIN_BYTE                                 */
/*===========================================================================*/
static unsigned char kin_byte(unsigned long port)
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
static void kout_byte(unsigned long port,unsigned long data)
{
PUSH(port);
PUSH(data);
asm("
    popl	%eax
    popl	%edx
    outb	%al,%dx    
");
}

ENTRY(keyboard_end);
