/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*                                    LS                                     */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
#include "./pcall.h"
asm("jmp	main");
#include "../lib/stdio.c"

void main(int argc, char *argv[])
{
unsigned char i;
char *xp;
    xp = (char *) &argv[0]+1;
    mk_dir(xp);
PUSHN(3);PUSHESP;PUSHN(1);PUSHN(0x1303);CALL(105);ADDESP(20);//exit
}
