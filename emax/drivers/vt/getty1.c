#include "../../include/pcall.h"
#include "../../include/pis.h"
#include "../../include/tss.h"
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*                                  GETTY1                                    */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void writeln(char * ch);
static void readln(char * ch);
static unsigned long exec(const char * ch); /* import this from emax lib*/
#include "./getty1.h"
ENTRY(getty1);
static char t[270];

void getty1()
{
 while (TRUE) {
    writeln("Press ENTER to activete console \0");
    readln(t);
    exec("/bin/sh");	/* This is the shell that's statred (later it will */ 
                        /* be the login ;-) ) */
 }
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
/*===========================================================================*/
/*                                 READLN                                    */
/*===========================================================================*/
static void readln(char * ch)
{
unsigned long buff[267];
unsigned long i,*cp;
cp=&buff[1];
PUSHN(6);PUSH(cp);PUSHN(255);PUSHN(0x108);CALL(105);ADDESP(16);
PUSHN(0);PUSHN(0);PUSHN(3);PUSHESP;PUSHN(3);PUSHN(0x603);CALL(105);ADDESP(24);
    i=1;    
    while (buff[i] != 0) {
    ch[i-1] = buff[i];
    i++;
    if (i == 255) {buff[i] = 0;}
    }ch[i-1] = 0;ch[i] = 0;
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
ENTRY(getty1_end);

