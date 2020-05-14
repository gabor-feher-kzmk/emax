/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*                                    LS                                     */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
#include "./pcall.h"
asm("jmp	main");
#include "../lib/stdlib.c"

void main(int argc, char *argv[])
{
unsigned char i;
unsigned long num;
char *xp;
    xp = (char *) &argv[0]+1;
    num = atol(xp);
    PUSH(num);PUSHN(2);PUSHESP;PUSHN(2);PUSHN(0x1303);CALL(105);ADDESP(20);
PUSHN(3);PUSHESP;PUSHN(1);PUSHN(0x1303);CALL(105);ADDESP(20);//exit
}
