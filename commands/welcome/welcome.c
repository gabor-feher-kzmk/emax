/*	Greatings to User >>
 *****************************************************************************/

#include "./pcall.h"
asm("jmp	main");
#include "../lib/stdlib.c"
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
	     movb	%ds:(%edi),%al\n\t
	     popl	%edi\n\t
	     pushl	%eax	    	
	    ");
	asm("popl	%0"
		: "=r" (lng)
		: );
return lng;
}

static void writeln(char * ch);

static char num[110];
 
void main(int argc, char *argv[])
{
unsigned char i;
char *xp;
    writeln("Welcome to EMAX os !\n\0");
    writeln("The argc is : \0");
    ltoa(num,argc);writeln(num);writeln("\n\0");
    
    writeln("The argv[0] pointer is : \0");
    ltoa(num,(unsigned long ) &argv[0]);writeln(num);writeln("\n\0");
    xp = (char *) &argv[0];
    writeln("argument : \0");writeln(xp);writeln("\n\0");

PUSHN(3);PUSHESP;PUSHN(1);PUSHN(0x1303);CALL(105);ADDESP(20);//exit
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
