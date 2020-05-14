/* *   			proc1.c includes an emergeancy shell 		  *
   *   -- By Kozmik (C) --                                     _____      *
   *      -------------  __  __           ______       _____  /    /  (C) *
   *     /____________/ /  \/  \         /      \      \    \/    /       *
   *    -------------  /        \       /   /\   \      \        /        *
   *   /____________/ /   /\/\   \     /   /  \   \     /       /         *
   *  -------------  /   /    \   \   /   /____\   \   /        \         *
   * /____________/ /___/      \___\ /___//_________\ /    /\____\        *
   *                                                 /____/               *
   ************************************************************************/
#define		TRUE			1
#define		NULL			0

#include "./pcall.h"
asm("jmp	main");
#include "../lib/string.c"
#include "../lib/emax.c"

static void r_stdin();
static void writeln(char * ch);
static void readln(char * ch);

static char command[601];

void main()
{
    while(TRUE) { 
	writeln("emax:~)\0");
	readln(command);
//	writeln(command);
	r_stdin();
    }
}
/*===========================================================================*/
/*                                 R_STDIN                                   */
/*===========================================================================*/
static void r_stdin()
{
 char *xp;
    if (strncmp(command,"exec",4)==0) {
	exec(strchr(command,' ')+1);
    return;}
    if (command[0]==0) {return;}
    exec(command);
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
