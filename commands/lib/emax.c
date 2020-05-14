/*===========================================================================*/
/*                                 GET_MEM                                   */
/*===========================================================================*/
static unsigned long get_mem(unsigned long size)
{
 unsigned long ret;
 PUSH(size);PUSHN(1);PUSHESP;PUSHN(2);PUSHN(0xC03);CALL(105);ADDESP(20);
 RES(ret);
 return(ret);
}
/*===========================================================================*/
/*                                FREE_REGION                                */
/*===========================================================================*/
static int free_region(unsigned long from,unsigned long to)
{
 unsigned long ret;
 PUSH(to);PUSH(from);PUSHN(2);PUSHESP;PUSHN(3);PUSHN(0xC03);CALL(105);
 ADDESP(24);
 RES(ret);
 return(ret);
}
/*===========================================================================*/
/*                                   EXEC                                    */
/*===========================================================================*/
static unsigned long exec(const char * ch)
{
unsigned long ret,i;
PUSHN(19);PUSH(ch);PUSHN(65);PUSHN(0x09);CALL(105);ADDESP(16);
PUSHN(1);PUSHESP;PUSHN(1);PUSHN(0x1303);CALL(105);ADDESP(16);
RES(ret);
return(ret);
}
