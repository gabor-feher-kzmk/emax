/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*                                    LS                                     */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
#include "./pcall.h"
static char dir[60];

asm("jmp	main");
#include "../lib/stdio.c"
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
/*                                   LIST                                    */
/*===========================================================================*/
static void list(char * ch)
{
unsigned long num;
    num = find_next(ch,dir,0);
    while (num != 0) {
	num++;
	writeln(dir);writeln("\r\0");
	num = find_next(ch,dir,num);
    }
}

void main(int argc, char *argv[])
{
unsigned char i;
char *xp;
    xp = (char *) &argv[0]+1;
    list(xp);
PUSHN(3);PUSHESP;PUSHN(1);PUSHN(0x1303);CALL(105);ADDESP(20);//exit
}
