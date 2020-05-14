/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
/*                                    CAT                                    */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
#include "./pcall.h"
static unsigned long fopen(unsigned char * ch,unsigned long mode);
static int fclose(unsigned long fp);
static unsigned char fread(unsigned long fp,unsigned long pos);
static unsigned long fsize(unsigned long fp);

static void writeln(char * ch);
static int cat(unsigned char * ch);

void main(int argc, char *argv[])
{
char *xp;
    xp = (char *) &argv[0]+1;
    cat(xp);
PUSHN(3);PUSHESP;PUSHN(1);PUSHN(0x1303);CALL(105);ADDESP(20);//exit
}
static char fs_buff[263];
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
/*                                    CAT                                    */
/*===========================================================================*/
static int cat(unsigned char * ch)
{
unsigned long fp,i,c,size,bs;
int ret;
    fp = fopen(ch,1);
    if (fp == 0) {
	writeln("Error opening file : \0");writeln(ch);writeln("\r\0");
    } else {
	size = fsize(fp);
        bs = size / 200;
	for (c=0;c<bs;c++) {
	    for (i=0;i<200;i++) {
		fs_buff[i] = fread(fp,i+(c*200));
	    }fs_buff[200]=0;writeln(fs_buff);
	}
	c = size % 200;
	    for (i=0;i<c;i++) {
		fs_buff[i] = fread(fp,i+(bs*200));
	    }fs_buff[c]=0;writeln(fs_buff);writeln("\r\0");
    }
    fclose(fp);
return(ret);
}
/*===========================================================================*/
/*                                  FOPEN                                    */
/*===========================================================================*/
static unsigned long fopen(unsigned char * ch,unsigned long mode)
{
unsigned long ret,i;
PUSHN(14);PUSH(ch);PUSHN(15);PUSHN(0x09);CALL(105);ADDESP(16);
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
