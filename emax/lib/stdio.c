/*===========================================================================*/
/*                                  MK_DIR                                   */
/*===========================================================================*/
static int mk_dir(unsigned char * ch)
{
int ret;
unsigned long i;
PUSHN(14);PUSH(ch);PUSHN(64);PUSHN(0x09);CALL(105);ADDESP(16);
PUSHN(3);PUSHESP;PUSHN(1);PUSHN(0xE03);CALL(105);ADDESP(16);
RES(ret);
return(ret);
}
/*===========================================================================*/
/*                                  FOPEN                                    */
/*===========================================================================*/
static unsigned long fopen(unsigned char * ch,unsigned long mode)
{
unsigned long ret,i;
PUSHN(14);PUSH(ch);PUSHN(64);PUSHN(0x09);CALL(105);ADDESP(16);
PUSH(mode);PUSHN(4);PUSHESP;PUSHN(2);PUSHN(0xE03);CALL(105);ADDESP(20);
RES(ret);
return(ret);
}
/*===========================================================================*/
/*                                   FCLOSE                                  */
/*===========================================================================*/
static int fclose(unsigned long fp)
{
int ret;
unsigned long i;
PUSH(fp);PUSHN(5);PUSHESP;PUSHN(2);PUSHN(0xE03);CALL(105);ADDESP(20);
RES(ret);
return(ret);
}
/*===========================================================================*/
/*                                   FREAD                                   */
/*===========================================================================*/
static unsigned char fread(unsigned long fp,unsigned long pos)
{
int ret;
PUSH(pos);PUSH(fp);PUSHN(6);PUSHESP;PUSHN(3);PUSHN(0xE03);CALL(105);ADDESP(24);
RES(ret);
return(ret);
}
/*===========================================================================*/
/*                                   FSIZE                                   */
/*===========================================================================*/
static unsigned long fsize(unsigned long fp)
{
unsigned long ret;
PUSH(fp);PUSHN(7);PUSHESP;PUSHN(2);PUSHN(0xE03);CALL(105);ADDESP(20);
RES(ret);
return(ret);
}
/*===========================================================================*/
/*                                FIND_NEXT                                  */
/*===========================================================================*/
static int find_next(unsigned char * ch,unsigned char * nm,unsigned long num)
{
int ret;
unsigned long i;
PUSHN(14);PUSH(ch);PUSHN(64);PUSHN(0x09);CALL(105);ADDESP(16);
PUSHN(14);PUSH(nm);PUSHN(13);PUSHN(0x309);CALL(105);ADDESP(16);
PUSH(num);PUSHN(2);PUSHESP;PUSHN(2);PUSHN(0xE03);CALL(105);ADDESP(20);
RES(ret);
return(ret);
}
/*===========================================================================*/
/*                                 FRLONG                                    */
/*===========================================================================*/
static unsigned long frlong(unsigned long fp,unsigned long byte)
{
unsigned long ret;
    ret = 0;
    ret += fread(fp,byte);byte++;
    ret += (fread(fp,byte)*0x100);byte++;
    ret += (fread(fp,byte)*0x10000);byte++;
    ret += (fread(fp,byte)*0x1000000);
    return ret;
}
/*===========================================================================*/
/*                                  FGET_DEV                                 */
/*===========================================================================*/
static unsigned long fget_dev(unsigned long fp)
{
unsigned long ret;
PUSH(fp);PUSHN(8);PUSHESP;PUSHN(2);PUSHN(0xE03);CALL(105);ADDESP(20);
RES(ret);
return(ret);
}
