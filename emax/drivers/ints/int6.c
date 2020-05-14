/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*                                FLOPPY_INT                                 */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#include "int6.h"
ENTRY(int6);
static void grab_int()
{
 extern unsigned long fdc_int_wait;
LOCK;
 while (TRUE) {
 if (fdc_int_wait == 1) {fdc_int_wait = 0;}
 asm( "movb	$0x26,%al\n\t
       outb	%al,$0x20\n\t");
    CALL(88);
 }
}
ENTRY(int6_end);
